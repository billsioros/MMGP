
#ifndef __CLUSTER__
#define __CLUSTER__

#include "vector2.hpp"
#include "student.hpp"
#include <list>
#include <functional>

class Cluster
{
    Vector2 centroid;

    std::list<Cluster *> children;

public:

    Cluster(const Student&);
    ~Cluster();

    static const Cluster * hierarchical
    (
        const std::list<Student>&,
        unsigned,
        const std::function<bool(const Cluster*, const Cluster*)>&
    );
};

#endif
