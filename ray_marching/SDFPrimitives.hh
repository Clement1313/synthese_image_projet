#pragma once

#include <memory>

#include "../texture/UniformTexture.hh"
#include "SDF.hh"

class SphereSDF : public SDF
{
public:
	Vector3 center;
	float radius;
	Colors color;
	std::shared_ptr<TextureMaterial> texture;

	SphereSDF(const Vector3& center, float radius,
	          const Colors& color = Colors(235, 110, 85),
	          const std::shared_ptr<TextureMaterial>& texture = nullptr)
	    : center(center),
	      radius(radius),
	      color(color),
	      texture(texture
	                  ? texture
	                  : std::make_shared<UniformTexture>(
	                        MaterialInfo(0.85f, 0.25f, 20.0f, color))) {}

	float distance(const Vector3& p) const override;
	Colors getColor() const override { return color; }
	MaterialInfo getMaterial(const Vector3& p) const override
	{
		return texture ? texture->getMaterial(p)
		               : MaterialInfo(0.85f, 0.25f, 20.0f, color);
	}
};

class PlaneSDF: public SDF
{
public:
	Vector3 point;
	Vector3 normal;
	Colors color;
	std::shared_ptr<TextureMaterial> texture;

	PlaneSDF(const Vector3& point, const Vector3& normal,
	         const Colors& color = Colors(140, 140, 145),
	         const std::shared_ptr<TextureMaterial>& texture = nullptr)
	    : point(point),
	      normal(normal.normalized()),
	      color(color),
	      texture(texture
	                  ? texture
	                  : std::make_shared<UniformTexture>(
	                        MaterialInfo(0.85f, 0.20f, 12.0f, color))) {}
	
	float distance(const Vector3& p) const override;
	Colors getColor() const override { return color; }
	MaterialInfo getMaterial(const Vector3& p) const override
	{
		return texture ? texture->getMaterial(p)
		               : MaterialInfo(0.85f, 0.20f, 12.0f, color);
	}

};
