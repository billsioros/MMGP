
#include "manager.hpp"
#include "timewindow.hpp"
#include "vector2.hpp"
#include "Database.h"
#include "json.hpp"
#include <vector>           // std::vector
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
    const Vector2& _position,
    const Timewindow& _timewindow
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
_position(other._position),
_timewindow(other._timewindow)
{
}

Manager::Student::Student(Manager::Student&& other) noexcept
:
_studentId(std::move(other._studentId)),
_addressId(std::move(other._addressId)),
_position(std::move(other._position)),
_timewindow(std::move(other._timewindow))
{
}

Manager::Student& Manager::Student::operator=(const Student& other)
{
    _studentId   = other._studentId;
    _addressId   = other._addressId;
    _position    = other._position;
    _timewindow  = other._timewindow;

    return *this;
}

Manager::Student& Manager::Student::operator=(Student&& other) noexcept
{
    _studentId   = std::move(other._studentId);
    _addressId   = std::move(other._addressId);
    _position    = std::move(other._position);
    _timewindow  = std::move(other._timewindow);

    return *this;
}

Manager::Student::operator std::string() const
{
    return "[ " + _studentId + " " + _addressId + " ]";
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

void Manager::load(SQLite::Database& database, Student& depot, Log& log)
{
    log(Log::Code::Message, "Loading depot...");

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
    Log& log
)
{
    log(Log::Code::Message, "Loading students...");

    // SELECT Schedule.StudentID, Schedule.AddressID, Address.GPS_X, Address.GPS_Y FROM Schedule, Address, Student WHERE Student.StudentID = Schedule.StudentID AND Schedule.AddressID = Address.AddressID AND Schedule.DayPart = "Morning"

    std::string query
    (
        "SELECT Schedule.StudentID, Schedule.AddressID, "\
        /*"Monday, Tuesday, Wednesday, Thursday, Friday, "\*/
        "Address.GPS_X, Address.GPS_Y "\
        "FROM Schedule, Address, Student "\
        "WHERE Student.StudentID = Schedule.StudentID "\
        "AND Schedule.AddressID = Address.AddressID "\
        "AND Schedule.DayPart = ?"
    );

    #ifdef __DEBUG_MANAGER__
        query += " LIMIT 200";
    #endif

    SQLite::Statement stmt(database, query);

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
            log(Log::Code::Warning, "duplicate detected ?", student);

            continue;
        }

        const double x = stmt.getColumn(current++).getDouble();
        const double y = stmt.getColumn(current++).getDouble();
        const Vector2 _position(x, y);

        uint32_t lower = 0U, upper = 0U;
        // try
        // {
        //     lower = Timewindow::evaluate(stmt.getColumn(current++).getText());
        //     upper = Timewindow::evaluate(stmt.getColumn(current++).getText());
        // }
        // catch (std::exception& e)
        // {
        //     throw std::invalid_argument
        //     (
        //         std::string(e.what()) +
        //         " (student: " + static_cast<std::string>(student) + ")"
        //     );

        //     return;
        // }

        const Timewindow _timewindow(lower, upper);

        students.emplace_back(_studentId, _addressId, _position, _timewindow);
    }

    log(Log::Code::Message, "? students successfully loaded", students.size());
}

void Manager::load(SQLite::Database& database, Buses& buses, Log& log)
{
    log(Log::Code::Message, "Loading buses...");

    std::string query("SELECT * FROM BUS");

    #ifdef __DEBUG_MANAGER__
        query += " LIMIT 2";
    #endif

    SQLite::Statement stmt(database, query);
    
    while (stmt.executeStep())
    {
        buses.emplace_back
        (
            stmt.getColumn(0).getText(),
            stmt.getColumn(1).getUInt(),
            stmt.getColumn(2).getUInt()
        );
    }

    log(Log::Code::Message, "? buses successfully loaded", buses.size());
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
                            { "studentId", student._studentId         },
                            { "addressId", student._addressId         },
                            { "longitude", student._position.x()      },
                            { "latitude",  student._position.y()      },
                            { "early",  student._timewindow.first  },
                            { "late",    student._timewindow.second }
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
    Log& log
)
{
    try
    {
        SQLite::Statement stmt(database,
            "SELECT Duration "\
            "FROM " + daypart + "Distance "\
            "WHERE AddressID_1 = ? AND AddressID_2 = ?");

        stmt.bind(1, A._addressId);
        stmt.bind(2, B._addressId);

        stmt.executeStep();
        
        return stmt.getColumn(0).getDouble();
    }
    catch (std::exception& e)
    {
        log(Log::Code::Error, "Unable to retrieve the distance between ? and ?", A, B);
        
        throw;
    }
}

Manager::Student operator+(const Manager::Student& A, const Manager::Student& B)
{
    Manager::Student student;

    student._position   = A._position   + B._position;

    return student;
}

Manager::Student operator/(const Manager::Student& _student, double factor)
{
    Manager::Student student;

    student._position   = _student._position / factor;

    return student;
}

std::ostream& operator<<(std::ostream& os, const Manager::Student& student)
{
    os << "[ " << student._studentId << ' ' << student._addressId << " ]";
    
    return os;
}
