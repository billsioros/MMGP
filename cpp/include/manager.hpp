
#pragma once

#include "vector2.hpp"
#include "Database.h"
#include "json.hpp"
#include <vector>           // std::vector
#include <bitset>           // std::bitset
#include <string>           // std::string
#include <vector>           // std::vector
#include <iosfwd>           // std::ostream

namespace Manager
{
    struct Student
    {
        std::string    _studentId;
        std::string    _addressId;
        std::bitset<5> _days;
        Vector2        _position;
        Vector2        _timespan;

        Student();
        Student(const Student&);
        Student(
            const std::string&,
            const std::string&,
            const std::bitset<5>&,
            const Vector2&,
            const Vector2&
        );

        Student& operator=(const Student&);

        friend bool operator==(const Student& A, const Student& B)
        {
            return A._studentId == B._studentId && A._addressId == B._addressId;
        }

        friend bool operator!=(const Student& A, const Student& B)
        {
            return !(A == B);
        }
    };

    struct Bus
    {
        std::string          _busId;
        unsigned             _number;
        unsigned             _capacity;
        std::vector<Student> _students;
        double               _cost;

        Bus(const std::string&, unsigned, unsigned);
    };

    using Buses     = std::vector<Bus>;
    using Schedules = std::vector<Buses>;

    void load(SQLite::Database&, Student&);
    void load(SQLite::Database&, std::vector<Student>&, const std::string&);
    void load(SQLite::Database&, std::vector<Bus>&);

    nlohmann::json json(const std::string&, const Schedules&);

    double distance(
        SQLite::Database&,
        const Student&,
        const Student&,
        const std::string&
    );
}

Manager::Student operator+(const Manager::Student&, const Manager::Student&);
Manager::Student operator/(const Manager::Student&, double);

std::ostream& operator<<(std::ostream&, const Manager::Student&);

namespace std
{
    template <> struct hash<Manager::Student>
    {
        std::size_t operator()(const Manager::Student& P) const noexcept
        {
            return std::hash<std::string>{}("SID" + P._studentId + "AID" + P._addressId);
        }
    };
}
