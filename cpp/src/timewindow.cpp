
#include "timewindow.hpp"
#include <utility>          // std::pair
#include <cstdint>          // uint32
#include <fstream>          // std::ostream
#include <stdexcept>        // std::invalid_argument
#include <iomanip>          // std::setw

Timewindow::Timewindow()
:
std::pair<uint32_t, uint32_t>(0U, 0U)
{
}

Timewindow::Timewindow(uint32_t lower, uint32_t upper)
:
std::pair<uint32_t, uint32_t>(0U, 0U)
{
    if (lower > 86340U)
        throw std::invalid_argument("the lower bound is not in the range [00:00, 23:59]");

    if (upper > 86340U)
        throw std::invalid_argument("the upper bound is not in the range [00:00, 23:59]");

    if (lower > upper)
        throw std::invalid_argument("negative time interval detected");

    first = lower; second = upper;
}

Timewindow::Timewindow(uint8_t lhours, uint8_t lminutes, uint8_t uhours, uint8_t uminutes)
:
std::pair<uint32_t, uint32_t>(0U, 0U)
{
    uint32_t lower = evaluate(lhours, lminutes);
    uint32_t upper = evaluate(uhours, uminutes);

    if (lower > upper)
        throw std::invalid_argument("negative time interval detected");

    first = lower; second = upper;
}

std::ostream& operator<<(std::ostream& os, const Timewindow& timewindow)
{
    uint8_t lhours, lminutes, uhours, uminutes;

    lhours = static_cast<uint8_t>(timewindow.first  / 3600U);
    uhours = static_cast<uint8_t>(timewindow.second / 3600U);

    lminutes = static_cast<uint8_t>((timewindow.first  % 3600U) / 60U);
    uminutes = static_cast<uint8_t>((timewindow.second % 3600U) / 60U);

    os
    << "["
    << std::setw(2) << std::setfill('0') << lhours
    << ":"
    << std::setw(2) << std::setfill('0') << lminutes
    << ", "
    << std::setw(2) << std::setfill('0') << uhours
    << ":"
    << std::setw(2) << std::setfill('0') << uminutes
    << "]";

    return os;
}

uint32_t Timewindow::evaluate(uint8_t hours, uint8_t minutes)
{
    if (hours > 23U)
        throw std::invalid_argument("hours should be in the range [00, 23]");

    if (minutes > 59U)
        throw std::invalid_argument("minutes should be in the range [00, 59]");

    return static_cast<uint32_t>(hours) * 3600U + static_cast<uint32_t>(minutes) * 60U;
}
