
#if defined _WIN32 || defined _WIN64
    #define _USE_MATH_DEFINES
    #include <cmath>
#endif

#include <SQLiteCpp.h>
#include <manager.hpp>
#include <cmeans.hpp>
#include <log.hpp>
#include <wrapper.hpp>
#include <benchmark.hpp>
#include <node.h>
#include <uv.h>

namespace VRP_GROUP
{

struct Worker
{
    Log log;
    
    std::string err;

    uv_work_t request;
    v8::Persistent<v8::Function> callback;

    std::string dbname, dayPart;

    Manager::Schedules schedules;

    static void work(uv_work_t *);
    static void completed(uv_work_t *, int);

    Worker(const std::string& name) : log(name) {}
};

void group(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate * iso = args.GetIsolate();
    
    v8::HandleScope scope(iso);

    if (args.Length() != 3)
    {
        iso->ThrowException
        (
            v8::Exception::SyntaxError
            (
                v8::String::NewFromUtf8
                (
                    iso,
                    (
                        "function requires 3 arguements "\
                        "(" + std::to_string(args.Length()) + " given)"
                    ).c_str()
                )
            )
        );

        args.GetReturnValue().Set(v8::Undefined(iso)); return;
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
                    "Invalid arguement(s) "\
                    "[ group(dbname, dayPart, callback) ]"
                )
            )
        );

        args.GetReturnValue().Set(v8::Undefined(iso)); return;
    }

    Worker * worker = new Worker("group");

    worker->log(Log::Code::Message, "Initializing worker thread...");

    worker->request.data = worker;
    worker->callback.Reset(iso, args[2].As<v8::Function>());

    worker->dbname  = *(v8::String::Utf8Value(args[0].As<v8::String>()));
    worker->dayPart = *(v8::String::Utf8Value(args[1].As<v8::String>()));

    uv_queue_work(uv_default_loop(), &worker->request, Worker::work, Worker::completed);

    args.GetReturnValue().Set(v8::Undefined(iso));
}

void Worker::work(uv_work_t * request)
{
    Worker * worker = static_cast<Worker *>(request->data);

    std::vector<Manager::Student> students;
    Manager::Buses buses;

    worker->log(Log::Code::Message, "Worker thread loading students and buses...");

    try
    {
        SQLite::Database database(worker->dbname);

        Manager::load(database, students, worker->dayPart, worker->log);

        Manager::load(database, buses, worker->log);
    }
    catch (std::exception& e)
    {
        const std::string msg
        (
            "database=" + worker->dbname + " day-part=" + worker->dayPart + " sqlitecpp-exception=" + e.what()
        );

        worker->log(Log::Code::Error, worker->err = msg);
        
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

    auto [ms, ticks, groups] = utility::benchmark
    (
        Cluster<Manager::Student>::cmeans,
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

    worker->log(Log::Code::Message, "Clustering students... (? milliseconds / ? ticks elapsed)", ms, ticks);

    const std::size_t total = std::accumulate
    (
        groups.begin(),
        groups.end(),
        0UL,
        [](std::size_t currentSum, const Cluster<Manager::Student>& group)
        {
            return currentSum + group.elements().size();
        }
    );

    if (students.size() != total)
    {
        const std::string msg
        (
            "assertion-failed=\"The number of students before and after clustering differs\""
        );

        worker->log(Log::Code::Error, worker->err = msg);
        
        return;
    }

    std::size_t busId = std::numeric_limits<std::size_t>().max();
    for (const auto& group : groups)
    {
        if (busId >= buses.size())
        {
            worker->schedules.push_back(buses); busId = 0UL;
        }

        Manager::Bus& bus = worker->schedules.back()[busId++];

        for (const auto& element : group.elements())
            bus._students.push_back(*element);
    }
}

void Worker::completed(uv_work_t * request, int status)
{
    v8::Isolate * iso = v8::Isolate::GetCurrent();

    v8::HandleScope scope(iso);

    Worker * worker = static_cast<Worker *>(request->data);

    worker->log(Log::Code::Message, "Packaging results...");

    Wrapper::Array wschedules(iso);

    for (std::size_t sid = 0UL; sid < worker->schedules.size(); sid++)
    {
        const Manager::Buses& buses = worker->schedules[sid];

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
                wstudent.set("early",     static_cast<const int32_t&>(student._timewindow.first));
                wstudent.set("late",      static_cast<const int32_t&>(student._timewindow.second));

                wstudents.set(pid, wstudent);
            }

            Wrapper::Object wbus(iso);

            wbus.set("busId",    bus._busId);
            wbus.set("students", wstudents);

            wbuses.set(bid, wbus);
        }

        wschedules.set(sid, wbuses);
    }

    worker->log(Log::Code::Message, "Invoking callback...");

    v8::Local<v8::Value> argv[] =
    {
        worker->err.empty()
        ? v8::Null(iso).As<v8::Value>()
        : v8::Exception::Error(v8::String::NewFromUtf8(iso, worker->err.c_str())),
        wschedules.raw()
    };
    
    v8::Local<v8::Function>::New(iso, worker->callback)->
        Call(iso->GetCurrentContext()->Global(), 2, argv);

    worker->callback.Reset(); delete worker;
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
    NODE_SET_METHOD(module, "exports", group);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

}
