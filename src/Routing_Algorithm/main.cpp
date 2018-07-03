
#include "cluster.hpp"
#include "student.hpp"
#include "vector2.hpp"
#include <list>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>

// #define FRAND(min, max) ((max - min) * ((double) std::rand() / (double) RAND_MAX) + min)

// #define MIN (-1.0)
// #define MAX (+1.0)
#define SIZE (5)

static double dist(const Vector2& A, const Vector2& B)
{
    const double xdiff = A.coordinates[0] - B.coordinates[0];
    const double ydiff = A.coordinates[1] - B.coordinates[1];

    return std::sqrt(xdiff * xdiff + ydiff * ydiff);
}

int main()
{
    std::srand((unsigned)std::time(nullptr));

    std::list<Student> students;
    // for (unsigned i = 0; i < SIZE; i++)
    // {
    //     students.push_back(Student(Vector2(FRAND(MIN, MAX), FRAND(MIN, MAX))));
    // }

    students.push_back(Vector2(0.0, 0.0));
    students.push_back(Vector2(1.0, 2.0));
    students.push_back(Vector2(2.0, 1.0));
    students.push_back(Vector2(4.0, 1.0));
    students.push_back(Vector2(5.0, 0.0));
    students.push_back(Vector2(5.0, 3.0));
    std::cout << std::endl;

    for (const auto& current : students)
        for (const auto& other : students)
            if (current.id != other.id)
                std::cout << current << ' ' << other << " | " << dist(current.position, other.position) << std::endl;

    auto evaluation = [](const Cluster& A, const Cluster& B)
    {
        return 1.0 / dist(A.centroid().position, B.centroid().position);
    };

    const Cluster * cluster = Cluster::hierarchical(students, evaluation);

    std::cout << std::endl;

    cluster->traverse(std::cout);

    return 0;
}
