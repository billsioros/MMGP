
#ifndef __CLUSTER__
#define __CLUSTER__

#include "vector2.hpp"
#include "student.hpp"
#include <list>
#include <functional>

class Cluster
{
protected:

    const Cluster * _left, * _right;

    Cluster();
    
public:

    virtual ~Cluster();

    virtual const Vector2 * centroid() const = 0;
    virtual const Student * student()  const = 0;

    static const Cluster * hierarchical
    (
        const std::list<Student>&,
        const std::function<double(const Cluster&, const Cluster&)>&
    );
};

class OCluster : public Cluster
{
    friend class Cluster;

    Student _student;

    OCluster(const Student&);

public:

    virtual const Vector2 * centroid() const { return &_student.position; }
    virtual const Student * student()  const { return &_student; }
};

class ICluster : public Cluster
{
    friend class Cluster;

    Vector2 _centroid;

    ICluster(const Vector2&);

public:

    virtual const Vector2 * centroid() const { return &_centroid; }
    virtual const Student * student()  const { return nullptr; }
};

#endif
