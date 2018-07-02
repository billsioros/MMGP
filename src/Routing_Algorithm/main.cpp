
#include "cluster.hpp"
#include "student.hpp"
#include "vector2.hpp"
#include <list>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>

#define FRAND(min, max) ((max - min) * (std::rand() / RAND_MAX) + min)

#define MIN (-1.0)
#define MAX (+1.0)
#define SIZE (10)

int main()
{
    std::srand((unsigned)std::time(nullptr));

    std::list<Student> students;
    for (unsigned i = 0; i < SIZE; i++)
        students.push_back(Student(Vector2(FRAND(MIN, MAX), FRAND(MIN, MAX))));

    auto evaluation = [](const Cluster& A, const Cluster& B)
    {
        const double xdiff = A.centroid().position.coordinates[0] - B.centroid().position.coordinates[0];
        const double ydiff = A.centroid().position.coordinates[1] - B.centroid().position.coordinates[1];

        return std::sqrt(xdiff * xdiff + ydiff * ydiff);
    };

    const Cluster * cluster = Cluster::hierarchical(students, evaluation);

    cluster->traverse();

    return 0;
}
