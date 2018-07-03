
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

public:

    Student(const Vector2&);
    
    unsigned id() const { return _id; }
    const Vector2& position() const { return _position; }

    friend std::ostream& operator<<(std::ostream&, const Student&);
};

#endif
