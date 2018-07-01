
#include "cluster.hpp"
#include "vector2.hpp"
#include "student.hpp"
#include "heap.hpp"
#include <vector>
#include <algorithm>
#include <utility>

// Cluster Base Class:
Cluster::Cluster()
:
_left(nullptr), _right(nullptr)
{
}

Cluster::~Cluster()
{
    if (_left)
        delete _left;
    
    if (_right)
        delete _right;
}

const Cluster * Cluster::hierarchical
(
    const std::list<Student>& students,
    const std::function<double(const Cluster&, const Cluster&)>& evaluation
)
{
   std::vector<Cluster *> clusters;
   for (const auto& student : students)
       clusters.push_back(new OCluster(student));

    while (clusters.size() > 1)
    {
        std::pair<Cluster *, Cluster *> nearest;
        for (const auto& current : clusters)
        {
            auto priority = [&](const Cluster * A, const Cluster * B)
            {
                return evaluation(*A, *current) < evaluation(*B, *current);
            };

            heap<Cluster *> candidates(clusters.size() - 1UL, priority);
            for (const auto& other : clusters)
                if (other != current)
                    candidates.push(other);

            Cluster * top; candidates.pop(top);
            if (!nearest.first || !nearest.second
            || (evaluation(*nearest.first, *nearest.second) < evaluation(*current, *top)))
            {
                nearest.first = current; nearest.second = top;
            }
        }
        
        Cluster * parent;
        parent = new ICluster((*nearest.first->centroid() + *nearest.second->centroid()) / 2.0);
        parent->_left  = nearest.first;
        parent->_right = nearest.second;
        
        clusters.push_back(parent);
    }

    return clusters.front();
}

// Outer Cluster Class:
OCluster::OCluster(const Student& _student)
:
Cluster(), _student(_student)
{
}

// Inner Cluster Class:
ICluster::ICluster(const Vector2& _centroid)
:
Cluster(), _centroid(_centroid)
{
}
