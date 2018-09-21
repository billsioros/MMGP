
#include "Database.h"
#include "Statement.h"
#include "manager.hpp"
#include "cmeans.hpp"
#include <vector>
#include <memory>

#include "wrapper.hpp"
#include <node.h>

v8::Local<v8::Array> package(v8::Isolate *, const Manager::Schedules&);

void group(const v8::FunctionCallbackInfo<v8::Value>& args)
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
                    (
                        "TypeError: function requires 2 arguements "\
                        "(" + std::to_string(args.Length()) + " given)"
                    ).c_str()
                )
            )
        );

        return;
    }

    if (!args[0]->IsString() || !args[1]->IsString())
    {
        isolate->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    isolate,
                    "TypeError: Invalid arguement(s) type "\
                    "[ group(const std::string& database, const std::string& daypart) ]"
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
    
    std::string daypart(*(v8::String::Utf8Value(args[1]->ToString())));

    std::vector<Manager::Student> students;
    Manager::load(*database, students, daypart);

    Manager::Buses buses;
    Manager::load(*database, buses);

    const std::size_t CAPACITY = static_cast<std::size_t>
    (
        std::ceil
        (
            static_cast<double>
            (
                std::accumulate
                (
                    buses.begin(),
                    buses.end(),
                    0UL,
                    [&](std::size_t currentSum, const Manager::Bus& bus)
                    {
                        return currentSum + bus._capacity;
                    }
                )
            )
            /
            static_cast<double>(buses.size())
        )
    );

    auto haversine = [](const Vector2& A, const Vector2& B)
    {
        auto rads = [](double degrees) { return degrees * M_PI / 180.0; };

        const double f1 = rads(A.x()), f2 = rads(B.x());
        const double l1 = rads(A.y()), l2 = rads(B.y());

        const double u1 = std::sin((f2 - f1) / 2.0), u2 = std::sin((l2 - l1) / 2.0);

        return 2.0 * 6.371 * std::asin(std::sqrt(u1 * u1 + std::cos(f1) * std::cos(f2) * u2 * u2));
    };

    std::unique_ptr<const std::vector<Cluster<Manager::Student>>> groups
    (
        Cluster<Manager::Student>::cmeans
        (
            students,
            CAPACITY,
            [&haversine](const Manager::Student& A, const Manager::Student& B)
            {
                return haversine(A._position, B._position);
            },
            [](const Manager::Student& student)
            {
                return 1.0;
            }
        )
    );

    Manager::Schedules schedules;

    std::size_t busId = std::numeric_limits<std::size_t>().max();
    for (const auto& group : *groups)
    {
        if (busId >= buses.size())
        {
            schedules.push_back(buses); busId = 0UL;
        }

        Manager::Bus& bus = schedules.back()[busId++];

        for (const auto& element : group.elements())
            bus._students.push_back(*element);
    }

    args.GetReturnValue().Set(package(isolate, schedules));
}

v8::Local<v8::Array> package(v8::Isolate * isolate, const Manager::Schedules& schedules)
{
    WArray wschedules(isolate);

    for (std::size_t sid = 0UL; sid < schedules.size(); sid++)
    {
        const Manager::Buses& buses = schedules[sid];

        if (buses.empty())
            continue;
        
        WArray wbuses(isolate);

        for (std::size_t bid = 0UL; bid < buses.size(); bid++)
        {
            const Manager::Bus& bus = buses[bid];

            if (bus._students.empty())
                continue;

            WArray wstudents(isolate, bus._students.size());

            for (std::size_t pid = 0UL; pid < bus._students.size(); pid++)
            {
                const Manager::Student& student = bus._students[pid];

                WObject wstudent(isolate);

                wstudent.set("studentId", student._studentId);
                wstudent.set("addressId", student._addressId);
                wstudent.set("longitude", student._position.x());
                wstudent.set("latitude",  student._position.y());
                wstudent.set("earliest",  student._timewindow.x());
                wstudent.set("latest",    student._timewindow.y());

                wstudents.set(pid, wstudent);
            }

            WObject wbus(isolate);

            wbus.set("busId",    bus._busId);
            wbus.set("students", wstudents);

            wbuses.set(bid, wbus);
        }

        wschedules.set(sid, wbuses);
    }

    return wschedules.raw();
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
    NODE_SET_METHOD(module, "exports", group);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
