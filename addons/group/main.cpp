
#include "Database.h"
#include "Statement.h"
#include "manager.hpp"
#include "cmeans.hpp"
#include <vector>
#include <memory>

#include "wrapper.hpp"
#include <node.h>

namespace VRP_GROUP
{

v8::Local<v8::Array> package(v8::Isolate *, const Manager::Schedules&);

void group(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate * iso = args.GetIsolate();

    if (args.Length() != 3)
    {
        iso->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    iso,
                    (
                        "TypeError: function requires 3 arguements "\
                        "(" + std::to_string(args.Length()) + " given)"
                    ).c_str()
                )
            )
        );

        return;
    }

    if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsFunction())
    {
        iso->ThrowException
        (
            v8::Exception::TypeError
            (
                v8::String::NewFromUtf8
                (
                    iso,
                    "TypeError: Invalid arguement(s) "\
                    "[ group(dbname, dayPart, callback) ]"
                )
            )
        );

        return;
    }

    std::vector<Manager::Student> students;
    Manager::Buses buses;
    
    try
    {
        SQLite::Database database
        (
            *(v8::String::Utf8Value(args[0]->ToString()))
        );

        Manager::load
        (
            database,
            students,
            *(v8::String::Utf8Value(args[1]->ToString()))
        );

        Manager::load
        (
            database,
            buses
        );
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

    std::vector<Cluster<Manager::Student>> groups = Cluster<Manager::Student>::cmeans
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
    );

    Manager::Schedules schedules;

    std::size_t busId = std::numeric_limits<std::size_t>().max();
    for (const auto& group : groups)
    {
        if (busId >= buses.size())
        {
            schedules.push_back(buses); busId = 0UL;
        }

        Manager::Bus& bus = schedules.back()[busId++];

        for (const auto& element : group.elements())
            bus._students.push_back(*element);
    }

    args.GetReturnValue().Set(package(iso, schedules));
}

v8::Local<v8::Array> package(v8::Isolate * iso, const Manager::Schedules& schedules)
{
    Wrapper::Array wschedules(iso);

    for (std::size_t sid = 0UL; sid < schedules.size(); sid++)
    {
        const Manager::Buses& buses = schedules[sid];

        if (buses.empty())
            continue;
        
        Wrapper::Array wbuses(iso);

        for (std::size_t bid = 0UL; bid < buses.size(); bid++)
        {
            const Manager::Bus& bus = buses[bid];

            if (bus._students.empty())
                continue;

            Wrapper::Array wstudents(iso, bus._students.size());

            for (std::size_t pid = 0UL; pid < bus._students.size(); pid++)
            {
                const Manager::Student& student = bus._students[pid];

                Wrapper::Object wstudent(iso);

                wstudent.set("studentId", student._studentId);
                wstudent.set("addressId", student._addressId);
                wstudent.set("longitude", student._position.x());
                wstudent.set("latitude",  student._position.y());
                wstudent.set("earliest",  student._timewindow.x());
                wstudent.set("latest",    student._timewindow.y());

                wstudents.set(pid, wstudent);
            }

            Wrapper::Object wbus(iso);

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

}
