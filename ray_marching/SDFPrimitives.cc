#include "SDFPrimitives.hh"

float SphereSDF::distance(const Vector3& p) const {
	return (p - center).norm() - radius;
}
