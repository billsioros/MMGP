
#include "Database.h"
#include "Statement.h"
#include "cluster.hpp"
#include "student.hpp"
#include <list>
#include <vector>
#include <iostream>
#include <ctime>
#include <memory>
#include <limits>
#include <stdexcept>
#include <cmath>

static std::unique_ptr<SQLite::Database> database; static std::string daypart;

double _evaluation(const Cluster<Student>&, const Cluster<Student>&);

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cerr << "<ERR>: Not enough arguements" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    daypart = argv[2];
    try
    {
        database.reset(new SQLite::Database(argv[1]));

        SQLite::Statement stmt(*database, "SELECT name FROM sqlite_master WHERE type = 'table' AND name = ?");

        stmt.bind(1, daypart + "Distance");

        if (!stmt.executeStep())
            daypart.clear();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    if (daypart.empty())
    {
        std::cerr << "<ERR>: No such table" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::list<Student> students;

    try
    {
        SQLite::Statement stmt(*database,
            "SELECT Student.StudentID, Student.AddressID, "\
            "Student.Monday, Student.Tuesday, Student.Wednesday, Student.Thursday, Student.Friday, "\
            "Address.GPS_X, Address.GPS_Y "\
            "FROM Student, Address "\
            "WHERE Student.AddressID = Address.AddressID AND Student.DayPart = ? LIMIT 50");

        stmt.bind(1, daypart);

        while (stmt.executeStep())
        {
            int current = 0;

            const char * _studentId = stmt.getColumn(current++).getText();
            const char * _addressId = stmt.getColumn(current++).getText();

            bool _days[5] = { false };
            for (; current < 7; current++)
                _days[current] = stmt.getColumn(current).getText()[0] == '1';

            double _p[] =
            {
                stmt.getColumn(current++).getDouble(),
                stmt.getColumn(current++).getDouble()
            };

            double _t[] =
            {
                0.0, // stmt.getColumn(current++).getDouble(),
                0.0  // stmt.getColumn(current++).getDouble()
            };

            students.emplace_back(_studentId, _addressId, _days, _p, _t);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "<ERR>: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
    
    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;
    std::cout << "|ID                                    |POSITION               |TIMESPAN                 |DAYS   +" << std::endl;
    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;
    
    for (const auto& student : students)
        std::cout << student << std::endl;

    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;

    // CLUSTERING

    std::clock_t beg = std::clock();

    const Cluster<Student> * cluster = Cluster<Student>::hierarchical(students, _evaluation);

    std::cerr << "\n Elapsed time: " << (std::clock() - beg) / (double) CLOCKS_PER_SEC << std::endl;

    std::cout << std::endl;

    // std::vector<Bus> buses;
    // try
    // {
    //     SQLite::Statement stmt(*database, "SELECT * FROM BUS");

    //     while (stmt.executeStep())
    //     {
    //         const char * _busId    = stmt.getColumn(0).getText();
    //         unsigned     _number   = stmt.getColumn(1).getUInt();
    //         unsigned     _capacity = stmt.getColumn(2).getUInt();

    //         buses.emplace_back(_busId, _number, _capacity);
    //     }
    // }
    // catch (std::exception& e)
    // {
    //     std::cerr << e.what() << std::endl;
    // }

    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;
    std::cout << "|ID                                    |POSITION               |TIMESPAN                 |DAYS   +" << std::endl;
    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;
    
    // std::size_t index = 0UL;
    cluster->traverse(
        [&](const Cluster<Student>& cluster)
        {
            std::cout << cluster.centroid() << std::endl;

            // if (buses[index]._students.size() < buses[index]._capacity)
            //     buses[index]._students.push_back(cluster.centroid());
            // else
            //     index++;
        });

    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;

    delete cluster;
    
    // for (const auto& bus : buses)
    //     std::cout << bus << std::endl;

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

static double DB_Distance(const Student& A, const Student& B)
{
    try
    {
        SQLite::Statement stmt(*database, "SELECT Duration FROM " + daypart + "Distance WHERE AddressID_1 = ? AND AddressID_2 = ?");

        stmt.bind(1, A._addressId);
        stmt.bind(2, B._addressId);

        if (!stmt.executeStep())
        {
            std::cerr << "<ERR>: Existing students don' t have a recorded distance" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        return stmt.getColumn(0).getDouble();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
};

double _evaluation(const Cluster<Student>& A, const Cluster<Student>& B)
{
    const Student * target[] = { nullptr, nullptr };
    const Student * best[]   = { nullptr, nullptr };
    double min[]             = { -1.0,       -1.0 };

    auto nearest = [&](const Cluster<Student>& cluster)
    {
        const Student * cen = &cluster.centroid();
        
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

    const Student& pa = A.centroid(), &pb = B.centroid();

    const double w[] = { 0.25, 0.25 }, max = std::numeric_limits<double>().max();

    std::pair<const Student *, const Student *> pair1, pair2;

    target[0] = &pa; target[1] = &pb; min[0] = min[1] = max; A.traverse(nearest);
    pair1.first = best[0]; pair2.first = best[1];

    target[0] = &pb; target[1] = &pa; min[0] = min[1] = max; B.traverse(nearest);
    pair1.second = best[0]; pair2.second = best[1];

    double dx = 0.0, dt = 0.0;

    dx += (1.0 - w[0]) * DB_Distance(*pair1.first, *pair1.second);
    dx += w[0]         * DB_Distance(*pair2.first, *pair2.second);

    dt = intersection(A.centroid()._timespan, B.centroid()._timespan);

    return (1.0 - w[1]) * (1.0 / dx) + w[1] * dt;
}
