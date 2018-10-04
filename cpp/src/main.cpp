
#include "Database.h"
#include "Statement.h"
#include "manager.hpp"
#include "cmeans.hpp"
#include "tsp.hpp"
#include "annealing.hpp"
#include "json.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <iomanip>

using DVector = std::unordered_map<Manager::Student, double>;
using DMatrix = std::unordered_map<Manager::Student, DVector>;

TSP::path<Manager::Student> tsp
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

    try
    {
        SQLite::Database database(dbname);

        Manager::load(database, students, dayPart, std::cerr);

        Manager::load(database, buses, std::cerr);

        Manager::load(database, depot, std::cerr);
        
        for (const auto& A : students)
            for (const auto& B : students)
                if (A != B)
                    dmatrix[A][B] = (A == depot ? 0.0 : serviceTime)
                                  + Manager::distance(database, A, B, dayPart, std::cerr);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        
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

    std::cerr << "<MSG>: Clustering students..." << std::endl;

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

    std::cerr << "<MSG>: " << diff << " seconds elapsed" << std::endl;

    std::cerr << "<MSG>: Optimizing routes..." << std::endl;

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

        TSP::path<Manager::Student> route = tsp
        (
            depot,
            bus._students,
            cost
        );

        bus._students = route.second; bus._cost = route.first;

        bus._students.erase(bus._students.begin()); bus._students.pop_back();

        std::cerr << "<MSG>: Duration of route "
                << std::setw(2) << std::setfill('0') << schedules.size()
                << '.'
                << std::setw(2) << std::setfill('0') << busId
                << ": "
                << std::fixed << std::setprecision(4) << route.first / 60.00
                << " minutes" << std::endl;
    }

    end = std::chrono::high_resolution_clock::now();

    diff = static_cast<double>
    (
        std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count()
    ) / 1000.0;

    std::cerr << "<MSG>: " << diff << " seconds elapsed" << std::endl;

    nlohmann::json json = Manager::json(dayPart, schedules);

    time_t raw; std::time(&raw);

    struct std::tm * tm = std::localtime(&raw);

    char strtime[512UL];

    std::strftime(strtime, 511, "%Y%m%d%H%M%S", tm);

    std::ofstream ofs(dayPart + strtime + ".json");
    if (!ofs.is_open())
    {
        std::cerr << "<ERR>: Unable to save the data in json format" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    ofs << std::setw(4) << json;

    return 0;
}

TSP::path<Manager::Student> tsp
(
    const Manager::Student& depot,
    const std::vector<Manager::Student>& students,
    const std::function<double(const Manager::Student&, const Manager::Student&)>& cost
)
{
    TSP::path<Manager::Student> path;
    
    path = TSP::nearestNeighbor<Manager::Student>(depot, students, cost);

    #ifdef __TEST_TSP__
    std::cerr << "NN: " << path.first << std::endl;
    #endif

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, cost);

    #ifdef __TEST_TSP__
    std::cerr << "OPT2: " << path.first << std::endl;
    #endif

    path = Annealing::simulated<TSP::path<Manager::Student>>(
        path,
        [&cost](const TSP::path<Manager::Student>& current)
        {
            TSP::path<Manager::Student> next(0.0, current.second);

            const std::size_t i = 1UL + std::rand() % (next.second.size() - 2UL);
            const std::size_t j = 1UL + std::rand() % (next.second.size() - 2UL);

            const Manager::Student temp(next.second[i]);
            next.second[i] = next.second[j];
            next.second[j] = temp;

            next.first = TSP::totalCost<Manager::Student>(next.second, cost);

            return next;
        },
        [](const TSP::path<Manager::Student>& path)
        {
            return path.first;
        },
        1000000000.0,
        0.0015,
        1000000UL
    );

    #ifdef __TEST_TSP__
    std::cerr << "SA: " << path.first << std::endl;
    #endif

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, cost);

    #ifdef __TEST_TSP__
    std::cerr << "OPT2-SA: " << path.first << std::endl;
    #endif

    return path;
}
