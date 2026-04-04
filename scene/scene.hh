#pragma once

#include "../ray_marching/SDF.hh"

#include <memory>
#include <vector>

class scene: public SDF
{
public:
	void addObject(const std::shared_ptr<SDF>& object);
	float distance(const Vector3& p) const override;

private:
	std::vector<std::shared_ptr<SDF>> objects;

};