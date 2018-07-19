
#include "Database.h"
#include "Statement.h"
#include "cluster.hpp"
#include "manager.hpp"
#include <list>
#include <vector>
#include <iostream>
#include <ctime>
#include <memory>
#include <limits>
#include <stdexcept>
#include <cmath>

double _evaluation(
    SQLite::Database&,
    const Cluster<Manager::Student>&,
    const Cluster<Manager::Student>&,
    const std::string&,
    double w0, double w1);

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

    std::clock_t beg = std::clock();

    const Cluster<Manager::Student> * cluster =
        Cluster<Manager::Student>::hierarchical(students,
            [&](const Cluster<Manager::Student>& A, const Cluster<Manager::Student>& B)
            {
                return _evaluation(*database, A, B, argv[2], w0, w1);
            });

    std::cout << "\n Elapsed time: " << (std::clock() - beg) / (double) CLOCKS_PER_SEC << std::endl;

    students.clear();

    std::vector<Manager::Bus> buses;
    Manager::load(*database, buses); // Manager::print(buses);

    std::list<std::vector<Manager::Bus>> schedules; schedules.push_back(buses);
    
    std::size_t busId = 0UL;
    cluster->traverse(
        [&](const Cluster<Manager::Student>& cluster)
        {
            if (busId >= buses.size())
            {
                schedules.push_back(buses); busId = 0UL;
            }

            if (schedules.back()[busId]._students.size() < schedules.back()[busId]._capacity)
                schedules.back()[busId]._students.push_back(cluster.centroid());
            else
                busId++;
        });

    delete cluster;

    for (const auto& schedule : schedules)
        Manager::log(schedule);

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
    const Cluster<Manager::Student>& A,
    const Cluster<Manager::Student>& B,
    const std::string& daypart,
    double w0, double w1)
{
    const Manager::Student * target[] = { nullptr, nullptr };
    const Manager::Student * best[]   = { nullptr, nullptr };
    double min[]                      = { -1.0,       -1.0 };

    auto nearest = [&](const Cluster<Manager::Student>& cluster)
    {
        const Manager::Student * cen = &cluster.centroid();
        
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
