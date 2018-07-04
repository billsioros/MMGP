
#ifndef __STUDENT__
#define __STUDENT__

#include "vector2.hpp"
#include <iosfwd>

class Student
{
    friend class Cluster;

    static unsigned _count;

    const unsigned _id;
    Vector2 _position;
    Vector2 _timespan;

public:

    Student() : _id(++_count) {}
    Student(const Vector2& _position, const Vector2& _timespan)
    :
    _id(++_count), _position(_position), _timespan(_timespan)
    {
    }
    
    unsigned id() const { return _id; }
    const Vector2& position() const { return _position; }
    const Vector2& timespan() const { return _timespan; }

    friend std::ostream& operator<<(std::ostream&, const Student&);

    Student& operator=(const Student&);
    friend Student operator+(const Student&, const Student&);
    friend Student operator/(const Student&, double);

    friend bool operator==(const Student&, const Student&);
    friend bool operator!=(const Student&, const Student&);
};

#endif
