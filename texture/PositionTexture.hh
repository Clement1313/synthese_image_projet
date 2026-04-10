#pragma once

#include <algorithm>
#include <cmath>

#include "TextureMaterial.hh"

class PositionTexture : public TextureMaterial
{
public:
    MaterialInfo low;
    MaterialInfo high;
    float scale;

    PositionTexture(const MaterialInfo& low, const MaterialInfo& high,
                    float scale = 0.35f)
        : low(low), high(high), scale(scale)
    {
    }

    MaterialInfo getMaterial(const Vector3& p) const override
    {
        const float s = std::sin(scale * p.x)
            * std::cos(scale * 1.37f * p.y)
            * std::sin(scale * 0.73f * p.z);
        const float t = 0.5f * (s + 1.0f);

        const auto lerp = [t](float a, float b) { return a + (b - a) * t; };
        const auto lerpColor = [t](int a, int b) {
            return static_cast<int>(a + static_cast<float>(b - a) * t);
        };

        return MaterialInfo(
            lerp(low.kd, high.kd),
            lerp(low.ks, high.ks),
            lerp(low.ns, high.ns),
            Colors(
                std::clamp(lerpColor(low.color.r, high.color.r), 0, 255),
                std::clamp(lerpColor(low.color.g, high.color.g), 0, 255),
                std::clamp(lerpColor(low.color.b, high.color.b), 0, 255)));
    }
};
