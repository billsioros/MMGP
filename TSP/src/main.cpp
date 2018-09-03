
#include "Database.h"
#include "Statement.h"
#include "vector2.hpp"
#include "manager.hpp"
#include "tsp.hpp"
#include "sannealing.hpp"
#include "../../data/nlohmann/json.hpp"
#include <unordered_map>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include <node.h>

using Cost  = std::unordered_map<const Manager::Student *, double>;
using Costs = std::unordered_map<const Manager::Student *, Cost>;

std::string parse(const std::string&, v8::Isolate *);

void load(
    SQLite::Database&,
    const std::string&,
    std::vector<Manager::Student>&,
    Manager::Student&,
    v8::Isolate *);

void tsp(const v8::FunctionCallbackInfo<Value>& args)
{
    v8::Isolate * isolate = args.GetIsolate();

    if (args.Length() < 4)
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    "<MSG>: TSP <database> <Bus-Id> <Day-Part> <Schedule-Id>"
                )
            )
        );

        std::exit(EXIT_FAILURE);
    }

    if (!args[1]->IsNumber() || !args[3]->IsNumber())
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    "<ERR>: <Bus-Id> and <Schedule-Id> should be positive integers"
                )
            )
        );

        return;
    }

    std::string dbname(argv[0]->str()),
                BusScheduleId(argv[1]->str() + parse(argv[2]->str(), isolate) + argv[3]->str()),
                DayPart(argv[2]->str());

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
                    "<ERR>: Exception ( " + e.what() + " )"
                )
            )
        );
    }

    std::vector<Manager::Student> students; Manager::Student depot;

    load(*database, BusScheduleId, students, depot, isolate);

    auto cost = [&](const Manager::Student& A, const Manager::Student& B)
    {
        static Costs costs;
    
        Costs::const_iterator it1; Cost::const_iterator it2;
        if ((it1 = costs.find(&A)) != costs.end())
            if ((it2 = it1->second.find(&B)) != it1->second.end())
                return it2->second;

        return (costs[&A][&B] = Manager::distance(*database, A, B, DayPart));
    };

    TSP::path<Manager::Student> path;
    
    path = TSP::nearestNeighbor<Manager::Student>(
        depot,
        students,
        cost
    );

    path = TSP::opt2<Manager::Student>(
        path.second.front(),
        path.second,
        cost
    );

    path.second = SimulatedAnnealing<std::vector<Manager::Student>>(
        path.second,
        [](const std::vector<Manager::Student>& current)
        {
            std::vector<Manager::Student> next(current);

            const std::size_t i = 1UL + std::rand() % (next.size() - 2UL);
            const std::size_t j = 1UL + std::rand() % (next.size() - 2UL);

            std::reverse(next.begin() + i, next.begin() + j);

            return next;
        },
        [&cost](const std::vector<Manager::Student>& current)
        {
            double total = 0.0;
            for (std::size_t j = 0; j < current.size() - 1UL; j++)
                total += cost(current[j], current[j + 1UL]);

            return total;
        },
        1000000.0,
        0.00003,
        1000000UL
    );

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, cost);

    nlohmann::json json;
    for (const auto& element : path.second)
        json["students"].append(element._studentId);

    json["cost"] = path.first;
    
    v8::Local<v8::String> rv = v8::String::NewFromUtf8(isolate, json.to_string());

    args.GetReturnValue().Set(rv);
}

void Init(v8::Local<v8::Object> exports)
{
    NODE_SET_METHOD(exports, "tsp", tsp);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);

std::string parse(const std::string& DayPart, v8::Isolate * isolate)
{
    if (DayPart == "Morning")
    {
        return "\u03A0";
    }
    else if (DayPart == "Noon")
    {
        return "\u039c";
    }
    else if (DayPart == "Study")
    {
        return "\u0391";
    }
    else
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    "<ERR>: There can be no such option"
                )
            )
        );

        std::exit(EXIT_FAILURE);
    }
}

void load(
    SQLite::Database& database,
    const std::string& BusScheduleId,
    std::vector<Manager::Student>& students,
    Manager::Student& depot,
    v8::Isolate * isolate
)
{
    try
    {
        SQLite::Statement stmt(
            database,
            "SELECT Student.StudentID, Student.AddressID, Address.GPS_X, Address.GPS_Y "\
            "FROM Student, Address "\
            "WHERE Student.AddressID = Address.AddressID AND Student.DayPart = ? AND Student.BusSchedule = ?"
        );

        stmt.bind(1, BusScheduleId);

        while (stmt.executeStep())
        {
            int current = 0;

            const std::string _studentId(stmt.getColumn(current++).getText());
            const std::string _addressId(stmt.getColumn(current++).getText());
            const Vector2 _position(
                stmt.getColumn(current++).getDouble(),
                stmt.getColumn(current++).getDouble()
            );

            Manager::Student student;
            student._studentId = _studentId;
            student._addressId = _addressId;
            student._position  = _position;

            students.push_back(student);
        }
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
                    "<ERR>: Exception ( " + e.what() + " )"
                )
            )
        );
    }

    try
    {
        SQLite::Statement stmt(
            database,
            "SELECT AddressID, GPS_X, GPS_Y "\
            "FROM Depot"
        );

        while (stmt.executeStep())
        {
            int current = 0;
            const std::string _addressId(stmt.getColumn(current++).getText());
            const Vector2 _position(
                stmt.getColumn(current++).getDouble(),
                stmt.getColumn(current++).getDouble()
            );

            depot._addressId = _addressId;
            depot._position  = _position;
        }
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
                    "<ERR>: Exception ( " + e.what() + " )"
                )
            )
        );
    }
}
