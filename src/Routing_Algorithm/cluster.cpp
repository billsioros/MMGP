
#include "cluster.hpp"
#include "vector2.hpp"
#include "student.hpp"
#include "heap.hpp"
#include <list>         // std::list<Cluster *>
#include <functional>   // const std::function<double(const Cluster&, const Cluster&)>
#include <algorithm>    // std::remove_if
#include <fstream>      // Definition of istream & ostream

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
        bestMatch = new ICluster({ { 0.0, 0.0 } });
        bestMatch->_left = nullptr; bestMatch->_right = nullptr;

        // Step 1:
        // In each iteration, determine the "best-matching" cluster
        // to the current cluster, while at the same time checking
        // if it rocks the highest score so far and thus making it
        // the new "best-match"
        double bestScore = 0.0, currentScore = 0.0;
        for (const auto& current : clusters)
        {
            // Definition of "priority":
            // A lambda function used during the insertion of an element
            // to the heap, so that at the end of the insertion step
            // the "best-match" to the current cluster is on the top
            auto priority = [&](const Cluster * A, const Cluster * B)
            {
                return evaluation(*A, *current) > evaluation(*B, *current);
            };

            heap<Cluster *> candidates(clusters.size() - 1UL, priority);
            for (const auto& other : clusters)
                if (other != current)
                    candidates.push(other);

            Cluster * other; candidates.pop(other);
            
            currentScore = evaluation(*current, *other);

            if (!bestMatch->_left || !bestMatch->_right || currentScore > bestScore)
            {
                bestMatch->_left = current; bestMatch->_right = other;
                
                bestScore = currentScore;
            }
        }
        
        const Vector2& c1 = bestMatch->_left->_centroid.position;
        const Vector2& c2 = bestMatch->_right->_centroid.position;
        bestMatch->_centroid.position += (c1 + c2) / 2.0;

        // Step 2:
        // Delete the child clusters of the newly created cluster from the list
        clusters.erase(std::remove_if(clusters.begin(), clusters.end(), isBestMatch), clusters.end());

        // Step 3:
        // Add the newly created cluster to the list
        clusters.push_back(bestMatch);
    }

    return clusters.front();
}

void Cluster::traverse(std::ostream& os) const
{
    os << " ** " << _centroid << std::endl;
}

// ICluster Derived class:
void ICluster::traverse(std::ostream& os) const
{
    os << " <- " << std::endl;
    if (_left)
        _left->traverse(os);

    Cluster::traverse(os);

    os << " -> " << std::endl;
    if (_right)
        _right->traverse(os);
}
