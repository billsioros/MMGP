
#include "cluster.hpp"
#include "vector2.hpp"
#include "heap.hpp"
#include <unordered_set>
#include <vector>
#include <cstdlib>
#include <unordered_map>

Cluster::Cluster()
:
centroid(0.0, 0.0)
{
}

Cluster::Cluster(const Vector2& centroid)
:
centroid(centroid)
{
}

Cluster::~Cluster()
{
    for (auto& subcluster : children)
        if (subcluster)
            delete subcluster;
}

const Cluster * Cluster::hierarchical
(
    const std::vector<Vector2>& points,
    unsigned bfactor,
    const std::function<bool(const Cluster*, const Cluster*)>& evaluation
)
{
   std::vector<Cluster *> clusters;
   for (auto& point : points)
       clusters.push_back(new Cluster(point));

    while (clusters.size() > 1)
    {
        size_t csize = clusters.size();
        for (size_t i = 0U; i < csize; i++)
        {
            heap<Cluster *> candidates(csize - i, evaluation);
            for (size_t j = i + 1U; j < csize; j++)
                candidates.push(clusters[j]);

            Cluster * parent = new Cluster;
            for (size_t k = 0; k < bfactor; k++)
            {
                Cluster * child; candidates.pop(child);
                
                parent->children.push_back(child);
                parent->centroid += child->centroid;
            }

            parent->centroid *= 1.0 / (double) (double)parent->children.size();
            
            clusters.push_back(parent);
        }

        clusters.erase(clusters.begin(), clusters.begin() + csize);
    }

    return clusters.front();
}
