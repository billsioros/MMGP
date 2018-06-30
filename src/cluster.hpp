
#ifndef __CLUSTER__
#define __CLUSTER__

#include "vector2.hpp"
// #include <unordered_set>
#include <vector>

// namespace std
// {
//     template <>
//     struct hash<Vector2>
//     {
//         size_t operator()(const Vector2& vec2) const noexcept
//         {
//             return (5939 + std::hash<double>()(vec2[0])) * 5939 + std::hash<double>()(vec2[0]);
//         }
//     };
// }

class Cluster
{
    Cluster(double, double);

public:

    Vector2 centroid;

    std::vector<Cluster *> elements;

    static const unsigned bfactor;

    static const std::vector<Cluster> * hierarchical(const std::vector<Vector2>&);
};

#endif
