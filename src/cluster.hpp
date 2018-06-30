
#ifndef __CLUSTER__
#define __CLUSTER__

#include "vector2.hpp"
#include <vector>
#include <functional>

struct Student
{
    Vector2 position;
};

class Cluster
{
    Vector2 centroid;

    std::vector<Cluster *> children;

public:

    Cluster();
    Cluster(const Vector2&);
    ~Cluster();

    static const Cluster * hierarchical
    (
        const std::vector<Vector2>&,
        unsigned,
        const std::function<bool(const Cluster*, const Cluster*)>&
    );
};

#endif
