
#include "Database.h"
#include "Statement.h"
#include "vector2.hpp"
#include "manager.hpp"
#include "tsp.hpp"
#include "annealing.hpp"
#include "json.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

#include <node.h>

// @ Parameter tsp-json:
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

    if (args.Length() != 2)
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    "No matching function for call to route(" + std::to_string(args.Length()) + ")"
                )
            )
        );

        return;
    }

    std::string dbname(*(v8::String::Utf8Value(args[0]->ToString())));

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

    double serviceTime, departureTime;
    std::string dayPart;
    Manager::Student depot;
    std::vector<Manager::Student> students;

    try
    {
        nlohmann::json json = nlohmann::json::parse
        (
            *(v8::String::Utf8Value(args[1]->ToString()))
        );
        
        serviceTime      = json["serviceTime"];
        departureTime    = json["departureTime"];
        dayPart          = json["dayPart"];
        depot._addressId = json["depot"];

        for (const auto& entry : json["students"])
        {
            Manager::Student student;
            student._studentId = entry["studentId"];
            student._addressId = entry["addressId"];
            student._timewindow  = Vector2(entry["timespan"][0], entry["timewindow"][1]);

            students.emplace_back(student);
        }
    } catch (std::exception& e)
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

    nlohmann::json json;
    for (const auto& student : path.second)
    {
        json["students"].emplace_back(nlohmann::json::object());

        json["students"].back()["addressId"] = student._addressId;
        json["students"].back()["studentId"] = student._studentId;
    }

    json["cost"]    = path.first;
    json["penalty"] = penalty(path);

    args.GetReturnValue().Set
    (
        v8::String::NewFromUtf8
        (
            isolate, json.dump().c_str()
        )
    );
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
    NODE_SET_METHOD(module, "exports", route);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
