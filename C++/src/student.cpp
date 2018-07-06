
#include "student.hpp"
#include <fstream>
#include <iomanip>

unsigned Student::_count = 0U;

std::ostream& operator<<(std::ostream& os, const Student& student)
{
    os << '|' << std::right << std::setw(4) << std::setfill(' ') << student._id << "| " << student._position << " | " << student._timespan << " |";
}

Student& Student::operator=(const Student& other)
{
    _position = other._position;
    _timespan = other._timespan;
}

Student operator+(const Student& A, const Student& B)
{
    return Student(A._position + B._position, A._timespan + B._timespan);
}

Student operator/(const Student& student, double lambda)
{
    return Student(student._position / lambda, student._timespan / lambda);
}

bool operator==(const Student& A, const Student& B)
{
    return A._id == B._id;
}

bool operator!=(const Student& A, const Student& B)
{
    return !(A == B);
}
