
#include "cluster.hpp"
#include "vector2.hpp"
#include "utility.hpp"
#include "student.hpp"
#include "heap.hpp"
#include <list>         // std::list<Cluster *>
#include <functional>   // const std::function<double(const Cluster&, const Cluster&)>
#include <algorithm>    // std::remove_if
#include <fstream>      // Definition of istream & ostream
#include <limits>       // std::numeric_limits<double>()

// Cluster Base class:
const Cluster * Cluster::hierarchical
(
    const std::list<Student>& students,
    const std::function<double(const Cluster&, const Cluster&)>& evaluation
)
{
    // Pseudocode@: http://www.saedsayad.com/clustering_hierarchical.htm

    // Create the initial "trivial" (one student per cluster) clusters
    std::list<Cluster *> clusters;
    for (const auto& student : students)
        clusters.push_back(new Cluster(student));

    // Definition of "isBestMatch":
    // A lambda function used in order to delete the child clusters
    // of the most recently added cluster from the list of clusters
    // so that they are not available for pairing in the next iteration
    ICluster * bestMatch = nullptr;
    auto isBestMatch = [&](const auto& cluster)
    {
        return cluster == bestMatch->_left || cluster == bestMatch->_right;
    };

    while (clusters.size() > 1)
    {
        bestMatch = new ICluster();

        // Step 1:
        // In each iteration, determine the "best-matching" cluster
        // to the current cluster, while at the same time checking
        // if it rocks the highest score so far and thus making it
        // the new "best-match"
        double bestScore = std::numeric_limits<double>().min();
        for (const auto& current : clusters)
        {
            // Definition of "priority":
            // A lambda function used during the insertion of an element
            // to the heap, so that at the end of the insertion step
            // the "best-match" to the current cluster is on the top

            double currentScore = std::numeric_limits<double>().min();
            auto priority = [&](const Cluster * A, const Cluster * B)
            {
                const double ea = evaluation(*A, *current);
                const double eb = evaluation(*B, *current);

                if (ea > currentScore) { currentScore = ea; }
                if (eb > currentScore) { currentScore = eb; }

                return ea > eb;
            };

            heap<Cluster *> candidates(clusters.size() - 1UL, priority);
            for (const auto& other : clusters)
                if (other != current)
                    candidates.push(other);

            Cluster * other; candidates.pop(other);

            if (!bestMatch->_left || !bestMatch->_right || currentScore > bestScore)
            {
                bestMatch->_left = current; bestMatch->_right = other;
                
                bestScore = currentScore;
            }
        }
        
        const Student& c1 = bestMatch->_left->_centroid;
        const Student& c2 = bestMatch->_right->_centroid;
        bestMatch->_centroid = (c1 + c2) / 2.0;

        // Step 2:
        // Delete the child clusters of the newly created cluster from the list
        clusters.erase(std::remove_if(clusters.begin(), clusters.end(), isBestMatch), clusters.end());

        // Step 3:
        // Add the newly created cluster to the list
        clusters.push_back(bestMatch);
    }

    return clusters.front();
}

double Cluster::evaluation(const Cluster& A, const Cluster& B)
{
    const Vector2 * target[] = { nullptr, nullptr };
    const Vector2 * best[]   = { nullptr, nullptr };
    double min[]             = { -1.0,       -1.0 };

    auto nearest = [&](const Cluster& cluster)
    {
        const Vector2 * p = &cluster.centroid().position();
        
        double distance;
        if ((distance = haversine(*p, *target[0])) < min[0])
        {
            best[0] = p; min[0] = distance;
        }

        if ((distance = haversine(*target[1], *p)) < min[1])
        {
            best[1] = p; min[1] = distance;
        }
    };

    const Vector2& pa = A.centroid().position(), &pb = B.centroid().position();

    const double w[] = { 0.25, 0.25 }, max = std::numeric_limits<double>().max();

    std::pair<const Vector2 *, const Vector2 *> pair1, pair2;

    target[0] = &pa; target[1] = &pb; min[0] = min[1] = max; A.traverse(nearest);
    pair1.first = best[0]; pair2.first = best[1];

    target[0] = &pb; target[1] = &pa; min[0] = min[1] = max; B.traverse(nearest);
    pair1.second = best[0]; pair2.second = best[1];

    double dx = 0.0, dt = 0.0;

    dx += (1.0 - w[0]) * haversine(*pair1.first, *pair1.second);
    dx += w[0]         * haversine(*pair2.first, *pair2.second);

    dt = intersection(A.centroid().timespan(), B.centroid().timespan());

    return (1.0 - w[1]) * (1.0 / dx) + w[1] * dt;
}

void Cluster::traverse(const std::function<void(const Cluster&)>& f) const
{
    f(*this);
}

// #define __DEBUG__
#ifdef  __DEBUG__

#include <iostream>

#endif

// ICluster Derived class:
void ICluster::traverse(const std::function<void(const Cluster&)>& f) const
{
    #ifdef __DEBUG__

    std::cout << " <- " << std::endl;

    #endif

    if (_left)
        _left->traverse(f);

    Cluster::traverse(f);

    #ifdef __DEBUG__

    std::cout << " -> " << std::endl;
    
    #endif

    if (_right)
        _right->traverse(f);
}