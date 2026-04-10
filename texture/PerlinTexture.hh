#pragma once

#include <algorithm>
#include <cmath>

#include "../generation/perlin.hh"
#include "TextureMaterial.hh"

class PerlinTexture : public TextureMaterial
{
public:
    MaterialInfo low;
    MaterialInfo high;
    float scale;
    int octaves;
    float lacunarity;
    float gain;
    float yMin;
    float yMax;
    float topTextureScaleFactor;
    float topBrightness;

    PerlinTexture(const MaterialInfo& low, const MaterialInfo& high,
                  float scale = 0.35f, int octaves = 4,
                  float lacunarity = 2.0f, float gain = 0.5f,
                  float yMin = -14.0f, float yMax = 14.0f,
                  float topTextureScaleFactor = 2.0f,
                  float topBrightness = 0.30f)
        : low(low),
          high(high),
          scale(scale),
          octaves(std::max(1, octaves)),
          lacunarity(lacunarity),
          gain(gain),
          yMin(yMin),
          yMax(yMax),
          topTextureScaleFactor(topTextureScaleFactor),
          topBrightness(std::max(0.0f, topBrightness))
    {
    }

    MaterialInfo getMaterial(const Vector3& p) const override
    {
        const float heightDenom = std::max(0.001f, yMax - yMin);
        const float heightT = std::clamp((p.y - yMin) / heightDenom, 0.0f, 1.0f);

        const float bottomNoiseT = sampleFBM(p, scale);
        const float topNoiseT = sampleFBM(p, scale * topTextureScaleFactor);
        const float topMapped = std::clamp(0.2f + 0.8f * topNoiseT, 0.0f, 1.0f);
        const float t = (1.0f - heightT) * bottomNoiseT + heightT * topMapped;

        const auto lerp = [t](float a, float b) { return a + (b - a) * t; };
        const auto lerpColor = [t](int a, int b) {
            return static_cast<int>(a + static_cast<float>(b - a) * t);
        };

        const float brightness = 1.0f + topBrightness * heightT;

        const int r = std::clamp(
            static_cast<int>(lerpColor(low.color.r, high.color.r) * brightness),
            0, 255);
        const int g = std::clamp(
            static_cast<int>(lerpColor(low.color.g, high.color.g) * brightness),
            0, 255);
        const int b = std::clamp(
            static_cast<int>(lerpColor(low.color.b, high.color.b) * brightness),
            0, 255);

        return MaterialInfo(
            lerp(low.kd, high.kd),
            lerp(low.ks, high.ks) * (1.0f - 0.20f * heightT),
            lerp(low.ns, high.ns),
            Colors(r, g, b));
    }

private:
    float sampleFBM(const Vector3& p, float baseScale) const
    {
        float amplitude = 1.0f;
        float frequency = baseScale;
        float sum = 0.0f;
        float norm = 0.0f;

        for (int i = 0; i < octaves; i++)
        {
            const Vector3 q = p * frequency;
            sum += amplitude * perlin_noise::perlin_noise(q);
            norm += amplitude;
            frequency *= lacunarity;
            amplitude *= gain;
        }

        if (norm <= 0.0f)
        {
            return 0.5f;
        }

        const float n = sum / norm;
        const float t = std::clamp(0.5f * (n + 1.0f), 0.0f, 1.0f);
        return std::clamp(0.5f + (t - 0.5f) * 0.75f, 0.0f, 1.0f);
    }
};
