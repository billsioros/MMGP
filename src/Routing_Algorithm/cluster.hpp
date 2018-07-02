
#ifndef __CLUSTER__
#define __CLUSTER__

#include "vector2.hpp"
#include "student.hpp"
#include <list>         // std::list<Student>
#include <functional>   // std::function<double(const Cluster&, const Cluster&)>
#include <iosfwd>       // Declaration of istream & ostream

class Cluster
{
protected:

    Student _centroid;

    Cluster(const Student& _centroid)  : _centroid(_centroid) {}

public:

    virtual ~Cluster() {};

    const Student& centroid() const { return _centroid; }

    static const Cluster * hierarchical
    (
        const std::list<Student>&,
        const std::function<double(const Cluster&, const Cluster&)>&
    );

    virtual void traverse() const;
};

class ICluster : public Cluster
{
    friend class Cluster;

    const Cluster * _left, * _right;

    ICluster(const Student& _centroid) : Cluster(_centroid), _left(nullptr), _right(nullptr) {}

    ~ICluster()
    {
        if (_left)  delete _left;
        if (_right) delete _right;
    }

    void traverse() const;
};

#endif
