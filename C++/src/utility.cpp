
#include "utility.hpp"
#include "vector2.hpp"
#include <cmath>

#define RADIUS (6.371)

double euclidean2(const Vector2& A, const Vector2& B)
{
    const double xdiff = A.x() - B.x();
    const double ydiff = A.y() - B.y();

    return xdiff * xdiff + ydiff * ydiff;
}

double haversine(const Vector2& A, const Vector2& B)
{
    auto rads = [](double degrees) { return degrees * M_PI / 180.0; };

    const double f1 = rads(A.x()), f2 = rads(B.x());
    const double l1 = rads(A.y()), l2 = rads(B.y());

    const double u1 = std::sin((f2 - f1) / 2.0), u2 = std::sin((l2 - l1) / 2.0);

    return 2.0 * RADIUS * std::asin(std::sqrt(u1 * u1 + std::cos(f1) * std::cos(f2) * u2 * u2));
}

double intersection(const Vector2& A, const Vector2& B)
{
    const double Ax = A.x(), Ay = A.y();
    const double Bx = B.x(), By = B.y();

    const double maxX = Ax > Bx ? Ax : Bx;
    const double minY = Ay < By ? Ay : By;

    return (minY - maxX);
}
