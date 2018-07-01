
#ifndef __VECTOR2__
#define __VECTOR2__

#include <iosfwd>

class Vector2
{
    double coordinates[2];

public:

    Vector2();
    Vector2(double, double);
    Vector2(const Vector2&);

    // Access:
    double operator[](std::size_t) const;

    // IO:
    friend std::ostream& operator<<(std::ostream&, const Vector2&);
    friend std::istream& operator>>(std::istream&, Vector2&);

    // Operations:
    Vector2& operator=(const Vector2&);
    friend Vector2 operator+(const Vector2&, const Vector2&);
    friend Vector2 operator-(const Vector2&, const Vector2&);
    friend double  operator*(const Vector2&, const Vector2&);
    friend Vector2 operator*(const Vector2&, double);
    friend Vector2 operator/(const Vector2&, double);

    Vector2& operator+=(const Vector2&);
    Vector2& operator-=(const Vector2&);
    Vector2& operator*=(double);

    friend bool operator< (const Vector2&, const Vector2&);
    friend bool operator> (const Vector2&, const Vector2&);
    friend bool operator==(const Vector2&, const Vector2&);
    friend bool operator!=(const Vector2&, const Vector2&);
};

#endif
