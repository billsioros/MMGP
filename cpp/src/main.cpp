
#include "Database.h"
#include "Statement.h"
#include "manager.hpp"
#include "cmeans.hpp"
#include "tsp.hpp"
#include "sannealing.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <iomanip>

using Group = Cluster<Manager::Student>;

using DVector = std::unordered_map<Manager::Student, double>;
using DMatrix = std::unordered_map<Manager::Student, DVector>;

TSP::path<Manager::Student> tsp(
    SQLite::Database&,
    const std::string&,
    const Manager::Student&,
    const std::vector<Manager::Student>&
);

int main(int argc, char * argv[])
{
    if (argc == 1)
    {
        std::cerr << "<MSG>: " << argv[0]
        << " -db <database>"
        << " -dp <day-part>" << std::endl;

        std::exit(EXIT_FAILURE);
    }

    std::unordered_map<std::string, std::string> args =
    {
        {"-db", ""},
        {"-dp", ""}
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

    std::unique_ptr<SQLite::Database> database;
    try
    {
        database = std::make_unique<SQLite::Database>(args["-db"]);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    std::vector<Manager::Student> students;
    Manager::load(*database, students, args["-dp"]);

    Manager::Buses buses;
    Manager::load(*database, buses);

    Manager::Student depot;
    Manager::load(*database, depot);

    auto haversine = [](const Vector2& A, const Vector2& B)
    {
        auto rads = [](double degrees) { return degrees * M_PI / 180.0; };

        const double f1 = rads(A.x()), f2 = rads(B.x());
        const double l1 = rads(A.y()), l2 = rads(B.y());

        const double u1 = std::sin((f2 - f1) / 2.0), u2 = std::sin((l2 - l1) / 2.0);

        return 2.0 * 6.371 * std::asin(std::sqrt(u1 * u1 + std::cos(f1) * std::cos(f2) * u2 * u2));
    };

    auto beg = std::chrono::system_clock().now();

    std::unique_ptr<const std::vector<Group>> groups(
        Group::cmeans(
            students,
            24UL,
            [&haversine](const Manager::Student& A, const Manager::Student& B)
            {
                return haversine(A._position, B._position);
            },
            [](const Manager::Student& student) { return 1.0; }
        )
    );

    auto end = std::chrono::system_clock().now();

    std::cerr
    << "<MSG>: Elapsed time: "
    << std::chrono::duration_cast<std::chrono::seconds>(end - beg).count()
    << " seconds (Student Clustering)" << std::endl;

    beg = std::chrono::system_clock().now();

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

        TSP::path<Manager::Student> route = tsp
        (
            *database,
            args["-dp"],
            depot,
            bus._students
        );

        bus._students = route.second; bus._cost = route.first;

        bus._students.erase(bus._students.begin()); bus._students.pop_back();

        std::cerr << "<MSG>: Duration of route "
                  << schedules.size() << '.'
                  << std::setw(2) << std::setfill('0') << busId << ": "
                  << std::fixed << std::setprecision(4) << route.first / 60.00
                  << " minutes" << std::endl;
    }

    end = std::chrono::system_clock().now();

    std::cerr
    << "<MSG>: Elapsed time: "
    << std::chrono::duration_cast<std::chrono::seconds>(end - beg).count()
    << " seconds (Route Optimization)" << std::endl;

    Manager::json(args["-dp"], schedules);

    return 0;
}

TSP::path<Manager::Student> tsp(
    SQLite::Database& database,
    const std::string& daypart,
    const Manager::Student& depot,
    const std::vector<Manager::Student>& students
)
{
    DMatrix dmatrix;
    auto cost = [&](const Manager::Student& A, const Manager::Student& B)
    {
        DMatrix::const_iterator mit;
        DVector::const_iterator vit;

        if ((mit = dmatrix.find(A)) != dmatrix.end())
            if ((vit = mit->second.find(B)) != mit->second.end())
                return vit->second;

        return (dmatrix[A][B] = Manager::distance(database, A, B, daypart));
    };

    TSP::path<Manager::Student> path;
    
    path = TSP::nearestNeighbor<Manager::Student>(depot, students, cost);

    #ifdef __TEST_TSP__
    std::cout << "NN: " << path.first << std::endl;
    #endif

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, cost);

    #ifdef __TEST_TSP__
    std::cout << "OPT2: " << path.first << std::endl;
    #endif

    path = SimulatedAnnealing<TSP::path<Manager::Student>>(
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
        100000000.0,
        0.003,
        1000000UL
    );

    #ifdef __TEST_TSP__
    std::cout << "SA: " << path.first << std::endl;
    #endif

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, cost);

    #ifdef __TEST_TSP__
    std::cout << "OPT2-SA: " << path.first << std::endl;
    #endif

    return path;
}
