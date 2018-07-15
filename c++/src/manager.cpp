
#include "manager.hpp"
#include "vector2.hpp"
#include "Database.h"
#include <bitset>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace Manager
{
    std::ostream& operator<<(std::ostream& os, const Student& student)
    {
        os << "| " << student._studentId << " | " << student._position << " | " << student._timespan << " | " << student._days << " |";

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Bus& bus)
    {
        os << "| ID " << bus._busId;
        os << " | NUMBER " <<  std::right << std::setw(3) << std::setfill('0') << bus._number;
        os << " | CAPACITY " << std::right << std::setw(3) << std::setfill('0') << bus._capacity;
        os << " | SIZE " << std::right << std::setw(3) << std::setfill('0') << bus._students.size() << " |";

        return os;
    }
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
            "WHERE Student.AddressID = Address.AddressID AND Student.DayPart = ? LIMIT 50");

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

void Manager::print(const std::list<Student>& students)
{
    if (students.empty())
        return;

    std::cout << std::endl;

    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;
    std::cout << "|ID                                    |POSITION               |TIMESPAN                 |DAYS   |" << std::endl;
    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;

    for (const auto& student : students)
        std::cout << student << std::endl;

    std::cout << "+--------------------------------------+-----------------------+-------------------------+-------+" << std::endl;

    std::cout << std::endl;
}

void Manager::print(const std::vector<Bus>& buses)
{
    if (buses.empty())
        return;

    std::cout << std::endl;

    for (const auto& bus : buses)
    {
        std::cout << "+-----------------------------------------+------------+--------------+----------+" << std::endl;

        std::cout << bus << std::endl;

        std::cout << "+-----------------------------------------+------------+--------------+----------+" << std::endl;

        print(bus._students);
    }

    std::cout << std::endl;
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
