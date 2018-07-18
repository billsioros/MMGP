
#pragma once

#include "heap.hpp"
#include <functional>
#include <vector>
#include <utility>
#include <fstream>
#include <algorithm>
#include <unordered_set>

template <typename T>
TSP::path<T> TSP::nearestNeighbor(
    const T& depot,
    const std::vector<T>& initial,
    const std::function<double(const T&, const T&)>& cost)
{
    path<T> path(0.0, std::vector<T>(1UL, depot));

    std::vector<T> vertices(initial);
    while (!vertices.empty())
    {
        typename std::vector<T>::const_iterator nearest = vertices.begin();
        double minCost = cost(path.second.back(), *nearest);

        typename std::vector<T>::const_iterator vertex = vertices.begin() + 1UL;
        for (; vertex != vertices.end(); vertex++)
        {
            double currentCost = cost(path.second.back(), *vertex);
            if (currentCost < minCost)
            {
                nearest = vertex; minCost = currentCost;
            }
        }

        path.second.push_back(*nearest); vertices.erase(nearest);
        path.first += minCost;
    }

    path.first += cost(path.second.back(), depot);
    path.second.push_back(depot);

    return path;
}

template <typename T>
TSP::path<T> TSP::multiFragment(
    const T& depot,
    const std::vector<T>& vertices,
    const std::function<double(const T&, const T&)>& cost)
{
    struct edge
    {
        const T * _fst, * _snd;
        double _cost;

        edge() : _fst(nullptr), _snd(nullptr), _cost(0.0) {}
        edge(const T * fst, const T * snd, double cost) : _fst(fst), _snd(snd), _cost(cost) {}

        edge& operator=(const edge& other)
        {
            this->_fst  = other._fst;
            this->_snd  = other._snd;
            this->_cost = other._cost;

            return *this;
        }
    };

    const std::size_t total = vertices.size() + 1UL;

    std::unordered_map<const T *, std::unordered_map<const T *, double>> out, in;
    for (const auto& vertex : vertices)
        out[&vertex] = in[&vertex] = std::unordered_map<const T *, double>();

    out[&depot] = in[&depot] = std::unordered_map<const T *, double>();

    auto shallAdd =
    [&](const edge& e)
    {
        if (out[e._fst].size() || in[e._snd].size())
            return false;

        const T * current = e._snd;
        for (unsigned len = 2UL; !out[current].empty(); len++)
        {
            const std::pair<const T *, double>& landing = *out[current].begin();
            if ((current = landing.first) == e._fst)
                return len == total;
        }

        return true;
    };

    heap<edge> edges(
        total * (total - 1UL),
        [](const edge& A, const edge& B) { return A._cost < B._cost; });

    for (const auto& vertex : vertices)
    {
        edges.push(edge(&vertex, &depot, cost(vertex, depot)));
        edges.push(edge(&depot, &vertex, cost(depot, vertex)));
    }

    for (const auto& fst : vertices)
        for (const auto& snd : vertices)
            if (fst != snd)
                edges.push(edge(&fst, &snd, cost(fst, snd)));

    edge top;
    while (edges.pop(top))
        if (shallAdd(top))
            out[top._fst][top._snd] = in[top._snd][top._fst] = cost(*top._fst, *top._snd);

    path<T> path(0.0, std::vector<T>(1UL, depot));

    const T * current = &depot;
    do
    {
        const std::pair<const T * const, double>& landing = (*out[current].begin());

        path.second.push_back(*(current = landing.first)); path.first += landing.second;
    } while (current != &depot);

    return path;
}

template <typename T>
TSP::path<T> TSP::opt2(
    const T& depot,
    const std::vector<T>& initial,
    const std::function<double(const T&, const T&)>& cost)
{
    auto totalCost =
    [&cost](const std::vector<T>& path)
    {
        double total = 0.0;
        for (std::size_t j = 0; j < path.size() - 1UL; j++)
            total += cost(path[j], path[j + 1UL]);

        return total;
    };

    auto opt2swap =
    [&totalCost](const path<T>& old, std::size_t i, std::size_t k)
    {
        path<T> current;
        // 1. take route[0] to route[i-1] and add them in order to new_route
        for (std::size_t j = 0UL; j < i; j++)
            current.second.push_back(old.second[j]);

        // 2. take route[i] to route[k] and add them in reverse order to new_route
        for (long long j = static_cast<long long>(k); j >= static_cast<long long>(i); j--)
            current.second.push_back(old.second[j]);

        // 3. take route[k+1] to end and add them in order to new_route
        for (std::size_t j = k + 1UL; j < old.second.size(); j++)
            current.second.push_back(old.second[j]);

        current.first = totalCost(current.second);

        return current;
    };

    path<T> best(totalCost(initial), initial);
    for (std::size_t i = 0; i < best.second.size() - 1; i++)
    {
        for (std::size_t k = i + 1UL; k < best.second.size(); k++)
        {
            if (best.second[i] == depot || best.second[k] == depot)
                continue;

            path<T> current = opt2swap(best, i, k);
            if (current.first < best.first)
                return opt2<T>(depot, current.second, cost);
        }
    }

    return best;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const TSP::path<T>& path)
{
    for (const auto& step : path.second)
    {
        os << "[ " << step.first << ", " << step.second << " ]";
        if (&step != &path.second.back())
            os << " -> ";
    }

    os << "\nTotal cost: " << path.first;

    return os;
}