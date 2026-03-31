#include "scene.hh"

#include <limits>

void scene::addObject(const std::shared_ptr<SDF>& object)
{
    objects.push_back(object);
}

float scene::distance(const Vector3& p) const
{
    if (objects.empty())
    {
        return std::numeric_limits<float>::infinity();
    }

    float minDistance = std::numeric_limits<float>::infinity();
    for (const auto& object : objects)
    {
        const float d = object->distance(p);
        if (d < minDistance)
        {
            minDistance = d;
        }
    }

    return minDistance;
}
