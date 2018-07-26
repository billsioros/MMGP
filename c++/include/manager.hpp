
#pragma once

#include "vector2.hpp"
#include "Database.h"
#include <vector>       // std::vector
#include <bitset>       // std::bitset
#include <string>       // std::string
#include <list>         // std::list
#include <iosfwd>       // std::ostream

namespace Manager
{
    struct Student
    {
        std::string    _studentId;
        std::string    _addressId;
        std::bitset<5> _days;
        Vector2        _position;
        Vector2        _timespan;

        Student() {}
        Student(const char * _studentId, const char * _addressId, const bool _days[], const double _p[], const double _t[])
        :
        _studentId(_studentId), _addressId(_addressId), _position(_p[0], _p[1]), _timespan(_t[0], _t[1])
        {
            for (std::size_t i = 0UL; i < 5UL; i++)
                this->_days.set(i, _days[i]);
        }
    };

    struct Bus
    {
        std::string        _busId;
        unsigned           _number;
        unsigned           _capacity;
        std::list<Student> _students;

        Bus(const char * _busId, unsigned _number, unsigned _capacity)
        :
        _busId(_busId), _number(_number), _capacity(_capacity)
        {
        }
    };

    void load(SQLite::Database&, std::list<Student>&, const std::string&);
    void load(SQLite::Database&, std::vector<Bus>&);

    void csv(const std::vector<Bus>&);
    void json(const std::vector<Bus>&);

    double distance(SQLite::Database&, const Student&, const Student&, const std::string&);
}
