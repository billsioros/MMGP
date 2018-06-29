// A C++ Linear Algebra Library created by Vasileios Sioros

#include "vector2.hpp"
#include <cmath>
#include <fstream>

Vector2::Vector2()
:
coordinates({ 0.0, 0.0 })
{
}

Vector2::Vector2(double x, double y)
:
coordinates({ 0.0, 0.0 })
{
    coordinates[0] = x;
    coordinates[1] = y;
}

Vector2::Vector2(const Vector2& other)
:
coordinates({ 0.0, 0.0 })
{
    coordinates[0] = other.coordinates[0];
    coordinates[1] = other.coordinates[1];
}

// Access:
inline double Vector2::operator[](std::size_t i) const
{
    return coordinates[i];
}

// IO:
inline std::ostream& operator<<(std::ostream& out, const Vector2& vec2)
{
    out << "[ " << vec2.coordinates[0] << ' ' << vec2.coordinates[1] << " ]";
}

inline std::istream& operator>>(std::istream& in, Vector2& vec2)
{
    in >> vec2.coordinates[0] >> vec2.coordinates[1];
}

// Operations:
Vector2& Vector2::operator=(const Vector2& other)
{
    coordinates[0] = other.coordinates[0];
    coordinates[1] = other.coordinates[1];
    
    return *this;
}

Vector2 operator+(const Vector2& A, const Vector2& B)
{
    double x = A.coordinates[0] + B.coordinates[0];
    double y = A.coordinates[1] + B.coordinates[1];

    return Vector2(x, y);
}

Vector2 operator-(const Vector2& A, const Vector2& B)
{
    double x = A.coordinates[0] - B.coordinates[0];
    double y = A.coordinates[1] - B.coordinates[1];

    return Vector2(x, y);
}

inline double operator*(const Vector2& A, const Vector2& B)
{
    return A.coordinates[0] * B.coordinates[0] + A.coordinates[1] * B.coordinates[1];
}

Vector2 operator*(const Vector2& vec2, const double lambda)
{
    double x = vec2.coordinates[0] * lambda;
    double y = vec2.coordinates[1] * lambda;

    return Vector2(x, y);
}

Vector2& Vector2::operator+=(const Vector2& other)
{
    coordinates[0] += other.coordinates[0];
    coordinates[1] += other.coordinates[1];
    
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other)
{
    coordinates[0] -= other.coordinates[0];
    coordinates[1] -= other.coordinates[1];
    
    return *this;
}

Vector2& Vector2::operator*=(const double lambda)
{
    coordinates[0] *= lambda;
    coordinates[1] *= lambda;
    
    return *this;
}

inline bool operator< (const Vector2& A, const Vector2& B)
{
    return (A.coordinates[0] < B.coordinates[0]) ||
           (A.coordinates[0] == B.coordinates[0] && A.coordinates[1] < B.coordinates[1]);
}

inline bool operator> (const Vector2& A, const Vector2& B)
{
    return B < A;
}

inline bool operator==(const Vector2& A, const Vector2& B)
{
    return !(A < B) && !(B < A);
}

inline bool operator!=(const Vector2& A, const Vector2& B)
{
    return !(A == B);
}

double Vector2::dist(const Vector2& A, const Vector2& B)
{
    double xdiff = A.coordinates[0] - B.coordinates[0];
    double ydiff = A.coordinates[1] - B.coordinates[1];

    return std::sqrt(xdiff * xdiff + ydiff * ydiff);
}
