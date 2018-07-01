
#include "cluster.hpp"
#include "vector2.hpp"
#include "student.hpp"
#include "heap.hpp"
#include <algorithm>

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

    bool priority = [](const Cluster& A, const Cluster& B, const Cluster& target)
    {
        return evaluation(A, target) < evaluation(B, target);
    }

    while (clusters.size() > 1)
    {
        for (const auto& current : clusters)
        {
            heap<Cluster *> candidates(clusters.size() - 1UL, cmp);
            for (const auto& other : clusters)
                if (other != current)
                    candidates.push(other);

            Cluster * parent = new Cluster({ { 0.0, 0.0 } });
            for (size_t candidate = 0; candidate < bfactor; candidate++)
            {
                Cluster * child; candidates.pop(child);
                
                parent->children.push_back(child);
                parent->centroid += child->centroid;

                std::vector<Cluster *>::iterator end = clusters.begin() + lvlsize;
                clusters.erase(std::remove(clusters.begin(), end, child), end);
                lvlsize--;
            }

            parent->centroid *= 1.0 / (double) parent->children.size();
            
            clusters.push_back(parent);
        }
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
