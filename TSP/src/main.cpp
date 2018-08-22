
#include "Database.h"
#include "Statement.h"
#include "vector2.hpp"
#include "manager.hpp"
#include "tsp.hpp"
#include <unordered_map>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

using Cost      = std::unordered_map<const Manager::Student *, double>;
using Costs     = std::unordered_map<const Manager::Student *, Cost>;

std::string parse(const std::string&);

void load(
    SQLite::Database&,
    const std::string&,
    std::vector<Manager::Student>&,
    Manager::Student&);

int main(int argc, char * argv[])
{
    if (argc < 4)
    {
        std::cerr << "<MSG>: TSP <database> <Bus-Id> <Day-Part> <Schedule-Id>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string dbname(argv[1]),
                BusScheduleId(argv[2] + parse(argv[3]) + argv[4]),
                DayPart(argv[3]);

    std::unique_ptr<SQLite::Database> database;
    try
    {
        database = std::make_unique<SQLite::Database>(dbname);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    std::vector<Manager::Student> students; Manager::Student depot;

    //CHANGE DayPart TO BusScheduleId ONCE EVERYTHING IS OKAY
    load(*database, DayPart, students, depot);

    auto cost = [&](const Manager::Student& A, const Manager::Student& B)
    {
        static Costs costs;
    
        Costs::const_iterator it1; Cost::const_iterator it2;
        if ((it1 = costs.find(&A)) != costs.end())
            if ((it2 = it1->second.find(&B)) != it1->second.end())
                return it2->second;

        return (costs[&A][&B] = Manager::distance(*database, A, B, DayPart));
    };

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

    std::cout << path << std::endl;
}

std::string parse(const std::string& DayPart)
{
    if (DayPart == "Morning")
    {
        return "\u03A0";
    }
    else if (DayPart == "Noon")
    {
        return "\u039c";
    }
    else if (DayPart == "Study")
    {
        return "\u0391";
    }
    else
    {
        std::cerr << "<ERR>: There can be no such option" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void load(
    SQLite::Database& database,
    const std::string& BusScheduleId,
    std::vector<Manager::Student>& students,
    Manager::Student& depot
)
{
    try
    {
        SQLite::Statement stmt(
            database,
            "SELECT Student.StudentID, Student.AddressID, Address.GPS_X, Address.GPS_Y "\
            "FROM Student, Address "\
            "WHERE Student.AddressID = Address.AddressID AND Student.DayPart = ? LIMIT 30 -- AND Student.BusSchedule = ?"
        );

        stmt.bind(1, BusScheduleId);

        while (stmt.executeStep())
        {
            int current = 0;

            const std::string _studentId(stmt.getColumn(current++).getText());
            const std::string _addressId(stmt.getColumn(current++).getText());
            const Vector2 _position(
                stmt.getColumn(current++).getDouble(),
                stmt.getColumn(current++).getDouble()
            );

            Manager::Student student;
            student._studentId = _studentId;
            student._addressId = _addressId;
            student._position  = _position;

            students.push_back(student);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    try
    {
        SQLite::Statement stmt(
            database,
            "SELECT AddressID, GPS_X, GPS_Y "\
            "FROM Depot"
        );

        while (stmt.executeStep())
        {
            int current = 0;
            const std::string _addressId(stmt.getColumn(current++).getText());
            const Vector2 _position(
                stmt.getColumn(current++).getDouble(),
                stmt.getColumn(current++).getDouble()
            );

            depot._addressId = _addressId;
            depot._position  = _position;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
