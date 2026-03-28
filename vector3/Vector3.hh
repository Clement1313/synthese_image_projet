#pragma once

#include <ostream>

class Vector3
{
public:
    float x, y, z;

    Vector3(float x, float y, float z): x(x), y(y), z(z) {};
    Vector3(): x(0), y(0), z(0) {};

    Vector3 operator*(const float &l) const;
    Vector3 operator-(const Vector3 &v ) const;
    Vector3 operator/(const float &l ) const;

    Vector3 normalized() const;
    float dot(const Vector3 &v) const;
    float norm() const;
    float scal(const Vector3& v) const;


    friend std::ostream& operator<<(std::ostream &out, Vector3 &vect);
};

