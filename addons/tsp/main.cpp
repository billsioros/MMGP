
#include "Database.h"
#include "Statement.h"
#include "vector2.hpp"
#include "manager.hpp"
#include "tsp.hpp"
#include "annealing.hpp"
#include "json.hpp"
#include <unordered_map>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include <node.h>

// @ Parameter tsp-json:
// {
//      "daypart": "Morning",
//      "depot": "XXXXXXXXXXXX",
//      "students":
//       [
//            {
//                 "timespan":  [ 7.30, 8.00 ],
//                 "addressId": "XXXXXXXXXXXX",
//                 "studentId": "YYYYYYYYYYYY"
//            },
//            ...
//            {
//                 "timespan":  [ 7.45, 8.15 ],
//                 "addressId": "XXXXXXXXXXXX",
//                 "studentId": "YYYYYYYYYYYY"
//            }
//       ]
// }

using DVector = std::unordered_map<Manager::Student, double>;
using DMatrix = std::unordered_map<Manager::Student, DVector>;

void tsp(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate * isolate = args.GetIsolate();

    // Firstly check if there are at least 2 arguements
    // otherwise throw an exception and exit
    if (args.Length() < 2)
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    "<ERR>: Missing parameter(s) <database> and/or <tsp-json>"
                )
            )
        );

        std::exit(EXIT_FAILURE);
    }

    // Secondly, assert arguement integrity.
    // Using a try / catch scheme handle any SQLiteCpp or
    // nlohmann::json exceptions.
    // An error in this stage (probably) indicates malformed arguements
    std::string daypart;
    Manager::Student depot;
    std::vector<Manager::Student> students;

    // (1) Parse the second arguement,
    // which is expected to be a json string of the format @ Parameter
    try
    {
        nlohmann::json json = nlohmann::json::parse(*(v8::String::Utf8Value(args[1]->ToString())));
        
        daypart = json["daypart"];
        
        depot._addressId = json["depot"];

        for (const auto& entry : json["students"])
        {
            Manager::Student student;
            student._studentId = entry["studentId"];
            student._addressId = entry["addressId"];
            student._timespan  = Vector2(entry["timespan"][0], entry["timespan"][1]);

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
                    (std::string("<ERR>: Exception ( ") + e.what() + " )").c_str()
                )
            )
        );
    }

    // (2) Initialize a SQLite::Database unique_ptr
    // with the specified database path
    std::unique_ptr<SQLite::Database> database;
    try
    {
        database = std::make_unique<SQLite::Database>(*(v8::String::Utf8Value(args[0]->ToString())));
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
                    (std::string("<ERR>: Exception ( ") + e.what() + " )").c_str()
                )
            )
        );
    }

    // Given the "distance" lambda function which
    // returns the expected duration to service
    // student A  and then go to student B,
    // determine an initial solution of the tsp
    // without taking into consideration time windows
    auto distance = [&](const Manager::Student& A, const Manager::Student& B)
    {
        static DMatrix dmatrix;

        DMatrix::const_iterator mit;
        DVector::const_iterator vit;

        if ((mit = dmatrix.find(A)) != dmatrix.end())
            if ((vit = mit->second.find(B)) != mit->second.end())
                return vit->second;

        return
        (
            dmatrix[A][B] = Manager::distance(*database, A, B, daypart)
                          + (A == depot ? 0.0 : 30.0)
        );
    };

    TSP::path<Manager::Student> path;
    
    path = TSP::nearestNeighbor<Manager::Student>(depot, students, distance);

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, distance);

    // Finally, use a variation of the Simulated Annealing algorithm
    // (Compressed Annealing) while taking into consideration
    // the time windows. The algorithm succeeds in minimizing both
    // the total travel time and the time window inconsistencies
    auto penalty = [&distance](const TSP::path<Manager::Student>& path)
    {
        double penalty = 0.0, partial = 0.0;
        for (std::size_t j = 0; j < path.second.size() - 1UL; j++)
        {
            const Manager::Student& previous = path.second[j];
            const Manager::Student& current  = path.second[j + 1UL];

            partial += distance(previous, current);

            const double departure = std::max<double>(partial, current._timespan.x());

            penalty += std::max<double>(0.0, departure - current._timespan.y());
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

    json["cost"] = path.first;
    
    v8::Local<v8::String> rv = v8::String::NewFromUtf8(isolate, json.dump().c_str());

    args.GetReturnValue().Set(rv);
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
    NODE_SET_METHOD(module, "exports", tsp);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
