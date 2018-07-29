
#include "tsp.hpp"

#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <fstream>

struct point
{
    int first, second;
    
    point() : first(0), second(0) {}
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

    friend std::ostream& operator<<(std::ostream&, const point&);
};

bool operator< (const point& A, const point& B) { return A.first < B.first || (A.first == B.first && A.second < B.second); }
bool operator> (const point& A, const point& B) { return B < A; }
bool operator==(const point& A, const point& B) { return !((A < B) || (B < A)); }
bool operator!=(const point& A, const point& B) { return !(A == B); }

std::ostream& operator<<(std::ostream& os, const point& p)
{
    os << "[ " << p.first << ", " << p.second << " ]";
}

#define frand(max, min) ((max - min) * ((double)std::rand() / (double)RAND_MAX) + min)

#define SIZE (30)
#define MAX  (100)

int main()
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
    std::cout << "NN: " << path1a.first << std::endl;



    TSP::path<point> path2a = TSP::opt2<point>(path1a.second.front(), path1a.second, cost);
    std::cout << "NN & opt-2: " << path2a.first << std::endl;



    TSP::path<point> path1b = TSP::multiFragment<point>(depot, points, cost);
    std::cout << "MF: " << path1b.first << std::endl;



    TSP::path<point> path2b = TSP::opt2<point>(path1b.second.front(), path1b.second, cost);
    std::cout << "MF & opt2: " << path2b.first << std::endl;

    TSP::path<point> path1c = TSP::opt2<point>(depot, points, cost);
    std::cout << "opt2: " << path1c.first << std::endl;
}
