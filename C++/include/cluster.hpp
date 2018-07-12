
#pragma once

#include <list>         // std::list
#include <functional>   // std::function

template <typename T>
class Cluster
{
protected:

    std::size_t _size;

    T _centroid;

    Cluster() : _size(0UL) {}
    Cluster(const T& _centroid)  : _size(0UL), _centroid(_centroid) {}

public:

    virtual ~Cluster() {};

    const T& centroid() const { return _centroid; }

    static const Cluster * hierarchical
    (
        const std::list<T>&,
        const std::function<double(const Cluster&, const Cluster&)>&
    );

    virtual void traverse(const std::function<void(const Cluster<T>&)>& f) const;
};

template <typename T>
class ICluster : public Cluster<T>
{
    friend class Cluster<T>;

    const Cluster<T> * _left, * _right;

    ICluster() : _left(nullptr), _right(nullptr) {}

    ~ICluster()
    {
        if (_left)  delete _left;
        if (_right) delete _right;
    }

    void traverse(const std::function<void(const Cluster<T>&)>& f) const;
};

#include "cluster.ipp"
