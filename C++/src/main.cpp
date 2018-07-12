
#include "Database.h"
#include "Statement.h"
#include "cluster.hpp"
#include "student.hpp"
#include "utility.hpp"
#include <list>
#include <iostream>
#include <ctime>
#include <memory>
#include <limits>
#include <stdexcept>

double _evaluation(const Cluster&, const Cluster&);

static std::unique_ptr<SQLite::Database> db;
static const char * dayPart = "Morning";

int main(int argc, char * argv[])
{
    std::list<Student> students;

    try
    {
        db.reset(new SQLite::Database(argv[1]));

        SQLite::Statement stmt(*db,
            "SELECT Student.StudentID, Student.AddressID, "\
            "Student.Monday, Student.Tuesday, Student.Wednesday, Student.Thursday, Student.Friday, "\
            "Address.GPS_X, Address.GPS_Y "\
            "FROM Student, Address "\
            "WHERE Student.AddressID = Address.AddressID and Student.DayPart = ? ");

        stmt.bind(1, dayPart);

        while (stmt.executeStep())
        {
            int current = 0;

            const char * _studentId = stmt.getColumn(current++).getText();
            const char * _addressId = stmt.getColumn(current++).getText();

            bool _days[5];
            for (; current < 7; current++)
                _days[current] = stmt.getColumn(current).getText()[0] == '1';

            double _p[] =
            {
                stmt.getColumn(current++).getDouble(),
                stmt.getColumn(current++).getDouble()
            };

            double _t[] =
            {
                0.0,
                0.0
            };
            // {
            //     stmt.getColumn(current++).getDouble(),
            //     stmt.getColumn(current++).getDouble()
            // };

            students.emplace_back(_studentId, _addressId, _days, _p, _t);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "<ERR>: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
    
    std::cout << "+-----------------------+-------------------------+-------+" << std::endl;
    std::cout << "|POSITION               |TIMESPAN                 |DAYS   +" << std::endl;
    std::cout << "+-----------------------+-------------------------+-------+" << std::endl;
    
    for (const auto& current : students)
        std::cout << current << std::endl;

    std::cout << "+-----------------------+--------------------------+------+" << std::endl;

    // CLUSTERING

    std::clock_t beg = std::clock();

    const Cluster * cluster = Cluster::hierarchical(students, _evaluation);

    std::cerr << "\n Elapsed time: " << (std::clock() - beg) / (double) CLOCKS_PER_SEC << std::endl;

    std::cout << std::endl;

    std::cout << "+-----------------------+-------------------------+-------+" << std::endl;
    std::cout << "|POSITION               |TIMESPAN                 |DAYS   +" << std::endl;
    std::cout << "+-----------------------+-------------------------+-------+" << std::endl;

    cluster->traverse([](const Cluster& cluster) { std::cout <<  cluster.centroid() << std::endl; });

    std::cout << "+-----------------------+-------------------------+-------+" << std::endl;

    delete cluster;
    
    return 0;
}

double _evaluation(const Cluster& A, const Cluster& B)
{
    auto DB_Distance = [&](const Student& A, const Student& B)
    {
        try
        {
            SQLite::Statement stmt(*db, "SELECT Duration FROM " + std::string(dayPart) + "Distance" + " WHERE AddressID_1 = ? and AddressID_2 = ?");

            stmt.bind(1, A._addressId);
            stmt.bind(2, B._addressId);

            if (!stmt.executeStep())
                throw std::out_of_range("<ERR>: Existing students don' t have a recorded distance");

            return stmt.getColumn(0).getDouble();
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    };

    const Student * target[] = { nullptr, nullptr };
    const Student * best[]   = { nullptr, nullptr };
    double min[]             = { -1.0,       -1.0 };

    auto nearest = [&](const Cluster& cluster)
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
