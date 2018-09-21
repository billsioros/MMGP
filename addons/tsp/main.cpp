
#include "Database.h"
#include "Statement.h"
#include "vector2.hpp"
#include "manager.hpp"
#include "tsp.hpp"
#include "annealing.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

#include "wrapper.hpp"
#include <node.h>

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

using DVector = std::unordered_map<Manager::Student, double>;
using DMatrix = std::unordered_map<Manager::Student, DVector>;

void route(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate * isolate = args.GetIsolate();

    if (args.Length() != 6)
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    (
                        "TypeError: function requires 6 arguements "\
                        "(" + std::to_string(args.Length()) + " given)"
                    ).c_str()
                )
            )
        );

        return;
    }

    if (
        !args[0]->IsString() || !args[1]->IsString() ||
        !args[2]->IsNumber() || !args[3]->IsNumber() ||
        !args[4]->IsObject() ||
        !args[5]->IsArray()
    )
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    "TypeError: Invalid arguement(s) type "\
                    "[ "\
                        "route"\
                        "("\
                            "const std::string& dbname, const std::string& dayPart, "\
                            "double departureTime, double serviceTime, "\
                            "const Manager::Student& depot, "\
                            "const std::vector<Manager::Student>& students"\
                        ") "\
                    "]"
                )
            )
        );

        return;
    }

    const std::string dbname(*v8::String::Utf8Value(args[0].As<v8::String>()));
    const std::string dayPart(*v8::String::Utf8Value(args[1].As<v8::String>()));
    
    const double departureTime = args[2].As<v8::Number>()->NumberValue();
    const double serviceTime   = args[3].As<v8::Number>()->NumberValue();

    Manager::Student depot;

    WObject wstudent(isolate, args[4].As<v8::Object>());

    wstudent.get("studentId", depot._studentId);
    wstudent.get("addressId", depot._addressId);

    std::vector<Manager::Student> students;

    WArray wstudents(isolate, args[5].As<v8::Array>());
    
    for (std::size_t sid = 0UL; sid < wstudents.size(); sid++)
    {
        Manager::Student student;

        wstudents.get(sid, wstudent);

        wstudent.get("studentId", student._studentId);
        wstudent.get("addressId", student._addressId);

        double fst, snd;

        wstudent.get("longitude", fst);
        wstudent.get("latitude",  snd);
        student._position = { fst, snd };

        wstudent.get("earliest", fst);
        wstudent.get("latest",   snd);
        student._timewindow = { fst, snd };

        students.emplace_back(student);
    }

    std::unique_ptr<SQLite::Database> database;
    try
    {
        database = std::make_unique<SQLite::Database>(dbname);
    }
    catch (std::exception& e)
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    (std::string("Exception ( ") + e.what() + " )").c_str()
                )
            )
        );

        return;
    }

    DMatrix dmatrix;
    auto distance = [&](const Manager::Student& A, const Manager::Student& B)
    {
        DMatrix::const_iterator mit;
        DVector::const_iterator vit;

        if ((mit = dmatrix.find(A)) != dmatrix.end())
            if ((vit = mit->second.find(B)) != mit->second.end())
                return vit->second;

        return
        (
            dmatrix[A][B] = (A == depot ? 0.0 : serviceTime)
                          + Manager::distance(*database, A, B, dayPart)
        );
    };

    TSP::path<Manager::Student> path;
    
    // Local Optimization
    path = TSP::nearestNeighbor<Manager::Student>(depot, students, distance);

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, distance);

    // Compressed Annealing
    auto penalty = [&](const TSP::path<Manager::Student>& path)
    {
        double penalty = 0.0, arrival = departureTime;
        for (std::size_t j = 0; j < path.second.size() - 1UL; j++)
        {
            const Manager::Student& previous = path.second[j];
            const Manager::Student& current  = path.second[j + 1UL];

            arrival += distance(previous, current);

            const double startOfService = std::max<double>(arrival, current._timewindow.x());

            penalty += std::max<double>(0.0, startOfService + serviceTime - current._timewindow.y());
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
    const double COOLING     = 0.95,    // (1)  Cooling Coefficient
                 ACCEPTANCE  = 0.94,    // (2)  Initial Acceptance Ratio
                 PRESSURE0   = 0.0,     // (3)  Initial Pressure
                 COMPRESSION = 0.06,    // (4)  Compression Coefficient
                 PCR         = 0.9999;  // (5)  Pressure Cap Ratio

    const std::size_t IPT = 30000UL,    // (6)  Iterations per temperature
                      MTC = 100UL,      // (7)  Minimum number of temperature changes
                      ITC = 75UL,       // (8)  Maximum idle temperature changes
                      TLI = IPT,        // (9)  Trial loop of iterations
                      TNP = 5000UL;     // (10) Trial neighbour pairs

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    path = Annealing::compressed<TSP::path<Manager::Student>>
    (
        path,
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

    wstudents = WArray(isolate, students.size());

    for (std::size_t sid = 1UL; sid < students.size() - 1UL; sid++)
    {
        const Manager::Student& student = students[sid];

        wstudent = WObject(isolate);

        wstudent.set("studentId", student._studentId);
        wstudent.set("addressId", student._addressId);

        wstudents.set(sid, wstudent);
    }

    WObject wtsp(isolate);

    wtsp.set("cost",     path.first);
    wtsp.set("penalty",  penalty(path));
    wtsp.set("students", wstudent);

    args.GetReturnValue().Set(wtsp.raw());
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
    NODE_SET_METHOD(module, "exports", route);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
