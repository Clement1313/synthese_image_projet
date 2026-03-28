#include "Vector3.hh"

#include <cmath>

Vector3 Vector3::operator*(const float &l) const {
    return Vector3(x*l, y*l, z*l);
}

Vector3 Vector3::operator-(const Vector3 &v ) const {
    return Vector3(x-v.x, y-v.y, z-v.z);
}

std::ostream& operator<<(std::ostream &out, Vector3 &vect) {
    return out << "(" << vect.x << ", " << vect.y << ", " << vect.z << ")";
}

Vector3 Vector3::operator/(const float &l ) const {
    return Vector3(x/l, y/l, z/l);
}

Vector3 Vector3::normalized() const {
    return *this / std::sqrt(dot(*this));
}


float Vector3::scal(const Vector3& v) const {
    return x*v.x + y*v.y + z*v.z;
}

float Vector3::norm() const {
    return std::sqrt(this->scal(*this));
}


float Vector3::dot(const Vector3 &v) const {
    return this->scal(v);
}


