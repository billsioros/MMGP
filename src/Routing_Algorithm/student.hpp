
#ifndef __STUDENT__
#define __STUDENT__

#include "vector2.hpp"
#include <iosfwd>

struct Student
{
    static unsigned currentId;

    Student(const Vector2&);

    Vector2 position;
    unsigned id;
    
    friend std::ostream& operator<<(std::ostream&, const Student&);
};

#endif
