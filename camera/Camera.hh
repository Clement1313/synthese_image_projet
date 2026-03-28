#pragma once 

#include "../vector3/Vector3.hh"

class Camera
{
public:
    Vector3 C;
    Vector3 P;
    Vector3 up;
    float fovX;
    float fovY;
    float zMin;


    Camera(Vector3 C, Vector3 P, Vector3 up, float fovX, float fovY, float zMin)
        : C(C), P(P), up(up), fovX(fovX), fovY(fovY), zMin(zMin) {}


    Vector3 pixelCenter(int i, int j, int width, int height);
};