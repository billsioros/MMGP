
#include "student.hpp"
#include <fstream>

unsigned Student::_count = 0U;

Student::Student(const Vector2& _position)
:
_id(++_count), _position(_position)
{
}

std::ostream& operator<<(std::ostream& os, const Student& student)
{
    os << student._position << " " << student._id;
}
