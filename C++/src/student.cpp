
#include "student.hpp"
#include <fstream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const Student& student)
{
    os << '|' << std::right << std::setw(4) << std::setfill(' ') << student._id << "| " << student._position << " | " << student._timespan << " |";
}

bool operator==(const Student& A, const Student& B)
{
    return A._id == B._id;
}

bool operator!=(const Student& A, const Student& B)
{
    return !(A == B);
}
