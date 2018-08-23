
#include "manager.hpp"
#include "vector2.hpp"
#include "Database.h"
#include <vector>       // std::vector
#include <bitset>       // std::bitset
#include <string>       // std::string
#include <vector>       // std::vector
#include <fstream>      // std::ostream
#include <iostream>     // std::cerr
#include <ctime>        // std::time etc

// Student Struct:
Manager::Student::Student()
{
}

Manager::Student::Student(const Manager::Student& other)
:
_studentId(other._studentId),
_addressId(other._addressId),
_days(other._days),
_position(other._position),
_timespan(other._timespan)
{
}

Manager::Student::Student(
    const std::string& _studentId,
    const std::string& _addressId,
    const std::bitset<5>& _days,
    const Vector2& _position,
    const Vector2& _timespan
)
:
_studentId(_studentId),
_addressId(_addressId),
_position(_position),
_timespan(_timespan)
{
}

Manager::Student& Manager::Student::operator=(const Manager::Student& other)
{
    this->_studentId = other._studentId;
    this->_addressId = other._addressId;
    this->_days      = other._days;
    this->_position  = other._position;
    this->_timespan  = other._timespan;

    return *this;
}

// Bus Struct:
Manager::Bus::Bus(
    const std::string& _busId,
    unsigned _number,
    unsigned _capacity
)
:
_busId(_busId),
_number(_number),
_capacity(_capacity)
{
}

void Manager::load(
    SQLite::Database& database,
    std::vector<Student>& students,
    const std::string& daypart
)
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

            std::string _studentId(stmt.getColumn(current++).getText());
            std::string _addressId(stmt.getColumn(current++).getText());

            std::bitset<5> _days;
            for (; current < 7; current++)
                _days.set(current - 2, stmt.getColumn(current).getText()[0] == '1');

            const Vector2 _position(
                stmt.getColumn(current++).getDouble(),
                stmt.getColumn(current++).getDouble()
            );

            const Vector2 _timespan(
                0.0, // stmt.getColumn(current++).getDouble(),
                0.0  // stmt.getColumn(current++).getDouble()
            );

            students.emplace_back(_studentId, _addressId, _days, _position, _timespan);
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
            std::string _busId(stmt.getColumn(0).getText());
            unsigned _number   = stmt.getColumn(1).getUInt();
            unsigned _capacity = stmt.getColumn(2).getUInt();

            buses.emplace_back(_busId, _number, _capacity);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

static std::string unique(const std::string& fname)
{
    time_t raw; std::time(&raw);
    
    struct std::tm * tm = std::localtime(&raw);
    
    char strtime[512UL];

    std::strftime(strtime, 511, "%Y%m%d%H%M%S", tm);

    return fname + strtime;
}

void Manager::csv(
    const std::string& dayPart,
    const std::vector<std::vector<Bus>>& schedules
)
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

void Manager::json(
    const std::string& dayPart,
    const std::vector<std::vector<Bus>>& schedules
)
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

double Manager::distance(
    SQLite::Database& database,
    const Student& A,
    const Student& B,
    const std::string& daypart
)
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

Manager::Student operator+(const Manager::Student& A, const Manager::Student& B)
{
    Manager::Student student;
    
    for (std::size_t i = 0; i < A._days.size(); i++)
        student._days[i] = A._days[i] || B._days[i];

    student._position = A._position + B._position;
    student._timespan = A._timespan + B._timespan;

    return student;
}

Manager::Student operator/(const Manager::Student& _student, double factor)
{
    Manager::Student student;

    student._position = _student._position / factor;
    student._timespan = _student._timespan / factor;

    return student;
}

std::ostream& operator<<(std::ostream& os, const Manager::Student& student)
{
    os << "[ " << student._studentId << ' ' << student._addressId << " ]";
    
    return os;
}

bool operator==(const Manager::Student& A, const Manager::Student& B)
{
    return A._studentId == B._studentId;
}
