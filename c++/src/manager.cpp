
#include "manager.hpp"
#include "vector2.hpp"
#include "Database.h"
#include <bitset>       // std::bitset
#include <string>       // std::string
#include <fstream>      // std::ostream
#include <iostream>     // std::cerr
#include <iomanip>      // std::setw & std::setfill

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

void Manager::csv(const std::vector<Bus>& buses)
{
    if (buses.empty())
        return;

    std::ofstream schedules("schedules.csv");
    if (!schedules.is_open())
    {
        std::cerr << "<ERR>: Unable to log bus info" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    schedules << "busId, studentId, longitude, latitude, earliest, latest, days" << std::endl;

    for (const auto& bus : buses)
    {
        if (bus._students.empty())
            continue;

        for (const auto& student : bus._students)
        {
            schedules
            << bus._busId << ", "
            << student._studentId << ", "
            << student._position.x() << ", "
            << student._position.y() << ", "
            << student._timespan.x() << ", "
            << student._timespan.y() << ", "
            << student._days << std::endl;
    }
    }

    schedules << std::endl;
}

void Manager::json(const std::vector<Bus>& buses)
{
    if (buses.empty())
        return;

    std::ofstream schedules("schedules.js");
    if (!schedules.is_open())
    {
        std::cerr << "<ERR>: Unable to log bus info" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    schedules
    << "schedules =" << std::endl
    << '[' << std::endl;
    for (const auto& bus : buses)
    {
        schedules
        << "    {" << std::endl
        << "        \"busId\": " << "\"" << bus._busId << "\"" << ',' << std::endl
        << "        \"students\":" << std::endl
        << "        [" << std::endl;
        for (const auto& student : bus._students)
        {
            schedules
            << "            {"
            << std::endl
            << "                \"studentId\": "  << "\"" << student._studentId << "\"" << ',' << std::endl
            << "                \"longitude\": "           << student._position.x()         << ',' << std::endl
            << "                \"latitude\": "          << student._position.y()         << ',' << std::endl
            << "                \"earliest\": "           << student._timespan.x()         << ',' << std::endl
            << "                \"latest\": "             << student._timespan.y()         << ',' << std::endl
            << "                \"days\": "       << "\"" << student._days      << "\""           << std::endl
            << "            }"
            << (student._studentId != bus._students.back()._studentId ? "," : "")
            << std::endl;
        }
        
        schedules
        << "        ]" << std::endl
        << "    }"
        << (bus._busId != buses.back()._busId ? "," : "") << std::endl;
    }

   schedules << ']' << std::endl;
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
