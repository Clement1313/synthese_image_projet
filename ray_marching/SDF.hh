#pragma once

#include "../Colors.hh"
#include "../vector3/Vector3.hh"

class SDF
{
public:
    virtual ~SDF() = default;
    virtual float distance(const Vector3& p) const = 0;
    virtual const SDF* closestObject(const Vector3&) const { return this; }
    virtual Colors getColor() const { return Colors(); }
};