#pragma once

#include "../vector3/Vector3.hh"

class SDF
{
public:
    virtual ~SDF() = default;
    virtual float distance(const Vector3& p) const = 0;
};