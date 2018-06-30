
#include "cluster.hpp"
#include "vector2.hpp"
#include <unordered_set>
#include <vector>
#include <cstdlib>
#include <unordered_map>

Cluster::Cluster(double cx, double cy)
:
centroid(cx, cy)
{
}

const unsigned Cluster::bfactor = 2U;

const std::vector<Cluster> * Cluster::hierarchical(const std::vector<Vector2>& points)
{
   
}
