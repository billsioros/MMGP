
#include "c++/include/tsp.hpp"

#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstdlib>

struct point
{
    int first, second;
    
    point(int f, int s) : first(f), second(s) {}

    point& operator=(const point& other)
    {
        this->first = other.first; this->second = other.second;

        return *this;    
    }

    friend bool operator< (const point&, const point&);
    friend bool operator> (const point&, const point&);
    friend bool operator==(const point&, const point&);
    friend bool operator!=(const point&, const point&);
};

bool operator< (const point& A, const point& B) { return A.first < B.first || (A.first == B.first && A.second < B.second); }
bool operator> (const point& A, const point& B) { return B < A; }
bool operator==(const point& A, const point& B) { return !((A < B) || (B < A)); }
bool operator!=(const point& A, const point& B) { return !(A == B); }

#define frand(max, min) ((max - min) * ((double)std::rand() / (double)RAND_MAX) + min)

#define SIZE (30)
#define MAX  (100)

int main(int argc, char * argv[])
{
    std::srand((unsigned)std::time(nullptr));

    std::vector<point> points;

    for (std::size_t i = 0; i < SIZE; i++)
        points.emplace_back(frand(-MAX, MAX), frand(-MAX, MAX));

    auto cost =
    [](const point& A, const point& B)
    {
        double xdiff = A.first - B.first;
        double ydiff = A.second - B.second;
        
        return xdiff * xdiff + ydiff * ydiff;
    };

    point depot(0.0, 0.0);

    TSP::path<point> path1a = TSP::nearestNeighbor<point>(depot, points, cost);
    // std::cout << "\nNearest Neighbour:\n" << path1a << std::endl;



    TSP::path<point> path2a = TSP::opt2<point>(path1a.second.front(), path1a.second, cost);
    // std::cout << "\nNearest Neighbour & opt-2:\n" << path2a << std::endl;



    TSP::path<point> path1b = TSP::multiFragment<point>(depot, points, cost);
    // std::cout << "\nMulti-Fragment:\n" << path1b << std::endl;



    TSP::path<point> path2b = TSP::opt2<point>(path1b.second.front(), path1b.second, cost);
    // std::cout << "\nMulti-Fragment & opt2:\n" << path2b << std::endl;

    std::cout << "false: " << path1a.first / path1b.first << std::endl;
    std::cout << "true : " << path2a.first / path2b.first << std::endl;
}
