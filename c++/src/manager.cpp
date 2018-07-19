
#include "manager.hpp"
#include "vector2.hpp"
#include "Database.h"
#include <bitset>       // std::bitset
#include <string>       // std::string
#include <fstream>      // std::ostream
#include <iostream>     // std::cerr
#include <iomanip>      // std::setw & std::setfill

std::ostream& operator<<(std::ostream& os, const Manager::Student& student)
{
    os << student._studentId << ", " << student._position << ", " << student._timespan << ", " << student._days;

    return os;
}

std::ostream& operator<<(std::ostream& os, const Manager::Bus& bus)
{
    os << "Bus, " << bus._busId;
    os << ", " <<  std::right << std::setw(3) << std::setfill('0') << bus._number;
    os << ", " << std::right << std::setw(3) << std::setfill('0') << bus._capacity;
    os << ", " << std::right << std::setw(3) << std::setfill('0') << bus._students.size();

    return os;
}

void Manager::load(SQLite::Database& database, std::list<Student>& students, const std::string& daypart)
{
    bool failed = false;

    try
    {
        SQLite::Statement stmt(database,
            "SELECT name "\
            "FROM sqlite_master "\
            "WHERE type = 'table' AND name = ?");

        stmt.bind(1, daypart + "Distance");

        failed = !stmt.executeStep();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    if (failed)
    {
        std::cerr << "<ERR>: No such table" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    try
    {
        SQLite::Statement stmt(database,
            "SELECT Student.StudentID, Student.AddressID, "\
            "Student.Monday, Student.Tuesday, Student.Wednesday, Student.Thursday, Student.Friday, "\
            "Address.GPS_X, Address.GPS_Y "\
            "FROM Student, Address "\
            "WHERE Student.AddressID = Address.AddressID AND Student.DayPart = ? -- LIMIT 50");

        stmt.bind(1, daypart);

        while (stmt.executeStep())
        {
            int current = 0;

            const char * _studentId = stmt.getColumn(current++).getText();
            const char * _addressId = stmt.getColumn(current++).getText();

            bool _days[5] = { false };
            for (; current < 7; current++)
                _days[current - 2] = stmt.getColumn(current).getText()[0] == '1';

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
        std::cerr << e.what() << std::endl;
    }
}

void Manager::load(SQLite::Database& database, std::vector<Bus>& buses)
{
    try
    {
        SQLite::Statement stmt(database, "SELECT * FROM BUS");

        while (stmt.executeStep())
        {
            const char * _busId    = stmt.getColumn(0).getText();
            unsigned     _number   = stmt.getColumn(1).getUInt();
            unsigned     _capacity = stmt.getColumn(2).getUInt();

            buses.emplace_back(_busId, _number, _capacity);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void Manager::log(const std::vector<Bus>& buses)
{
    std::ofstream schedules("schedules.csv");
    if (!schedules.is_open())
    {
        std::cerr << "<ERR>: Unable to log bus info" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (buses.empty())
        return;

    schedules << "busId, studentId, studentPosition, studentTimespan, studentDays" << std::endl;

    for (const auto& bus : buses)
    {
        if (bus._students.empty())
            continue;

        for (const auto& student : bus._students)
            schedules << bus._busId << ", " << student << std::endl;
    }

    schedules << std::endl;
}

double Manager::distance(SQLite::Database& database, const Student& A, const Student& B, const std::string& daypart)
{
    bool failed = false;

    try
    {
        SQLite::Statement stmt(database,
            "SELECT Duration "\
            "FROM " + daypart + "Distance "\
            "WHERE AddressID_1 = ? AND AddressID_2 = ?");

        stmt.bind(1, A._addressId);
        stmt.bind(2, B._addressId);

        if (!(failed = !stmt.executeStep()))
            return stmt.getColumn(0).getDouble();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    if (failed)
    {
        std::cerr << "<ERR>: No such students" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
