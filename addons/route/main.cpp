
#include "Database.h"
#include "Statement.h"
#include "manager.hpp"
#include "tsp.hpp"
#include "annealing.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>

#include "wrapper.hpp"
#include <node.h>
#include <uv.h>

// A JSON holding all necessary information for the tsptw solver
// {
//      "serviceTime": 30.0,                            # 30 seconds
//      "departureTime": 27000.0,                       # 7:30 AM
//      "dayPart": "Morning",                           # Morning / Noon / Study
//      "depot": "XXXXXXXXXXXXXXXXXXXXXXXX",            # AddressId
//      "students":
//       [
//            {
//                 "timewindow": [ 27000.0, 28800.0 ],  # [ 7.30 AM, 8.00 AM ]
//                 "addressId": "XXXXXXXXXXXXXXXXXXXXXXXX",
//                 "studentId": "YYYYYYYYYYYY"
//            },
//            ...
//            {
//                 "timewindow": [ 27900.0, 29700.0 ],  # [ 7.45 AM, 8.15 AM ]
//                 "addressId": "XXXXXXXXXXXXXXXXXXXXXXXX",
//                 "studentId": "YYYYYYYYYYYY"
//            }
//       ]
// }

#include "log.hpp"

namespace VRP_ROUTE
{

struct Worker
{
    Log log;

    std::string err;

    uv_work_t request;
    v8::Persistent<v8::Function> callback;

    std::string dbname, dayPart;
    double departureTime, serviceTime;
    Manager::Student depot;
    std::vector<Manager::Student> students;

    TSP::path<Manager::Student> path;

    static void work(uv_work_t *);
    static void completed(uv_work_t *, int);

    Worker(const std::string& name) : log(name) {}
};

void route(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate * iso = args.GetIsolate();

    v8::HandleScope scope(iso);

    if (args.Length() != 7)
    {
        iso->ThrowException
        (
            v8::Exception::SyntaxError
            (
                v8::String::NewFromUtf8
                (
                    iso,
                    (
                        "function requires 7 arguements "\
                        "(" + std::to_string(args.Length()) + " given)"
                    ).c_str()
                )
            )
        );

        args.GetReturnValue().Set(v8::Undefined(iso)); return;
    }

    if
    (
        !args[0]->IsString() || !args[1]->IsString() ||
        !args[2]->IsNumber() || !args[3]->IsNumber() ||
        !args[4]->IsObject() ||
        !args[5]->IsArray()  ||
        !args[6]->IsFunction()
    )
    {
        iso->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    iso,
                    "Invalid arguement(s) "\
                    "[ "\
                        "route"\
                        "("\
                            "dbname, dayPart, departureTime, serviceTime, depot, students, callback"\
                        ")"\
                    " ]"
                )
            )
        );

        args.GetReturnValue().Set(v8::Undefined(iso)); return;
    }

    Worker * worker = new Worker("route");
    
    worker->log(Log::Code::Message, "Initializing worker thread...");

    worker->request.data = worker;
    worker->callback.Reset(iso, args[6].As<v8::Function>());

    worker->dbname  = *v8::String::Utf8Value(args[0].As<v8::String>());
    worker->dayPart = *v8::String::Utf8Value(args[1].As<v8::String>());

    worker->departureTime = args[2].As<v8::Number>()->NumberValue();
    worker->serviceTime   = args[3].As<v8::Number>()->NumberValue();

    Wrapper::Object wstudent(iso, args[4].As<v8::Object>());

    wstudent.get("studentId", worker->depot._studentId);
    wstudent.get("addressId", worker->depot._addressId);

    Wrapper::Array wstudents(iso, args[5].As<v8::Array>());
    
    for (std::size_t sid = 0UL; sid < wstudents.size(); sid++)
    {
        wstudents.get(sid, wstudent);

        Manager::Student student;

        wstudent.get("studentId", student._studentId);
        wstudent.get("addressId", student._addressId);

        double earliest, latest;
        wstudent.get("earliest", earliest);
        wstudent.get("latest",   latest);

        student._timewindow = { earliest, latest };

        worker->students.emplace_back(student);
    }

    uv_queue_work(uv_default_loop(), &worker->request, Worker::work, Worker::completed);

    args.GetReturnValue().Set(v8::Undefined(iso));
}

void Worker::work(uv_work_t * request)
{
    Worker * worker = static_cast<Worker *>(request->data);

    using DVector = std::unordered_map<Manager::Student, double>;
    using DMatrix = std::unordered_map<Manager::Student, DVector>;

    DMatrix dmatrix;
    auto distance = [&dmatrix](const Manager::Student& A, const Manager::Student& B)
    {
        return dmatrix[A][B];
    };

    worker->log(Log::Code::Message, "Worker thread initializing distance matrix...");

    auto beg = std::chrono::high_resolution_clock::now();

    try
    {
        SQLite::Database database(worker->dbname);

        for (const auto& A : worker->students)
            for (const auto& B : worker->students)
                if (A != B)
                    dmatrix[A][B] = (A == worker->depot ? 0.0 : worker->serviceTime)
                                  + Manager::distance(database, A, B, worker->dayPart, worker->log);
    }
    catch (std::exception& e)
    {
        const std::string msg
        (
            "database=" + dbname + " day-part=" + dayPart + " sqlitecpp-exception=" + e.what()
        );

        worker->log(Log::Code::Error, worker->err = msg);

        return;
    }

    auto end = std::chrono::high_resolution_clock::now();

    double diff = static_cast<double>
    (
        std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count()
    ) / 1000.0;

    worker->log(Log::Code::Message, std::to_string(diff) + " seconds elapsed");

    // Local Optimization
    worker->log(Log::Code::Message, "Optimizing Route...");

    beg = std::chrono::high_resolution_clock::now();

    worker->path = TSP::nearestNeighbor<Manager::Student>
    (
        worker->depot,
        worker->students,
        distance
    );

    worker->path = TSP::opt2<Manager::Student>
    (
        worker->path.second.front(),
        worker->path.second,
        distance
    );

    // Compressed Annealing
    auto penalty = [&](const TSP::path<Manager::Student>& path)
    {
        double penalty = 0.0, arrival = worker->departureTime;
        for (std::size_t j = 0; j < path.second.size() - 1UL; j++)
        {
            const Manager::Student& previous = path.second[j];
            const Manager::Student& current  = path.second[j + 1UL];

            arrival += distance(previous, current);

            const double startOfService = std::max<double>
            (
                arrival,
                current._timewindow.x()
            );

            penalty += std::max<double>
            (
                0.0,
                startOfService + worker->serviceTime - current._timewindow.y()
            );
        }

        return penalty;
    };

    auto shift1 = [&distance](const TSP::path<Manager::Student>& current)
    {
        TSP::path<Manager::Student> next(0.0, current.second);

        const std::size_t i = 1UL + std::rand() % (next.second.size() - 2UL);
        const std::size_t j = 1UL + std::rand() % (next.second.size() - 2UL);

        const Manager::Student v(next.second[i]);
        next.second.erase(next.second.begin() + i);
        next.second.insert(next.second.begin() + j, v);

        next.first = TSP::totalCost<Manager::Student>(next.second, distance);

        return next;
    };

    auto cost = [](const TSP::path<Manager::Student>& path)
    {
        return path.first;
    };

    // Parameter Initialization (Robust Set provided by the authors):
    const double COOLING    = 0.95,    // (1)  Cooling Coefficient
                ACCEPTANCE  = 0.94,    // (2)  Initial Acceptance Ratio
                PRESSURE0   = 0.0,     // (3)  Initial Pressure
                COMPRESSION = 0.06,    // (4)  Compression Coefficient
                PCR         = 0.9999;  // (5)  Pressure Cap Ratio

    const std::size_t IPT = 30000UL,    // (6)  Iterations per temperature
                    MTC   = 100UL,      // (7)  Minimum number of temperature changes
                    ITC   = 75UL,       // (8)  Maximum idle temperature changes
                    TLI   = IPT,        // (9)  Trial loop of iterations
                    TNP   = 5000UL;     // (10) Trial neighbour pairs

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    worker->path = Annealing::compressed<TSP::path<Manager::Student>>
    (
        worker->path,
        shift1,
        cost,
        penalty,
        COOLING,
        ACCEPTANCE,
        PRESSURE0,
        COMPRESSION,
        PCR,
        IPT,
        MTC,
        ITC,
        TLI,
        TNP
    );

    // Remove depot instances
    worker->path.second.erase(worker->path.second.begin());
    worker->path.second.erase(worker->path.second.end());

    end = std::chrono::high_resolution_clock::now();

    diff = static_cast<double>
    (
        std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count()
    ) / 1000.0;

    worker->log(Log::Code::Message, std::to_string(diff) + " seconds elapsed");

    if (worker->students.size() != worker->path.second.size())
    {
        const std::string msg
        (
            "assertion-failed=\"The number of students before and after routing differs\""
        );

        worker->log(Log::Code::Error, worker->err = msg);

        return;
    }
}

void Worker::completed(uv_work_t * request, int status)
{
    v8::Isolate * iso = v8::Isolate::GetCurrent();

    v8::HandleScope scope(iso);

    Worker * worker = static_cast<Worker *>(request->data);

    worker->log(Log::Code::Message, "Packaging results...");

    Wrapper::Array wstudents = Wrapper::Array(iso, worker->path.second.size());

    for (std::size_t sid = 0UL; sid < worker->path.second.size(); sid++)
    {
        const Manager::Student& student = worker->path.second[sid];

        Wrapper::Object wstudent = Wrapper::Object(iso);

        wstudent.set("studentId", student._studentId);
        wstudent.set("addressId", student._addressId);

        wstudents.set(sid, wstudent);
    }

    Wrapper::Object wpath(iso);

    wpath.set("students", wstudents);
    wpath.set("cost",     worker->path.first);

    v8::Local<v8::Value> argv[] =
    {
        worker->err.empty()
        ? v8::Null(iso).As<v8::Value>()
        : v8::Exception::Error(v8::String::NewFromUtf8(iso, worker->err.c_str())),
        wpath.raw()
    };

    worker->log(Log::Code::Message, "Invoking callback...");

    v8::Local<v8::Function>::New(iso, worker->callback)->
        Call(iso->GetCurrentContext()->Global(), 2, argv);

    worker->callback.Reset(); delete worker;
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
    NODE_SET_METHOD(module, "exports", route);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

}
