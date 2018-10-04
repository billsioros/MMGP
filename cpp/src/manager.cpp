
#include "manager.hpp"
#include "vector2.hpp"
#include "Database.h"
#include "json.hpp"
#include <vector>           // std::vector
#include <bitset>           // std::bitset
#include <string>           // std::string
#include <fstream>          // std::ostream
#include <iomanip>          // std::setw
#include <ctime>            // std::time etc
#include <unordered_set>    // std::unordered_set
#include <limits>           // std::numeric_limits<double>().max()

// Student Struct:
Manager::Student::Student()
{
}

Manager::Student::Student(
    const std::string& _studentId,
    const std::string& _addressId,
    const std::bitset<5>& _days,
    const Vector2& _position,
    const Vector2& _timewindow
)
:
_studentId(_studentId),
_addressId(_addressId),
_position(_position),
_timewindow(_timewindow)
{
}

Manager::Student::Student(const Manager::Student& other)
:
_studentId(other._studentId),
_addressId(other._addressId),
_days(other._days),
_position(other._position),
_timewindow(other._timewindow)
{
}

Manager::Student::Student(const Manager::Student&& other)
:
_studentId(std::move(other._studentId)),
_addressId(std::move(other._addressId)),
_days(std::move(other._days)),
_position(std::move(other._position)),
_timewindow(std::move(other._timewindow))
{
}

Manager::Student& Manager::Student::operator=(const Student& other)
{
    _studentId   = other._studentId;
    _addressId   = other._addressId;
    _days        = other._days;
    _position    = other._position;
    _timewindow  = other._timewindow;

    return *this;
}

Manager::Student& Manager::Student::operator=(const Student&& other)
{
    _studentId   = std::move(other._studentId);
    _addressId   = std::move(other._addressId);
    _days        = std::move(other._days);
    _position    = std::move(other._position);
    _timewindow  = std::move(other._timewindow);

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
_capacity(_capacity),
_cost(std::numeric_limits<double>().max())
{
}

void Manager::load(SQLite::Database& database, Student& depot, std::ostream& logger)
{
    logger << "<MSG>: Loading depot..." << std::endl;

    SQLite::Statement stmt(database, "SELECT AddressID, GPS_X, GPS_Y FROM Depot");

    stmt.executeStep();

    depot._addressId = stmt.getColumn(0).getText();
    depot._position  =
    {
        stmt.getColumn(1).getDouble(),
        stmt.getColumn(2).getDouble()
    };
}

void Manager::load(
    SQLite::Database& database,
    std::vector<Student>& students,
    const std::string& daypart,
    std::ostream& logger
)
{
    logger << "<MSG>: Loading students..." << std::endl;

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

    std::unordered_set<Student> set;
    while (stmt.executeStep())
    {
        int current = 0;

        std::string _studentId(stmt.getColumn(current++).getText());
        std::string _addressId(stmt.getColumn(current++).getText());

        Student student;
        student._studentId = _studentId; student._addressId = _addressId;
        if (!set.insert(student).second)
        {
            logger << "<WRN>: Duplicate student " << student << std::endl;

            continue;
        }

        std::bitset<5> _days;
        for (; current < 7; current++)
            _days.set(current - 2, stmt.getColumn(current).getText()[0] == '1');

        const double x = stmt.getColumn(current++).getDouble();
        const double y = stmt.getColumn(current++).getDouble();
        const Vector2 _position(x, y);

        const Vector2 _timewindow(
            0.0, // stmt.getColumn(current++).getDouble(),
            0.0  // stmt.getColumn(current++).getDouble()
        );

        students.emplace_back(_studentId, _addressId, _days, _position, _timewindow);
    }

    logger << "<MSG>: " << students.size() << " students successfully loaded" << std::endl;
}

void Manager::load(SQLite::Database& database, Buses& buses, std::ostream& logger)
{
    logger << "<MSG>: Loading buses..." << std::endl;

    #ifdef __DEBUG_MANAGER__
    SQLite::Statement stmt(database, "SELECT * FROM BUS LIMIT 2");
    #else
    SQLite::Statement stmt(database, "SELECT * FROM BUS");
    #endif
    
    while (stmt.executeStep())
    {
        buses.emplace_back
        (
            stmt.getColumn(0).getText(),
            stmt.getColumn(1).getUInt(),
            stmt.getColumn(2).getUInt()
        );
    }

    logger << "<MSG>: " << buses.size() << " buses successfully loaded" << std::endl;
}

nlohmann::json Manager::json(
    const std::string& dayPart,
    const Schedules& schedules
)
{
    nlohmann::json json;
    
    for (const auto& buses : schedules)
    {
        if (buses.empty())
            continue;

        json.emplace_back(nlohmann::json::object());

        for (const auto& bus : buses)
        {
            if (bus._students.empty())
                continue;

            json.back()["buses"].emplace_back
            (
                nlohmann::json::object
                (
                    {
                        { "busId", bus._busId },
                        { "cost",  bus._cost }
                    }
                )
            );

            for (const auto& student : bus._students)
            {
                json.back()["buses"].back()["students"].emplace_back
                (
                    nlohmann::json::object
                    (
                        {
                            { "studentId", student._studentId      },
                            { "addressId", student._addressId      },
                            { "longitude", student._position.x()   },
                            { "latitude",  student._position.y()   },
                            { "earliest",  student._timewindow.x() },
                            { "latest",    student._timewindow.y() }
                        }
                    )
                );
            }
        }
    }

    return json;
}

double Manager::distance(
    SQLite::Database& database,
    const Student& A,
    const Student& B,
    const std::string& daypart,
    std::ostream& logger
)
{
    logger << "<MSG>: Retrieving distance of " << A << " and " << B << std::endl;

    SQLite::Statement stmt(database,
        "SELECT Duration "\
        "FROM " + daypart + "Distance "\
        "WHERE AddressID_1 = ? AND AddressID_2 = ?");

    stmt.bind(1, A._addressId);
    stmt.bind(2, B._addressId);

    stmt.executeStep();
    
    return stmt.getColumn(0).getDouble();
}

Manager::Student operator+(const Manager::Student& A, const Manager::Student& B)
{
    Manager::Student student;
    
    for (std::size_t i = 0; i < A._days.size(); i++)
        student._days[i] = A._days[i] || B._days[i];

    student._position   = A._position   + B._position;
    student._timewindow = A._timewindow + B._timewindow;

    return student;
}

Manager::Student operator/(const Manager::Student& _student, double factor)
{
    Manager::Student student;

    student._position = _student._position / factor;
    student._timewindow = _student._timewindow / factor;

    return student;
}

std::ostream& operator<<(std::ostream& os, const Manager::Student& student)
{
    os << "[ " << student._studentId << ' ' << student._addressId << " ]";
    
    return os;
}
