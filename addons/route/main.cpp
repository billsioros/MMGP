
#include "SQLiteCpp.h"
#include "manager.hpp"
#include "tsp.hpp"
#include "log.hpp"
#include "wrapper.hpp"
#include <benchmark.hpp>
#include <node.h>
#include <uv.h>

namespace VRP_ROUTE
{

struct Worker
{
    Log log;

    std::string err;

    uv_work_t request;
    v8::Persistent<v8::Function> callback;

    std::string dbname, dayPart;
    uint32_t departureTime, serviceTime;
    Manager::Student depot;
    std::vector<Manager::Student> students;

    tsptw<Manager::Student> path;

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
        !args[2]->IsString() || !args[3]->IsNumber() ||
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

    try
    {
        worker->departureTime = Timewindow::evaluate
        (
            *v8::String::Utf8Value(args[2].As<v8::String>())
        );
    }
    catch (std::exception& e)
    {
        worker->log(Log::Code::Error, "departure-time-exception=?", worker->err = e.what());
    }

    worker->serviceTime = args[3].As<v8::Uint32>()->Uint32Value();

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

        std::string lower, upper;
        wstudent.get("early", lower);
        wstudent.get("late",  upper);

        try
        {
            student._timewindow = Timewindow
            (
                Timewindow::evaluate(lower),
                Timewindow::evaluate(upper)
            );

            worker->students.emplace_back(student);
        }
        catch (std::exception& e)
        {
            worker->log(Log::Code::Error, "? (student=?)", worker->err = e.what(), student);
        }
    }

    uv_queue_work(uv_default_loop(), &worker->request, Worker::work, Worker::completed);

    args.GetReturnValue().Set(v8::Undefined(iso));
}

void Worker::work(uv_work_t * request)
{
    Worker * worker = static_cast<Worker *>(request->data);

    if (!worker->err.empty())
        return;

    worker->log(Log::Code::Message, "Worker thread initializing distance matrix...");

    using DVector = std::unordered_map<Manager::Student, double>;
    using DMatrix = std::unordered_map<Manager::Student, DVector>;

    DMatrix dmatrix;
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
            "database=" + worker->dbname + " day-part=" + worker->dayPart + " sqlitecpp-exception=" + e.what()
        );

        worker->log(Log::Code::Error, worker->err = msg);

        return;
    }

    // Local Optimization
    worker->log(Log::Code::Message, "Optimizing Route...");

    try
    {
        worker->path = tsptw<Manager::Student>
        (
            worker->depot,
            worker->students,
            [&worker](const Manager::Student& s)
            {
                return s == worker->depot ? 0.0 : 30.0;
            },
            [&dmatrix](const Manager::Student& A, const Manager::Student& B)
            {
                return dmatrix[A][B];
            },
            worker->departureTime,
            [](const Manager::Student& s)
            {
                return s._timewindow;
            }
        );
    }
    catch (std::exception& e)
    {
        worker->log(Log::Code::Error, worker->err = e.what());

        return;
    }

    auto [ms, ticks] = utility::benchmark
    (
        [worker]()
        {
            worker->path = worker->path.nneighbour();
            worker->path = worker->path.opt2();
            worker->path = worker->path.cannealing();
        }
    );

    worker->log(Log::Code::Message, "? milliseconds / ? ticks elapsed", ms, ticks);

    if (worker->students.size() != worker->path.elements().size())
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

    if (!worker->err.empty())
    {
        v8::Local<v8::Value> argv[] =
        {
            v8::Exception::Error(v8::String::NewFromUtf8(iso, worker->err.c_str())),
            v8::Undefined(iso).As<v8::Value>()
        };

        worker->log(Log::Code::Message, "Invoking callback...");

        v8::Local<v8::Function>::New(iso, worker->callback)->
            Call(iso->GetCurrentContext()->Global(), 2, argv);

        worker->callback.Reset(); delete worker; return;
    }

    worker->log(Log::Code::Message, "Packaging results...");

    Wrapper::Array wstudents = Wrapper::Array(iso, worker->path.elements().size());

    for (std::size_t sid = 0UL; sid < worker->path.elements().size(); sid++)
    {
        const Manager::Student& student = worker->path.elements()[sid];

        Wrapper::Object wstudent = Wrapper::Object(iso);

        wstudent.set("studentId", student._studentId);
        wstudent.set("addressId", student._addressId);

        wstudents.set(sid, wstudent);
    }

    Wrapper::Object wpath(iso);

    wpath.set("students", wstudents);
    wpath.set("cost",     worker->path.cost());
    wpath.set("penalty",  worker->path.penalty());

    v8::Local<v8::Value> argv[] =
    {
        v8::Null(iso).As<v8::Value>(),
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
