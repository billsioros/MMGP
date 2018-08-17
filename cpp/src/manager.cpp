
#include "manager.hpp"
#include "vector2.hpp"
#include "Database.h"
#include <vector>       // std::vector
#include <bitset>       // std::bitset
#include <string>       // std::string
#include <list>         // std::list
#include <fstream>      // std::ostream
#include <iostream>     // std::cerr
#include <ctime>        // std::time etc

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
        #ifdef __DEBUG_MANAGER__
        SQLite::Statement stmt(database,
            "SELECT Student.StudentID, Student.AddressID, "\
            "Student.Monday, Student.Tuesday, Student.Wednesday, Student.Thursday, Student.Friday, "\
            "Address.GPS_X, Address.GPS_Y "\
            "FROM Student, Address "\
            "WHERE Student.AddressID = Address.AddressID AND Student.DayPart = ? LIMIT 200");
        #else
        SQLite::Statement stmt(database,
            "SELECT Student.StudentID, Student.AddressID, "\
            "Student.Monday, Student.Tuesday, Student.Wednesday, Student.Thursday, Student.Friday, "\
            "Address.GPS_X, Address.GPS_Y "\
            "FROM Student, Address "\
            "WHERE Student.AddressID = Address.AddressID AND Student.DayPart = ?");
        #endif

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
        #ifdef __DEBUG_MANAGER__
        SQLite::Statement stmt(database, "SELECT * FROM BUS LIMIT 2");
        #else
        SQLite::Statement stmt(database, "SELECT * FROM BUS");
        #endif
        
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

static std::string unique(const char * fname)
{
    time_t raw; std::time(&raw);
    
    struct std::tm * tm = std::localtime(&raw);
    
    char strtime[512UL];

    std::strftime(strtime, 511, "%Y%m%d%H%M%S", tm);

    return std::string(fname) + strtime;
}

void Manager::csv(const char * dayPart, const std::list<std::vector<Bus>>& schedules)
{
    std::ofstream csv(unique(dayPart) + ".csv");
    if (!csv.is_open())
    {
        std::cerr << "<ERR>: Unable to log bus info" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    csv << "sheduleId, busId, studentId, longitude, latitude, earliest, latest, days" << std::endl;

    std::size_t sheduleId = 0UL;
    for (const auto& buses : schedules)
    {
        if (buses.empty())
            return;

        for (const auto& bus : buses)
        {
            if (bus._students.empty())
                continue;

            for (const auto& student : bus._students)
            {
                csv
                << sheduleId << ", "
                << bus._busId << ", "
                << student._studentId << ", "
                << student._position.x() << ", "
                << student._position.y() << ", "
                << student._timespan.x() << ", "
                << student._timespan.y() << ", "
                << student._days << std::endl;
            }
        }

        sheduleId++;
    }
}

void Manager::json(const char * dayPart, const std::list<std::vector<Bus>>& schedules)
{
    std::ofstream json(unique(dayPart) + ".json");
    if (!json.is_open())
    {
        std::cerr << "<ERR>: Unable to log bus info" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    json << '[' << std::endl;
    
    std::size_t scheduleId = 0UL;
    for (const auto& buses : schedules)
    {
        json
        << "    {"                                         << std::endl
        << "        \"scheduleId\": " << scheduleId << ',' << std::endl
        << "        \"buses\": "                           << std::endl
        << "        ["                                     << std::endl;

        if (buses.empty())
            return;

        for (const auto& bus : buses)
        {
            json
            << "            {" << std::endl
            << "                \"busId\": " << "\"" << bus._busId << "\"" << ',' << std::endl
            << "                \"students\":" << std::endl
            << "                [" << std::endl;
            for (const auto& student : bus._students)
            {
                json
                << "                    {"
                << std::endl
                << "                        \"studentId\": "  << "\"" << student._studentId    << "\"" << ',' << std::endl
                << "                        \"longitude\": "          << student._position.x()         << ',' << std::endl
                << "                        \"latitude\": "           << student._position.y()         << ',' << std::endl
                << "                        \"earliest\": "           << student._timespan.x()         << ',' << std::endl
                << "                        \"latest\": "             << student._timespan.y()         << ',' << std::endl
                << "                        \"days\": "       << "\"" << student._days         << "\""        << std::endl
                << "                    }"
                << (student._studentId != bus._students.back()._studentId ? "," : "")
                << std::endl;
            }
            
            json
            << "                ]" << std::endl
            << "            }"
            << (bus._busId != buses.back()._busId ? "," : "") << std::endl;
        }

        json
        << "        ]" << std::endl
        << "    }"
        << (scheduleId < schedules.size() - 1UL ? "," : "") << std::endl;

        scheduleId++;
    }

    json << ']' << std::endl;
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