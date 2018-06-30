
#include "cluster.hpp"
#include "vector2.hpp"
#include "heap.hpp"
#include <vector>

Cluster::Cluster(const Student& student)
:
centroid(student.position)
{
}

Cluster::~Cluster()
{
    for (auto& child : children)
        if (child)
            delete child;
}

const Cluster * Cluster::hierarchical
(
    const std::list<Student>& students,
    unsigned bfactor,
    const std::function<bool(const Cluster*, const Cluster*)>& evaluation
)
{
   std::vector<Cluster *> clusters;
   for (auto& student : students)
       clusters.emplace_back(student);

    while (clusters.size() > 1)
    {
        size_t lvlsize = clusters.size();
        for (size_t current = 0U; current < lvlsize; current++)
        {
            heap<Cluster *> candidates(lvlsize - current, evaluation);
            for (size_t other = current + 1U; other < lvlsize; other++)
                candidates.push(clusters[other]);

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
