#pragma once

#include "SDF.hh"

class SphereSDF : public SDF
{
public:
	Vector3 center;
	float radius;
	Colors color;

	SphereSDF(const Vector3& center, float radius,
	          const Colors& color = Colors(235, 110, 85))
	    : center(center), radius(radius), color(color) {}

	float distance(const Vector3& p) const override;
	Colors getColor() const override { return color; }
};

class PlaneSDF: public SDF
{
public:
	Vector3 point;
	Vector3 normal;
	Colors color;

	PlaneSDF(const Vector3& point, const Vector3& normal,
	         const Colors& color = Colors(140, 140, 145))
	    : point(point), normal(normal.normalized()), color(color) {}
	
	float distance(const Vector3& p) const override;
	Colors getColor() const override { return color; }

};
