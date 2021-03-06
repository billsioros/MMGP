
#pragma once

#include <functional>   // std::hash
#include <iosfwd>       // std::istream & std::ostream

class Vector2
{
    friend struct std::hash<Vector2>;

    double _x, _y;

public:

    Vector2();
    Vector2(double, double);
    Vector2(const Vector2&);
    Vector2(Vector2&&) noexcept;

    // Access:
    double x() const { return _x; }
    double y() const { return _y; }

    // IO:
    friend std::ostream& operator<<(std::ostream&, const Vector2&);
    friend std::istream& operator>>(std::istream&, Vector2&);

    // Operations:
    Vector2& operator=(const Vector2&);
    Vector2& operator=(Vector2&&) noexcept;

    friend Vector2 operator+(const Vector2&, const Vector2&);
    friend Vector2 operator-(const Vector2&, const Vector2&);
    friend double  operator*(const Vector2&, const Vector2&);
    friend Vector2 operator*(const Vector2&, double);
    friend Vector2 operator/(const Vector2&, double);

    Vector2& operator+=(const Vector2&);
    Vector2& operator-=(const Vector2&);
    Vector2& operator*=(double);
    Vector2& operator/=(double);

    friend bool operator< (const Vector2&, const Vector2&);
    friend bool operator> (const Vector2&, const Vector2&);
    friend bool operator==(const Vector2&, const Vector2&);
    friend bool operator!=(const Vector2&, const Vector2&);
};

namespace std
{
    template <>
    struct hash<Vector2>
    {
        size_t operator()(const Vector2& v) const noexcept
        {
            return (std::hash<double>{}(v._x) ^ (std::hash<double>{}(v._y) << 1));
        }
    };
}
