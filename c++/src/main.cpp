
#include "Database.h"
#include "Statement.h"
#include "cluster.hpp"
#include "manager.hpp"
#include <list>
#include <iostream>
#include <memory>
#include <limits>
#include <stdexcept>
#include <cmath>
#include <unordered_map>
#include <chrono>

#include <queue>
#include <unordered_set>
#include <stack>

using Group     = Cluster<Manager::Student>;
using Buses     = std::vector<Manager::Bus>;
using Schedules = std::list<Buses>;

double _evaluation(
    SQLite::Database&,
    const Group&,
    const Group&,
    const std::string&,
    double w0, double w1);

void createSchedules1(
    const Group *,
    const Buses&,
    Schedules&);

void createSchedules2(
    const Group *,
    const Buses&,
    Schedules&);

int main(int argc, char * argv[])
{
    if (argc < 5)
    {
        std::cerr << "<ERR>: Not enough arguements" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    double w0, w1;
    try
    {
        w0 = std::stod(argv[3]);
        w1 = std::stod(argv[4]);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    std::unique_ptr<SQLite::Database> database;
    try
    {
        database = std::make_unique<SQLite::Database>(argv[1]);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    std::list<Manager::Student> students;
    Manager::load(*database, students, argv[2]); // Manager::print(students);

    auto beg = std::chrono::high_resolution_clock().now();

    const Group * group =
        Group::hierarchical(students,
            [&](const Group& A, const Group& B)
            {
                return _evaluation(*database, A, B, argv[2], w0, w1);
            });

    auto end = std::chrono::high_resolution_clock().now();

    std::chrono::duration<double, std::micro> elapsed = end - beg;
    std::cerr << "<MSG>: Elapsed time: " << elapsed.count() << " seconds" << std::endl;

    Buses buses;
    Manager::load(*database, buses); // Manager::print(buses);

    Schedules schedules;
    createSchedules1(group, buses, schedules);
    // createSchedules2(group, buses, schedules);

    delete group;

    Manager::json(argv[2], schedules);

    return 0;
}

static double haversine(const Vector2& A, const Vector2& B)
{
    auto rads = [](double degrees) { return degrees * M_PI / 180.0; };

    const double f1 = rads(A.x()), f2 = rads(B.x());
    const double l1 = rads(A.y()), l2 = rads(B.y());

    const double u1 = std::sin((f2 - f1) / 2.0), u2 = std::sin((l2 - l1) / 2.0);

    return 2.0 * 6.371 * std::asin(std::sqrt(u1 * u1 + std::cos(f1) * std::cos(f2) * u2 * u2));
}

static double intersection(const Vector2& A, const Vector2& B)
{
    const double Ax = A.x(), Ay = A.y();
    const double Bx = B.x(), By = B.y();

    const double maxX = Ax > Bx ? Ax : Bx;
    const double minY = Ay < By ? Ay : By;

    return (minY - maxX);
}

double _evaluation(
    SQLite::Database& database,
    const Group& A,
    const Group& B,
    const std::string& daypart,
    double w0, double w1)
{
    const Manager::Student * target[] = { nullptr, nullptr };
    const Manager::Student * best[]   = { nullptr, nullptr };
    double min[]                      = { -1.0,       -1.0 };

    auto nearest = [&](const Group& group)
    {
        const Manager::Student * cen = &group.centroid();
        
        double distance;
        if ((distance = haversine(cen->_position, target[0]->_position)) < min[0])
        {
            best[0] = cen; min[0] = distance;
        }

        if ((distance = haversine(cen->_position, target[0]->_position)) < min[1])
        {
            best[1] = cen; min[1] = distance;
        }
    };

    const Manager::Student& pa = A.centroid(), &pb = B.centroid();

    const double w[] = { 0.25, 0.25 }, max = std::numeric_limits<double>().max();

    std::pair<const Manager::Student *, const Manager::Student *> pair1, pair2;

    target[0] = &pa; target[1] = &pb; min[0] = min[1] = max; A.traverse(nearest);
    pair1.first = best[0]; pair2.first = best[1];

    target[0] = &pb; target[1] = &pa; min[0] = min[1] = max; B.traverse(nearest);
    pair1.second = best[0]; pair2.second = best[1];

    double dx = 0.0, dt = 0.0;

    dx += (1.0 - w0) * Manager::distance(database, *pair1.first, *pair1.second, daypart);
    dx += w0         * Manager::distance(database, *pair2.first, *pair2.second, daypart);

    dt = intersection(A.centroid()._timespan, B.centroid()._timespan);

    return (1.0 - w1) * (1.0 / dx) + w1 * dt;
}

void createSchedules1(
    const Group * group,
    const Buses& buses,
    Schedules& schedules)
{
    std::unordered_map<const Group *, std::size_t> visited;

    auto remaining = [&](const Group * target)
    {
        if (visited.find(target) == visited.end())
            visited[target] = 0UL;

        return target->size() - visited[target];
    };

    std::function<void(const Group *, Manager::Bus&)> traverse =
    [&](const Group * target, Manager::Bus& bus)
    {
        if (target->left() && target->right())
        {
            const std::size_t lrem = remaining(target->left());
            const std::size_t rrem = remaining(target->right());
            
            #ifdef __DEBUG_SCHEDULE__
            std::cout << "<DBG>: remaining(target->left()): "  << lrem << std::endl;
            std::cout << "<DBG>: remaining(target->right()): " << rrem << std::endl;
            #endif

            std::pair<const Group *, std::size_t> brem, srem;

            if (lrem > rrem)
            {
                brem.first = target->left();  brem.second = lrem;
                srem.first = target->right(); srem.second = rrem;
            }
            else
            {
                srem.first = target->left();  srem.second = lrem;
                brem.first = target->right(); brem.second = rrem;
            }

            if (brem.second && bus._students.size() <= bus._capacity)
            {
                const std::size_t pvisited = visited[brem.first];

                traverse(brem.first, bus); visited[target] += visited[brem.first] - pvisited;
            }

            if (srem.second && bus._students.size() <= bus._capacity)
            {
                const std::size_t pvisited = visited[srem.first];

                traverse(srem.first, bus); visited[target] += visited[srem.first] - pvisited;
            }
        }
        else
        {
            if (remaining(target) && bus._students.size() <= bus._capacity)
            {
                visited[target]++; bus._students.push_back(target->centroid());
            }
        }
    };

    std::size_t busId = std::numeric_limits<std::size_t>().max();
    while (remaining(group))
    {
        if (busId >= buses.size())
        {
            schedules.push_back(buses); busId = 0UL;
        }

        #ifdef __DEBUG_SCHEDULE__
        std::cout << "<DBG>: remaining(group): " << remaining(group);
        std::cout << "<DBG>: busId: " << busId << std::endl;
        std::cout << "<DBG>: schedules.size(): " << schedules.size() << std::endl;
        std::cout << std::endl;
        #endif

        Manager::Bus& bus = schedules.back()[busId]; traverse(group, bus);

        if (bus._students.size() >= bus._capacity)
            busId++;
    }
}

void createSchedules2(
    const Group * group,
    const Buses& buses,
    Schedules& schedules)
{
    std::unordered_set<const Group *> visited;

    auto assign = [&visited](const Group * group, Manager::Bus& bus)
    {
        std::stack<const Group *> adjacent; adjacent.push(group);

        do
        {
            const Group * current = adjacent.top(); adjacent.pop();

            if (visited.find(current) == visited.end())
                visited.insert(current);
            else
                continue;

            if (current->left() && current->right())
            {
                adjacent.push(current->left());
                adjacent.push(current->right());
            }
            else
            {
                bus._students.push_back(current->centroid());
            }

        } while (!adjacent.empty());
    };

    std::size_t busId = std::numeric_limits<std::size_t>().max();

    std::queue<const Group *> adjacent; adjacent.push(group);
    do
    {
        const Group * current = adjacent.front(); adjacent.pop();

        if (current->size() <= schedules.back()[busId]._capacity)
        {
            if (busId >= buses.size())
            {
                schedules.push_back(buses); busId = 0UL;
            }

            assign(current, schedules.back()[busId]);

            busId++;
        }

        if (visited.find(current) == visited.end())
            visited.insert(current);
        else
            continue;

        if (current->left() && current->right())
        {
            adjacent.push(current->left());
            adjacent.push(current->right());
        }

    } while (!adjacent.empty());
}
