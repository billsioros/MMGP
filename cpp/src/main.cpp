
#include "SQLiteCpp.h"
#include "manager.hpp"
#include "cmeans.hpp"
#include "tsp.hpp"
#include "log.hpp"
#include <iostream>
#include <chrono>

using DVector = std::unordered_map<Manager::Student, double>;
using DMatrix = std::unordered_map<Manager::Student, DVector>;

tsp<Manager::Student> route
(
    const Manager::Student&,
    const std::vector<Manager::Student>&,
    const std::function<double(const Manager::Student&, const Manager::Student&)>&
);

int main(int argc, char * argv[])
{
    if (argc == 1)
    {
        std::cerr << "<MSG>: " << argv[0]
        <<
        " -dbname <path/to/database> "\
        "-dayPart <day-part> "\
        "-departureTime <departure-time> "\
        "-serviceTime <service-time>"
        <<
        std::endl;

        std::exit(EXIT_FAILURE);
    }

    std::unordered_map<std::string, std::string> args =
    {
        { "-dbname",        "" },
        { "-dayPart",       "" },
        { "-departureTime", "" },
        { "-serviceTime",   "" }
    };

    for (int i = 1; i < argc - 1; i += 2)
        if (args.find(argv[i]) != args.end())
            args[argv[i]] = argv[i + 1];

    for (const auto& arg : args)
    {
        if (arg.second.empty())
        {
            std::cerr << "<ERR>: Uninitialized arguement " << arg.first << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    const std::string dbname  = args["-dbname"];
    const std::string dayPart = args["-dayPart"];

    const double departureTime = std::stod(args["-departureTime"]);
    const double serviceTime   = std::stod(args["-serviceTime"]);

    DMatrix dmatrix;
    auto cost = [&dmatrix](const Manager::Student& A, const Manager::Student& B)
    {
        return dmatrix[A][B];
    };

    std::vector<Manager::Student> students;
    Manager::Buses buses;
    Manager::Student depot;

    Log log("MMGP");

    try
    {
        SQLite::Database database(dbname);

        Manager::load(database, students, dayPart, log);

        Manager::load(database, buses, log);

        Manager::load(database, depot, log);
        
        for (const auto& A : students)
            for (const auto& B : students)
                if (A != B)
                    dmatrix[A][B] = (A == depot ? 0.0 : serviceTime)
                                  + Manager::distance(database, A, B, dayPart, log);
    }
    catch (std::exception& e)
    {
        log
        (
            Log::Code::Error,
            "database=" + dbname + " day-part=" + dayPart + " sqlitecpp-exception=" + e.what()
        );
        
        return -1;
    }

    auto haversine = [](const Vector2& A, const Vector2& B)
    {
        auto rads = [](double degrees) { return degrees * M_PI / 180.0; };

        const double f1 = rads(A.x()), f2 = rads(B.x());
        const double l1 = rads(A.y()), l2 = rads(B.y());

        const double u1 = std::sin((f2 - f1) / 2.0), u2 = std::sin((l2 - l1) / 2.0);

        return 2.0 * 6.371 * std::asin(std::sqrt(u1 * u1 + std::cos(f1) * std::cos(f2) * u2 * u2));
    };

    log(Log::Code::Message, "Clustering students...");

    auto beg = std::chrono::high_resolution_clock::now();

    std::vector<Cluster<Manager::Student>> groups = Cluster<Manager::Student>::cmeans
    (
        students,
        24UL,
        [&haversine](const Manager::Student& A, const Manager::Student& B)
        {
            return haversine(A._position, B._position);
        },
        [](const Manager::Student& student) { return 1.0; }
    );

    auto end = std::chrono::high_resolution_clock::now();

    double diff = static_cast<double>
    (
        std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count()
    ) / 1000.0;

    log(Log::Code::Message, std::to_string(diff) + " seconds elapsed");

    log(Log::Code::Message, "Optimizing routes...");

    beg = std::chrono::high_resolution_clock::now();

    std::srand(static_cast<unsigned>(std::time(nullptr)));

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

        tsp<Manager::Student> path = route(depot, bus._students, cost);

        bus._students = path.elements(); bus._cost = path.cost();

        std::stringstream ss;

        ss
        << "Duration of route "
        << std::setw(2) << std::setfill('0') << schedules.size()
        << '.'
        << std::setw(2) << std::setfill('0') << busId
        << ": "
        << std::fixed << std::setprecision(4) << path.cost() / 60.00
        << " minutes";

        log(Log::Code::Message, ss.str());
    }

    end = std::chrono::high_resolution_clock::now();

    diff = static_cast<double>
    (
        std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count()
    ) / 1000.0;

    log(Log::Code::Message, std::to_string(diff) + " seconds elapsed");

    nlohmann::json json = Manager::json(dayPart, schedules);

    std::ofstream ofs(dayPart + Log::timestamp("%04d%02d%02d%02d%02d%02d%03lld") + ".json");
    if (!ofs.is_open())
    {
        log(Log::Code::Error, "Unable to save the data in json format");

        std::exit(EXIT_FAILURE);
    }

    ofs << std::setw(4) << json;

    return 0;
}

tsp<Manager::Student> route
(
    const Manager::Student& depot,
    const std::vector<Manager::Student>& students,
    const std::function<double(const Manager::Student&, const Manager::Student&)>& cost
)
{
    tsp<Manager::Student> path
    (
        depot,
        students,
        [&depot](const Manager::Student& s) { return s == depot ? 0.0 : 30.0; },
        cost
    );
    
    path = path.nneighbour();

    #ifdef __TEST_TSP__
    std::cerr << "NN: " << path.cost() << std::endl;
    #endif

    path = path.opt2();

    #ifdef __TEST_TSP__
    std::cerr << "OPT2: " << path.cost() << std::endl;
    #endif

    return path;
}
