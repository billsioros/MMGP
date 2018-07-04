
#include "cluster.hpp"
#include "student.hpp"
#include "vector2.hpp"
#include <list>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>

// #define MIN (-1.0)
// #define MAX (+1.0)
// #define FRAND(min, max) ((max - min) * ((double) std::rand() / (double) RAND_MAX) + min)

#define SIZE (5)

int main()
{
    std::srand((unsigned)std::time(nullptr));

    std::list<Student> students;
    // for (unsigned i = 0; i < SIZE; i++)
    // {
    //     students.push_back(Student(Vector2(FRAND(MIN, MAX), FRAND(MIN, MAX))));
    // }

    students.emplace_back(Vector2(0.0, 0.0), Vector2(0.0, 0.0));
    students.emplace_back(Vector2(1.0, 2.0), Vector2(0.0, 0.0));
    students.emplace_back(Vector2(2.0, 1.0), Vector2(0.0, 0.0));
    students.emplace_back(Vector2(4.0, 1.0), Vector2(0.0, 0.0));
    students.emplace_back(Vector2(5.0, 0.0), Vector2(0.0, 0.0));
    students.emplace_back(Vector2(5.0, 3.0), Vector2(0.0, 0.0));
    
    std::cout << std::endl;
    
    std::cout << "+----+-------------------+-------------------+" << std::endl;
    std::cout << "|ID  |TIMESPAN           |POSITION           |" << std::endl;
    std::cout << "+----+-------------------+-------------------+" << std::endl;
    
    for (const auto& current : students)
        std::cout << current << std::endl;

    std::cout << "+----+-------------------+-------------------+" << std::endl;

    const Cluster * cluster = Cluster::hierarchical(students, Cluster::evaluation);

    std::cout << std::endl;

    cluster->traverse(std::cout);

    return 0;
}
