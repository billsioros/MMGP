
#include "student.hpp"
#include <fstream>

unsigned Student::currentId = 0U;

Student::Student(const Vector2& position)
:
position(position), id(++currentId)
{
}

std::ostream& operator<<(std::ostream& os, const Student& student)
{
    os << student.position << ' ' << student.id;
}
