
#include "student.hpp"
#include "vector2.hpp"
#include <bitset>
#include <string>
#include <fstream>

std::ostream& operator<<(std::ostream& os, const Student& student)
{
    os << "| " << student._position << " | " << student._timespan << " | " << student._days << " |";
}
