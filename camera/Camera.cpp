#include "Camera.hh"
#include "../vector3/Vector3.hh"

#include <cmath>

namespace {
Vector3 addVec(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 crossVec(const Vector3& a, const Vector3& b) {
    return Vector3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}
} 

Vector3 Camera::pixelCenter(int i, int j, int width, int height) {
    Vector3 forward = (P - C).normalized();
    Vector3 t = up - forward * up.dot(forward);
    Vector3 trueUp = t.normalized();
    Vector3 right = crossVec(forward, trueUp).normalized();

    const float z = zMin;
    Vector3 imageCenter = addVec(C, forward * z);

    float halfWidth = std::tan(fovX * 0.5f) * z;
    float halfHeight = std::tan(fovY * 0.5f) * z;

    float u = ((static_cast<float>(i) + 0.5f) / static_cast<float>(width) - 0.5f) * 2.0f * halfWidth;
    float v = ((static_cast<float>(j) + 0.5f) / static_cast<float>(height) - 0.5f) * 2.0f * halfHeight;

    return addVec(addVec(imageCenter, right * u), trueUp * v);
}
