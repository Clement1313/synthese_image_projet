#pragma once

#include "SDF.hh"

class SphereSDF : public SDF
{
public:
	Vector3 center;
	float radius;

	SphereSDF(const Vector3& center, float radius): center(center), radius(radius) {}

	float distance(const Vector3& p) const override;
};
