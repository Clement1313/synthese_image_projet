#include "cavern.hh"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../detail/objects.hh"
#include "../../texture/UniformTexture.hh"
#include "../perlin.hh"

#include <iostream>

namespace
{

    float noiseAt(const Vector3& p)
    {
        return perlin_noise::perlin_noise(p);
    }

    float clamp01(float x)
    {
        return std::clamp(x, 0.0f, 1.0f);
    }

    float smoothMin(float a, float b, float k)
    {
        const float h = clamp01(0.5f + 0.5f * (b - a) / k);
        return (1.0f - h) * b + h * a - k * h * (1.0f - h);
    }

    float remap(float x, float inMin, float inMax, float outMin, float outMax)
    {
        const float denom = inMax - inMin;
        if (std::abs(denom) < 1e-6f)
        {
            return outMin;
        }
        const float t = (x - inMin) / denom;
        return outMin + (outMax - outMin) * t;
    }

    float fbm(const Vector3& p, int octaves)
    {
        float sum = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float norm = 0.0f;

        for (int i = 0; i < octaves; i++)
        {
            sum += amplitude * noiseAt(p * frequency);
            norm += amplitude;
            frequency *= 2.0f;
            amplitude *= 0.5f;
        }

        return (norm > 0.0f) ? (sum / norm) : 0.0f;
    }

    float ellipsoidSdf(const Vector3& p, const Vector3& center,
                       const Vector3& radii)
    {
        const Vector3 q = (p - center) / radii;
        const float minRadius = std::min(radii.x, std::min(radii.y, radii.z));
        return (q.norm() - 1.0f) * minRadius;
    }

    float capsuleSdf(const Vector3& p, const Vector3& a, const Vector3& b,
                     float radius)
    {
        const Vector3 pa = p - a;
        const Vector3 ba = b - a;
        const float denom = std::max(1e-6f, ba.dot(ba));
        const float h = std::clamp(pa.dot(ba) / denom, 0.0f, 1.0f);
        return (pa - ba * h).norm() - radius;
    }

    Vector3 warpPoint(const Vector3& p)
    {
        const float wx = 1.45f
            * noiseAt(Vector3(p.x * 0.085f + 7.1f, p.y * 0.085f, p.z * 0.085f));
        const float wy = 0.95f
            * noiseAt(Vector3(p.x * 0.10f, p.y * 0.10f + 3.3f, p.z * 0.10f));
        const float wz = 1.45f
            * noiseAt(Vector3(p.x * 0.095f, p.y * 0.095f, p.z * 0.095f + 5.9f));
        return Vector3(p.x + wx, p.y + wy, p.z + wz);
    }

    float chamberNetworkSdf(const Vector3& p)
    {
        const Vector3 q = warpPoint(p);
        float d = std::numeric_limits<float>::infinity();

        d = std::min(d,
                     ellipsoidSdf(q, Vector3(0.0f, -2.2f, 3.8f),
                                  Vector3(5.0f, 2.8f, 4.6f)));
        d = std::min(d,
                     ellipsoidSdf(q, Vector3(-3.8f, -2.1f, 10.0f),
                                  Vector3(8.4f, 3.8f, 6.2f)));
        d = std::min(d,
                     ellipsoidSdf(q, Vector3(0.8f, -2.0f, 15.8f),
                                  Vector3(8.2f, 3.8f, 6.8f)));
        d = std::min(d,
                     ellipsoidSdf(q, Vector3(3.6f, -2.0f, 21.0f),
                                  Vector3(9.2f, 4.2f, 7.4f)));
        d = std::min(d,
                     ellipsoidSdf(q, Vector3(-2.0f, -1.9f, 32.5f),
                                  Vector3(8.2f, 3.9f, 6.7f)));

        d = smoothMin(d,
                      ellipsoidSdf(q, Vector3(-9.0f, -2.2f, 18.0f),
                                   Vector3(4.8f, 2.9f, 5.0f)),
                      1.1f);
        d = smoothMin(d,
                      ellipsoidSdf(q, Vector3(8.7f, -2.0f, 27.5f),
                                   Vector3(4.4f, 2.7f, 4.8f)),
                      1.1f);

        d = smoothMin(d,
                      capsuleSdf(q, Vector3(0.1f, -2.2f, 2.4f),
                                 Vector3(-1.4f, -2.1f, 8.0f), 1.9f),
                      0.9f);
        d = smoothMin(d,
                      capsuleSdf(q, Vector3(-1.4f, -2.1f, 8.0f),
                                 Vector3(1.9f, -2.0f, 18.0f), 1.8f),
                      0.9f);
        d = smoothMin(d,
                      capsuleSdf(q, Vector3(1.9f, -2.0f, 18.0f),
                                 Vector3(-0.7f, -1.9f, 29.0f), 1.7f),
                      0.9f);
        d = smoothMin(d,
                      capsuleSdf(q, Vector3(-3.0f, -2.1f, 14.0f),
                                 Vector3(-7.6f, -2.1f, 18.0f), 1.3f),
                      0.8f);
        d = smoothMin(d,
                      capsuleSdf(q, Vector3(2.6f, -2.0f, 23.0f),
                                 Vector3(7.2f, -2.0f, 27.0f), 1.25f),
                      0.8f);

        return d;
    }

    float floorRoofConstraintSdf(const Vector3& p)
    {
        const float floorHeight =
            -5.5f + 0.80f * fbm(Vector3(p.x * 0.075f, p.z * 0.075f, 11.0f), 2);
        const float roofHeight =
            3.0f + 0.72f * fbm(Vector3(p.x * 0.082f, p.z * 0.082f, 4.0f), 2);

        const float belowRoof = p.y - roofHeight;
        const float aboveFloor = floorHeight - p.y;
        return std::max(aboveFloor, belowRoof);
    }

    float rockEnvelopeSdf(const Vector3& p)
    {
        float d = ellipsoidSdf(p, Vector3(0.0f, -1.7f, 20.0f),
                               Vector3(21.0f, 11.5f, 39.0f));
        d = std::min(d,
                     ellipsoidSdf(p, Vector3(-3.4f, -1.9f, 20.0f),
                                  Vector3(15.0f, 9.5f, 32.0f)));
        d = std::min(d,
                     ellipsoidSdf(p, Vector3(3.5f, -1.8f, 22.0f),
                                  Vector3(15.4f, 9.8f, 32.5f)));
        return d;
    }

    float cavitySdf(const Vector3& p)
    {
        float d = chamberNetworkSdf(p);
        d = std::max(d, floorRoofConstraintSdf(p));

        const float rough = 0.34f * fbm(p * 0.18f, 3)
            + 0.20f * noiseAt(p * 0.62f) + 0.08f * noiseAt(p * 1.35f);
        d += rough;
        return d;
    }
} // namespace

Cavern::Cavern(float seuil, float nombre_octaves, float frequence_multiplieur)
    : seuil_(seuil)
    , nombre_octaves_(nombre_octaves)
    , frequence_multiplicateur_(frequence_multiplieur)
    , texture_(std::make_shared<UniformTexture>(
          MaterialInfo(0.88f, 0.18f, 14.0f, Colors(155, 92, 60)))) {
}

void Cavern::setTexture(const std::shared_ptr<TextureMaterial>& texture)
{
    texture_ = texture;
}

float Cavern::octave_cavern(const Vector3& point) const
{
    const int octaves =
        std::max(1, static_cast<int>(std::round(nombre_octaves_)));
    return fbm(point * std::max(0.05f, frequence_multiplicateur_), octaves);
}

float Cavern::sdf_cavern(const Vector3& point) const
{
    const float envelope = rockEnvelopeSdf(point);

    const float shellConstraint = envelope + 2.20f;
    float cave = cavitySdf(point);
    cave = std::max(cave, shellConstraint);

    float rock = std::max(envelope, -cave);

    const float tightness = remap(seuil_, -0.5f, 0.8f, -0.20f, 0.20f);
    rock -= tightness;
    return rock;
}

float Cavern::distance(const Vector3& point) const
{
    float rock = sdf_cavern(point);

    for (const auto& stalactite: stalactites_) {
        float distance = stalactite->distance(point);
        rock = smoothMin(rock,distance,0.25f);
    }
    return rock;
}

Colors Cavern::getColor() const
{
    return Colors(255, 0, 0);
}

MaterialInfo Cavern::getMaterial(const Vector3& point) const
{
    if (texture_)
    {
        return texture_->getMaterial(point);
    }
    return MaterialInfo(0.88f, 0.18f, 14.0f, Colors(155, 92, 60));
}
