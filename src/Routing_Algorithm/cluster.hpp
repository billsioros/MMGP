
#ifndef __CLUSTER__
#define __CLUSTER__

#include "vector2.hpp"
#include "student.hpp"
#include <list>         // std::list<Student>
#include <functional>   // std::function<double(const Cluster&, const Cluster&)>
#include <iosfwd>       // Declaration of istream & ostream

class Cluster
{
    virtual const Vector2& centroid() const = 0;

    virtual void traverse() const = 0;

public:

    virtual ~Cluster() = 0;

    static const Cluster * hierarchical
    (
        const std::list<Student>&,
        const std::function<double(const Cluster&, const Cluster&)>&
    );

    friend std::ostream& operator<<(std::ostream&, const Cluster&);
    friend std::istream& operator>>(std::istream&, Cluster&);
};

class ICluster : public Cluster
{
    friend class Cluster;

    Vector2 _centroid;

    const Cluster * _left, * _right;

    ICluster(const Vector2& _centroid) : _centroid(_centroid), _left(nullptr), _right(nullptr) {}

    ~ICluster()
    {
        if (_left)  delete _left;
        if (_right) delete _right;
    }

    const Vector2& centroid() const { return _centroid; }

    void traverse() const;
};

class OCluster : public Cluster
{
    friend class Cluster;

    Student _student;

    OCluster(const Student& _student)  : _student(_student) {}

    const Vector2& centroid() const { return _student.position; }

    void traverse() const;
};

#endif
