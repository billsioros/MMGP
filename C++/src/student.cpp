
#include "student.hpp"
#include "vector2.hpp"
#include <bitset>
#include <string>
#include <fstream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const Student& student)
{
    os << "| " << student._studentId << " | " << student._position << " | " << student._timespan << " | " << student._days << " |";
}

std::ostream& operator<<(std::ostream& os, const Bus& bus)
{
    os << "***" << std::endl;
    os << '|' << std::right << std::setw(3) << std::setfill('0') << bus._number;
    os << '|' << std::right << std::setw(3) << std::setfill('0') << bus._capacity;
    os << '|' << std::right << std::setw(3) << std::setfill('0') << bus._students.size() << std::endl;
    os << "***" << std::endl;

    if (!bus._students.empty())
        for (const auto& student : bus._students)
            os << student << std::endl;
}
