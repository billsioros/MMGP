
#include "cluster.hpp"
#include "vector2.hpp"
#include <unordered_set>
#include <vector>
#include <cstdlib>
#include <unordered_map>

static inline double range()
{
    return (2.0 * ((double)rand() / (double)RAND_MAX) - 1.0);
}

const double Cluster::maxX = 100.0, Cluster::maxY = 100.0;

Cluster::Cluster()
:
centroid(maxX * range(), maxY * range())
{
}

const std::vector<Cluster> * Cluster::kmeans(const std::vector<Vector2>& points, unsigned k)
{
    std::vector<Cluster> * clusters = new std::vector<Cluster>(k, Cluster());

    std::unordered_map<Vector2, Cluster *> owners;
    for (const auto& point : points)
        owners[point] = nullptr;

    unsigned assignments;
    do
    {
        assignments = 0;

        for (const auto& point : points)
        {
            Cluster& nearest = clusters->front();
            for (const auto& cluster : *clusters)
                if (Vector2::dist(point, cluster.centroid) < Vector2::dist(point, nearest.centroid))
                    nearest = cluster;

            Cluster * owner;
            if ((owner = owners[point]) != &nearest)
            {
                if (owner)
                    owner->points.erase(point);

                nearest.points.insert(point);

                assignments++;
            }
        }
        
        for (auto& cluster : *clusters)
        {
            cluster.centroid = Vector2(0.0, 0.0);
            for (const auto& point : cluster.points)
                cluster.centroid += point;

            cluster.centroid *= 1.0 / (double)cluster.points.size();
        }

    } while (assignments);

    return clusters;
}
