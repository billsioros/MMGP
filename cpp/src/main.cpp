
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

using Group     = Cluster<Manager::Student>;

using Cost      = std::unordered_map<const Manager::Student *, double>;
using Costs     = std::unordered_map<const Manager::Student *, Cost>;

using Buses     = std::vector<Manager::Bus>;
using Schedules = std::vector<Buses>;

TSP::path<Manager::Student> optimize(
    const Manager::Student&,
    const std::vector<Manager::Student>&,
    const std::function<double(const Manager::Student&, const Manager::Student&)>&
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

    Buses buses;
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

    auto cost = [&](const Manager::Student& A, const Manager::Student& B)
    {
        static Costs costs;
    
        Costs::const_iterator it1; Cost::const_iterator it2;
        if ((it1 = costs.find(&A)) != costs.end())
            if ((it2 = it1->second.find(&B)) != it1->second.end())
                return it2->second;

        return (costs[&A][&B] = Manager::distance(*database, A, B, args["-dp"]));
    };

    beg = std::chrono::system_clock().now();

    Schedules schedules;

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

        // TSP::path<Manager::Student> route = optimize(depot, bus._students, cost);

        // bus._students = route.second; bus._cost = route.first;

        // bus._students.erase(bus._students.begin()); bus._students.pop_back();
    }

    end = std::chrono::system_clock().now();

    std::cerr
    << "<MSG>: Elapsed time: "
    << std::chrono::duration_cast<std::chrono::seconds>(end - beg).count()
    << " seconds (Route Optimization)" << std::endl;
  
    Manager::json(args["-dp"], schedules);

    return 0;
}

TSP::path<Manager::Student> optimize(
    const Manager::Student& depot,
    const std::vector<Manager::Student>& students,
    const std::function<double(const Manager::Student&, const Manager::Student&)>& cost
)
{
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
        0.003,// 0.00003
        1000000UL
    );

    path = TSP::opt2<Manager::Student>(path.second.front(), path.second, cost);

    return path;
}
