
#include "Database.h"
#include "Statement.h"
#include "manager.hpp"
#include "tsp.hpp"
#include "annealing.hpp"
#include <unordered_map>
#include <vector>
#include <string>

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

namespace VRP_ROUTE
{

struct Worker
{
    uv_work_t request;
    v8::Persistent<v8::Function> callback;

    std::string dbname, dayPart;
    double departureTime, serviceTime;
    Manager::Student depot;
    std::vector<Manager::Student> students;

    TSP::path<Manager::Student> path;

    static void work(uv_work_t *);
    static void completed(uv_work_t *, int);
};

void route(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate * iso = args.GetIsolate();

    v8::HandleScope scope(iso);

    if (args.Length() != 7)
    {
        iso->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    iso,
                    (
                        "TypeError: function requires 7 arguements "\
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
                    "TypeError: Invalid arguement(s) "\
                    "[ "\
                        "route"\
                        "("\
                            "dbname, dayPart, departureTime, serviceTime, depot, students"\
                        ")"\
                    " ]"
                )
            )
        );

        args.GetReturnValue().Set(v8::Undefined(iso)); return;
    }

    Worker * worker = new Worker;
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

        double fst, snd;

        wstudent.get("longitude", fst);
        wstudent.get("latitude",  snd);
        student._position = { fst, snd };

        wstudent.get("earliest", fst);
        wstudent.get("latest",   snd);
        student._timewindow = { fst, snd };

        worker->students.emplace_back(student);
    }

    uv_queue_work(uv_default_loop(), &worker->request, Worker::work, Worker::completed);

    args.GetReturnValue().Set(v8::Undefined(iso));
}

void Worker::work(uv_work_t * request)
{
    v8::Isolate * iso = v8::Isolate::GetCurrent();

    v8::HandleScope scope(iso);

    Worker * worker = static_cast<Worker *>(request->data);

    using DVector = std::unordered_map<Manager::Student, double>;
    using DMatrix = std::unordered_map<Manager::Student, DVector>;

    DMatrix dmatrix;
    auto distance = [&dmatrix](const Manager::Student& A, const Manager::Student& B)
    {
        DMatrix::const_iterator mit;
        DVector::const_iterator vit;

        if ((mit = dmatrix.find(A)) != dmatrix.end())
            if ((vit = mit->second.find(B)) != mit->second.end())
                return vit->second;

        return std::numeric_limits<double>().max();
    };

    try
    {
        SQLite::Database database(worker->dbname);

        for (const auto& A : worker->students)
            for (const auto& B : worker->students)
                if (A != B)
                    dmatrix[A][B] = (A == worker->depot ? 0.0 : worker->serviceTime)
                                  + Manager::distance(database, A, B, worker->dayPart);
    }
    catch (std::exception& e)
    {
        iso->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    iso,
                    (std::string("SQLiteCpp Exception ( ") + e.what() + " )").c_str()
                )
            )
        );

        return;
    }

    // Local Optimization
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
}

void Worker::completed(uv_work_t * request, int status)
{
    v8::Isolate * iso = v8::Isolate::GetCurrent();

    v8::HandleScope scope(iso);

    Worker * worker = static_cast<Worker *>(request->data);

    Wrapper::Array wstudents = Wrapper::Array(iso, worker->path.second.size() - 2UL);

    for (std::size_t sid = 1UL; sid < worker->path.second.size() - 1UL; sid++)
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

    v8::Local<v8::Value> argv[] = { wpath.raw() };

    v8::Local<v8::Function>::New(iso, worker->callback)->
        Call(iso->GetCurrentContext()->Global(), 1, argv);

    worker->callback.Reset(); delete worker;
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
    NODE_SET_METHOD(module, "exports", route);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);

}
