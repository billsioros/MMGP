
#pragma once

#include "heap.hpp"
#include <list>             // std::list
#include <functional>       // std::function
#include <algorithm>        // std::remove_if
#include <unordered_map>    // std::unordered_map
#include <utility>          // std::pair

// Cluster Base class:

template <typename T>
using Mapping = std::unordered_map<const Cluster<T> *, double>;

template <typename T>
using Map     = std::unordered_map<const Cluster<T> *, Mapping<T>>;

template <typename T>
const Cluster<T> * Cluster<T>::hierarchical
(
    const std::list<T>& elements,
    const std::function<double(const Cluster&, const Cluster&)>& _evaluation
)
{
    // Pseudocode@: http://www.saedsayad.com/clustering_hierarchical.htm

    Map<T> evalmap;

    auto evaluation = [&](const Cluster& A, const Cluster& B)
    {
        typename Map<T>::const_iterator     mapit;
        typename Mapping<T>::const_iterator mappingit;
    
        if ((mapit = evalmap.find(&A)) != evalmap.end())
            if ((mappingit = mapit->second.find(&B)) != mapit->second.end())
                return mappingit->second;

        return (evalmap[&A][&B] = _evaluation(A, B));
    };

    // Create the initial "trivial" (one element per cluster) clusters
    std::list<Cluster *> clusters;
    for (const auto& element : elements)
    {
        clusters.push_back(new Cluster(element)); clusters.back()->_size = 1UL;
    }

    // Definition of "isBestMatch":
    // A lambda function used in order to delete the child clusters
    // of the most recently added cluster from the list of clusters
    // so that they are not available for pairing in the next iteration
    ICluster<T> * bestMatch = nullptr;
    auto isBestMatch = [&](const auto& cluster)
    {
        return cluster == bestMatch->_left || cluster == bestMatch->_right;
    };

    while (clusters.size() > 1)
    {
        bestMatch = new ICluster<T>();

        // Step 1:
        // In each iteration, determine the "best-matching" cluster
        // to the current cluster, while at the same time checking
        // if mapit rocks the highest score so far and thus making mapit
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
        
        const T&    c1 = bestMatch->_left->_centroid, c2 = bestMatch->_right->_centroid;
        const std::size_t s1 = bestMatch->_left->_size,     s2 = bestMatch->_right->_size;

        bestMatch->_size = s1 + s2;

        const double w = static_cast<double>(s1) / static_cast<double>(bestMatch->_size);

        bestMatch->_centroid._position = (c1._position * w + c2._position * (1.0 - w)) / 2.0;
        bestMatch->_centroid._timespan = (c1._timespan * w + c2._timespan * (1.0 - w)) / 2.0;
        
        evalmap.erase(bestMatch->_left); evalmap.erase(bestMatch->_right);

        // Step 2:
        // Delete the child clusters of the newly created cluster from the list
        clusters.erase(std::remove_if(clusters.begin(), clusters.end(), isBestMatch), clusters.end());

        // Step 3:
        // Add the newly created cluster to the list
        clusters.push_back(bestMatch);
    }

    return clusters.front();
}

template <typename T>
void Cluster<T>::traverse(const std::function<void(const Cluster<T>&)>& f) const
{
    f(*this);
}

// #define __DEBUG__
#ifdef  __DEBUG__

#include <iostream>

#endif

// ICluster Derived class:
template <typename T>
void ICluster<T>::traverse(const std::function<void(const Cluster<T>&)>& f) const
{
    #ifdef __DEBUG__

    std::cout << " <- " << std::endl;

    #endif

    if (_left)
        _left->traverse(f);

    #ifdef __DEBUG__

    std::cout << " -> " << std::endl;
    
    #endif

    if (_right)
        _right->traverse(f);
}
