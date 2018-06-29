
#ifndef __CLUSTER__
#define __CLUSTER__

#include "vector2.hpp"
#include <unordered_set>
#include <vector>

namespace std
{
    template <>
    struct hash<Vector2>
    {
        size_t operator()(const Vector2& vec2) const noexcept
        {
            return (5939 + std::hash<double>()(vec2[0])) * 5939 + std::hash<double>()(vec2[0]);
        }
    };
}

class Cluster
{
    Cluster();

public:

    static const double maxX, maxY;

    Vector2 centroid;

    std::unordered_set<Vector2> points;

    static const std::vector<Cluster> * kmeans(const std::vector<Vector2>&, unsigned);
};

#endif
