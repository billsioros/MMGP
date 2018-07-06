
#include "cluster.hpp"
#include "student.hpp"
#include "vector2.hpp"
#include <list>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define MIN (-1.0)
#define MAX (+1.0)
#define FRAND(min, max) ((max - min) * ((double) std::rand() / (double) RAND_MAX) + min)

#define SIZE (1000)

int main()
{
    std::srand((unsigned)std::time(nullptr));

    std::list<Student> students;
    for (unsigned i = 0; i < SIZE; i++)
    {
        students.push_back(Student(Vector2(FRAND(MIN, MAX), FRAND(MIN, MAX)), Vector2(FRAND(MIN, MAX), FRAND(MIN, MAX))));
    }

    // students.emplace_back(Vector2(0.0, 0.0), Vector2(7.30, 8.30));
    // students.emplace_back(Vector2(1.0, 2.0), Vector2(7.15, 7.20));
    // students.emplace_back(Vector2(2.0, 1.0), Vector2(8.05, 8.10));
    // students.emplace_back(Vector2(4.0, 1.0), Vector2(7.45, 8.15));
    // students.emplace_back(Vector2(5.0, 0.0), Vector2(7.25, 7.55));
    // students.emplace_back(Vector2(5.0, 3.0), Vector2(7.35, 8.00));
    
    std::cout << std::endl;
    
    std::cout << "+----+-----------------------+-----------------------+" << std::endl;
    std::cout << "|ID  |POSITION               |TIMESPAN               |" << std::endl;
    std::cout << "+----+-----------------------+-----------------------+" << std::endl;
    
    for (const auto& current : students)
        std::cout << current << std::endl;

    std::cout << "+----+-----------------------+-----------------------+" << std::endl;

    std::clock_t beg = std::clock();

    const Cluster * cluster = Cluster::hierarchical(students, Cluster::evaluation);

    std::cout << "\n Elapsed time: " << (std::clock() - beg) / (double) CLOCKS_PER_SEC << std::endl;

    std::cout << std::endl;

    std::cout << "+----+-----------------------+-----------------------+" << std::endl;
    std::cout << "|ID  |POSITION               |TIMESPAN               |" << std::endl;
    std::cout << "+----+-----------------------+-----------------------+" << std::endl;

    cluster->traverse([](const Cluster& cluster) { std::cout <<  cluster.centroid() << std::endl; });

    std::cout << "+----+-----------------------+-----------------------+" << std::endl;

    delete cluster;
    
    return 0;
}
