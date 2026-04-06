#include "scene.hh"

#include <iostream>
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

    // float minDistance = std::numeric_limits<float>::infinity();
    // for (const auto& object : objects)
    // {
    //     const float d = object->distance(p);
    //     if (d < minDistance)
    //     {
    //         minDistance = d;
    //     }
    // }

    // return minDistance;

    float d = objects[0]->distance(p);

    for (int i = 1; i < objects.size(); i++)
    {
        float d2 = objects[i]->distance(p);
        float k = 0.3f;
        float h = std::max(k - std::abs(d - d2), 0.0f);
        d = std::min(d, d2) - (h * h) / (4.0f * k);
    }
    return d;
}

const SDF* scene::closestObject(const Vector3& p) const
{
    if (objects.empty())
    {
        return this;
    }

    const SDF* closest = objects.front().get();
    float minDistance = closest->distance(p);

    for (const auto& object : objects)
    {
        const float d = object->distance(p);
        if (d < minDistance)
        {
            minDistance = d;
            closest = object.get();
        }
    }

    return closest;
}
