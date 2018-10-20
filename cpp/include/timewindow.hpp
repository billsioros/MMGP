
#pragma once

#include <utility>  // std::pair
#include <cstdint>  // uint32
#include <iosfwd>   // std::ostream

struct Timewindow : public std::pair<uint32_t, uint32_t>
{
    Timewindow();
    Timewindow(uint32_t, uint32_t);
    Timewindow(uint8_t, uint8_t, uint8_t, uint8_t);

    friend std::ostream& operator<<(std::ostream&, const Timewindow&);

    static uint32_t evaluate(uint8_t, uint8_t);
    static uint32_t evaluate(const std::string&, char delimeter = '.');
};
