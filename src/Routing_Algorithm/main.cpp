
#include "cluster.hpp"
#include "student.hpp"
#include "vector2.hpp"
#include <list>
#include <iostream>
#include <cstdlib>
#include <cmath>

#define FRAND(min, max) ((max - min) * (std::rand() / RAND_MAX) + min)

#define MIN (-1.0)
#define MAX (+1.0)
#define SIZE (10)

int main()
{
    std::list<Student> students;
    for (unsigned i = 0; i < SIZE; i++)
        students.emplace_back(FRAND(MIN, MAX), FRAND(MIN, MAX));

    auto evaluation = [](const Cluster& A, const Cluster& B)
    {
        const double xdiff = A.position.coordinates[0] - B.position.coordinates[0];
        const double ydiff = A.position.coordinates[1] - B.position.coordinates[1];

        return std::sqrt(xdiff * xdiff + ydiff * ydiff);
    };

    const Cluster * clusterTree = Cluster::hierarchical(students, evaluation);

    return 0;
}
