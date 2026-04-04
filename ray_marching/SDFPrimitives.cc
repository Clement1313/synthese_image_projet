#include "SDFPrimitives.hh"

float SphereSDF::distance(const Vector3& p) const {
	return (p - center).norm() - radius;
}

float PlaneSDF::distance(const Vector3& p) const {
	return (p - point).dot(normal);
}

