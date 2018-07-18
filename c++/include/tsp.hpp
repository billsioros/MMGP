
#pragma once

#include <functional>
#include <vector>
#include <utility>
#include <iosfwd>
#include <unordered_map>

namespace TSP
{
    template <typename T>
    using path = std::pair<double, std::vector<T>>;

    template <typename T>
    path<T> nearestNeighbor(
        const T&,
        const std::vector<T>&,
        const std::function<double(const T&, const T&)>&);

    template <typename T>
    path<T> multiFragment(
        const T&,
        const std::vector<T>&,
        const std::function<double(const T&, const T&)>&);

    template <typename T>
    path<T> opt2(
        const T&,
        const std::vector<T>&,
        const std::function<double(const T&, const T&)>&);
}

template <typename T>
std::ostream& operator<<(std::ostream&, const TSP::path<T>&);

#include "tsp.ipp"
