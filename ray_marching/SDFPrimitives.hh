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

class PlaneSDF: public SDF
{
public:
	Vector3 point;
	Vector3 normal;

	PlaneSDF(const Vector3& point, const Vector3& normal): point(point), normal(normal.normalized()) {}
	
	float distance(const Vector3& p) const override;

};
