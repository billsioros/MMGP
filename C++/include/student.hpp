
#ifndef __STUDENT__
#define __STUDENT__

#include "vector2.hpp"
#include <bitset>
#include <string>
#include <iosfwd>

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

    friend std::ostream& operator<<(std::ostream&, const Student&);
};

#endif
