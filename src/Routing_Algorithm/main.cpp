
#include "cluster.hpp"
#include "student.hpp"
#include "vector2.hpp"
#include <list>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <fstream>

#define FRAND(min, max) ((max - min) * ((double) std::rand() / (double) RAND_MAX) + min)

#define MIN (-1.0)
#define MAX (+1.0)
#define SIZE (10)

int main()
{
    std::srand((unsigned)std::time(nullptr));

    std::ofstream output("output.txt", std::ios_base::trunc);
    if (!output.is_open())
    {
        std::cerr << "<ERR>: Unable to open the specified file for writing" << std::endl;
    }

    std::list<Student> students;
    for (unsigned i = 0; i < SIZE; i++)
    {
        students.push_back(Student(Vector2(FRAND(MIN, MAX), FRAND(MIN, MAX))));

        output << students.back() << std::endl;
    }

    auto evaluation = [](const Cluster& A, const Cluster& B)
    {
        const double xdiff = A.centroid().position.coordinates[0] - B.centroid().position.coordinates[0];
        const double ydiff = A.centroid().position.coordinates[1] - B.centroid().position.coordinates[1];

        return 1.0 / std::sqrt(xdiff * xdiff + ydiff * ydiff);
    };

    const Cluster * cluster = Cluster::hierarchical(students, evaluation);

    output << std::endl;
    cluster->traverse(output);

    return 0;
}
