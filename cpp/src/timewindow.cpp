
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
    if (lhours > 23U || uhours > 23U)
        throw std::invalid_argument("hours should be in the range [00, 23]");

    if (lminutes > 59U || uminutes > 59U)
        throw std::invalid_argument("minutes should be in the range [00, 59]");

    uint32_t lower = lhours * 3600U + lminutes * 60U;
    uint32_t upper = uhours * 3600U + uminutes * 60U;

    if (lower > upper)
        throw std::invalid_argument("negative time interval detected");

    first = lower; second = upper;
}

std::ostream& operator<<(std::ostream& os, const Timewindow& timewindow)
{
    uint16_t lhours, lminutes, uhours, uminutes;

    lhours = timewindow.first  / 3600U;
    uhours = timewindow.second / 3600U;

    lminutes = (timewindow.first  % 3600U) / 60U;
    uminutes = (timewindow.second % 3600U) / 60U;

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
