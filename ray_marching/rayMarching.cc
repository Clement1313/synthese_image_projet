#include "rayMarching.hh"

#include <algorithm>
#include <array>
#include <cmath>

#include "../camera/Camera.hh"
#include "../image/Image.hh"
#include "SDFPrimitives.hh"

namespace
{
    constexpr int WIDTH = 800;
    constexpr int HEIGHT = 600;
    constexpr int MAX_STEPS = 84;
    constexpr float MAX_DIST = 80.0f;
    constexpr float SURFACE_APPROX = 0.0050f;
    constexpr bool ENABLE_ANTIALIASING = false;
    constexpr int AA_SAMPLES = 4;

    constexpr float SAMPLE_OFFSETS[AA_SAMPLES][2] = { { 0.25f, 0.25f },
                                                      { 0.75f, 0.25f },
                                                      { 0.25f, 0.75f },
                                                      { 0.75f, 0.75f } };

    Vector3 addVec(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    Vector3 mulVec(const Vector3& a, float s)
    {
        return Vector3(a.x * s, a.y * s, a.z * s);
    }

    Colors clampColor(float r, float g, float b)
    {
        return Colors(std::clamp(static_cast<int>(r), 0, 255),
                      std::clamp(static_cast<int>(g), 0, 255),
                      std::clamp(static_cast<int>(b), 0, 255));
    }

    Colors lerpColor(const Colors& a, const Colors& b, float t)
    {
        const float h = std::clamp(t, 0.0f, 1.0f);
        return clampColor(a.r + (b.r - a.r) * h, a.g + (b.g - a.g) * h,
                          a.b + (b.b - a.b) * h);
    }

    Vector3 estimateNormal(const SDF& scene, const Vector3& p)
    {
        const float e = 0.0005f;
        const float dx = scene.distance(addVec(p, Vector3(e, 0.0f, 0.0f)))
            - scene.distance(addVec(p, Vector3(-e, 0.0f, 0.0f)));
        const float dy = scene.distance(addVec(p, Vector3(0.0f, e, 0.0f)))
            - scene.distance(addVec(p, Vector3(0.0f, -e, 0.0f)));
        const float dz = scene.distance(addVec(p, Vector3(0.0f, 0.0f, e)))
            - scene.distance(addVec(p, Vector3(0.0f, 0.0f, -e)));
        return Vector3(dx, dy, dz).normalized();
    }

    float softShadow(const SDF& scene, const Vector3& hitPoint,
                     const Vector3& rayDir, float mint, float maxt, float w)
    {
        float res = 1.0f;
        float t = mint;
        for (int i = 0; i < 52 && t < maxt; i++)
        {
            float h = scene.distance(addVec(hitPoint, rayDir * t));
            res = std::min(res, h / (w * t));
            t += std::clamp(h, 0.008f, 0.60f);
            if (res < -1.0f || t > maxt)
                break;
        }
        res = std::max(res, -1.0f);
        return 0.25f * (1.0f + res) * (1.0f + res) * (2.0f - res);
    }

    float ambientOcclusion(const SDF& scene, const Vector3& hitPoint,
                           const Vector3& normal, float stepDist,
                           float stepNumber)
    {
        float occlusion = 1.f;
        while (stepNumber > 0.0f)
        {
            const Vector3 samplePoint =
                addVec(hitPoint, normal * (stepNumber * stepDist));
            const float expectedDist = stepNumber * stepDist;
            const float actualDist = scene.distance(samplePoint);
            occlusion -= std::pow(expectedDist - actualDist, 2.0f) / stepNumber;
            stepNumber--;
        }
        return std::max(0.0f, occlusion);
    }

    Colors shadePixel(const SDF& scene, const Vector3& hitPoint,
                      const Vector3& rayDir, const MaterialInfo& material)
    {
        const Vector3 normal = estimateNormal(scene, hitPoint);
        const Vector3 keyLightPos(0.3f, -1.4f, 8.0f);
        const Vector3 fillLightPos(-4.0f, 1.0f, 24.0f);
        const Vector3 keyDir = (keyLightPos - hitPoint).normalized();
        const Vector3 fillDir = (fillLightPos - hitPoint).normalized();
        const Vector3 viewDir = (rayDir * -1.0f).normalized();

        const float diffuseKey = std::max(0.0f, normal.dot(keyDir));
        const float diffuseFill = std::max(0.0f, normal.dot(fillDir));

        const float ambient = 0.24f;
        const float minLight = 0.12f;

        const Vector3 halfVector = addVec(keyDir, viewDir).normalized();
        const float specular =
            std::pow(std::max(0.0f, normal.dot(halfVector)), material.ns);

        const float ao = std::clamp(
            std::pow(ambientOcclusion(scene, hitPoint, normal, 0.028f, 7.f),
                     0.5f),
            0.45f, 1.0f);

        float keyShadow = 1.0f;
        if (diffuseKey > 0.02f)
        {
            keyShadow = std::clamp(
                softShadow(scene, hitPoint, keyDir, 0.03f, 10.0f, 0.55f), 0.55f,
                1.0f);
        }

        float fillShadow = 1.0f;
        if (diffuseFill > 0.02f)
        {
            fillShadow = std::clamp(
                softShadow(scene, hitPoint, fillDir, 0.03f, 8.0f, 0.7f), 0.70f,
                1.0f);
        }

        const float keyIntensity = material.kd * diffuseKey * ao * keyShadow;
        const float fillIntensity =
            0.45f * material.kd * diffuseFill * ao * fillShadow;

        const float warmR =
            minLight + ambient * ao + keyIntensity + 0.18f * fillIntensity;
        const float warmG = minLight + ambient * ao + 0.95f * keyIntensity
            + 0.30f * fillIntensity;
        const float warmB = minLight + ambient * ao + 0.80f * keyIntensity
            + 0.60f * fillIntensity;

        const float specularIntensity = 0.30f * material.ks * specular;
        const float r = material.color.r * warmR + 195.0f * specularIntensity;
        const float g = material.color.g * warmG + 180.0f * specularIntensity;
        const float b = material.color.b * warmB + 165.0f * specularIntensity;

        const float gamma = 0.92f;
        const float rg =
            255.0f * std::pow(std::clamp(r / 255.0f, 0.0f, 1.0f), gamma);
        const float gg =
            255.0f * std::pow(std::clamp(g / 255.0f, 0.0f, 1.0f), gamma);
        const float bg =
            255.0f * std::pow(std::clamp(b / 255.0f, 0.0f, 1.0f), gamma);

        return clampColor(rg, gg, bg);
    }

    Colors tracePixel(const SDF& scene, const Camera& camera, float pixelX,
                      float pixelY)
    {
        const Vector3 pixelCenter =
            camera.pixelCenter(pixelX, pixelY, WIDTH, HEIGHT);
        const Vector3 rayOrigin = camera.C;
        const Vector3 rayDir = (pixelCenter - rayOrigin).normalized();

        float t = 0.0f;
        bool hit = false;
        Vector3 hitPoint;

        for (int step = 0; step < MAX_STEPS && t < MAX_DIST; step++)
        {
            const Vector3 p = addVec(rayOrigin, rayDir * t);
            const float dist = scene.distance(p);

            if (dist < SURFACE_APPROX)
            {
                hit = true;
                hitPoint = addVec(p, mulVec(rayDir, SURFACE_APPROX * 3.0f));
                break;
            }
            t += std::max(dist, 0.001f);
        }

        if (hit)
        {
            const SDF* hitObject = scene.closestObject(hitPoint);
            const MaterialInfo material = hitObject
                ? hitObject->getMaterial(hitPoint)
                : MaterialInfo(0.85f, 0.25f, 20.0f, Colors(235, 110, 85));
            const Colors lit = shadePixel(scene, hitPoint, rayDir, material);

            // Slight atmospheric fog to increase depth readability in the cave.
            const float fog = std::min(0.35f, 1.0f - std::exp(-0.012f * t));
            const Colors fogColor(38, 52, 66);
            return lerpColor(lit, fogColor, fog);
        }

        const float gradient = pixelY / static_cast<float>(HEIGHT - 1);
        return Colors(static_cast<int>(12 + gradient * 10),
                      static_cast<int>(16 + gradient * 12),
                      static_cast<int>(20 + gradient * 16));
    }
} // namespace

namespace ray_marching
{
    void render(const SDF& scene, const std::string& outputPath)
    {
        Image image(WIDTH, HEIGHT);

        struct CameraPreset
        {
            Vector3 c;
            Vector3 p;
            float fovX;
            float fovY;
        };

        const std::array<CameraPreset, 6> presets = {
            CameraPreset{ Vector3(0.1f, -2.0f, 11.0f),
                          Vector3(2.1f, -1.9f, 35.0f), 0.95f, 0.64f },
            CameraPreset{ Vector3(-0.3f, -2.0f, 9.0f),
                          Vector3(1.8f, -1.9f, 31.0f), 1.00f, 0.68f },
            CameraPreset{ Vector3(0.6f, -2.0f, 14.0f),
                          Vector3(-1.0f, -1.9f, 34.0f), 0.98f, 0.66f },
            CameraPreset{ Vector3(-1.2f, -2.1f, 8.0f),
                          Vector3(2.0f, -2.0f, 28.0f), 1.05f, 0.72f },
            CameraPreset{ Vector3(0.2f, -2.1f, 6.0f),
                          Vector3(-1.8f, -2.0f, 24.0f), 1.08f, 0.74f },
            CameraPreset{ Vector3(0.0f, -2.0f, 4.0f),
                          Vector3(1.2f, -2.0f, 20.0f), 1.10f, 0.76f }
        };

        CameraPreset chosen = presets.front();
        for (const auto& preset : presets)
        {
            if (scene.distance(preset.c) > 0.25f)
            {
                chosen = preset;
                break;
            }
        }

        Camera camera(chosen.c, chosen.p, Vector3(0.0f, 1.0f, 0.0f),
                      chosen.fovX, chosen.fovY, 1.0f);

        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                float rSum = 0.0f;
                float gSum = 0.0f;
                float bSum = 0.0f;
                const int sampleCount = ENABLE_ANTIALIASING ? AA_SAMPLES : 1;

                for (int sample = 0; sample < sampleCount; sample++)
                {
                    const float offsetX =
                        ENABLE_ANTIALIASING ? SAMPLE_OFFSETS[sample][0] : 0.5f;
                    const float offsetY =
                        ENABLE_ANTIALIASING ? SAMPLE_OFFSETS[sample][1] : 0.5f;
                    const float sampleX = static_cast<float>(x) + offsetX;
                    const float sampleY =
                        static_cast<float>(HEIGHT - 1 - y) + offsetY;
                    const Colors sampleColor =
                        tracePixel(scene, camera, sampleX, sampleY);
                    rSum += static_cast<float>(sampleColor.r);
                    gSum += static_cast<float>(sampleColor.g);
                    bSum += static_cast<float>(sampleColor.b);
                }

                image.setPixel(Colors(static_cast<int>(rSum / sampleCount),
                                      static_cast<int>(gSum / sampleCount),
                                      static_cast<int>(bSum / sampleCount)),
                               x, y);
            }
        }

        image.savePPM(outputPath);
    }
} // namespace ray_marching
