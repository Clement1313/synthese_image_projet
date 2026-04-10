#include "rayMarching.hh"

#include <algorithm>
#include <cmath>

#include "../camera/Camera.hh"
#include "../image/Image.hh"
#include "SDFPrimitives.hh"

namespace
{
    constexpr int WIDTH = 800;
    constexpr int HEIGHT = 600;
    constexpr int MAX_STEPS = 128;
    constexpr float MAX_DIST = 100.0f;
    constexpr float SURFACE_APPROX = 0.002f;
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
        for (float i = 0; i < 256 && t < maxt; i++)
        {
            float h = scene.distance(addVec(hitPoint, rayDir * t));
            res = std::min(res, h / (w * t));
            t += std::clamp(h, 0.005f, 0.50f);
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
        const Vector3 lightPos(3.0f, 2.2f, -1.0f);
        const Vector3 lightDir =
            (addVec(lightPos - hitPoint, Vector3(0.0f, 0.7f, 0.0f)).normalized());
        const Vector3 viewDir = (rayDir * -1.0f).normalized();

        const float diffuse = std::max(0.0f, normal.dot(lightDir));
        const float ambient = 0.30f;
        const float minLight = 0.22f;
        const Vector3 halfVector = addVec(lightDir, viewDir).normalized();
        const float specular =
            std::pow(std::max(0.0f, normal.dot(halfVector)), material.ns);

        const float ao = std::clamp(
            std::pow(ambientOcclusion(scene, hitPoint, normal, 0.015f, 20.f),
                     0.5f),
            0.35f, 1.0f);

        const float sh = std::clamp(
            softShadow(scene, hitPoint, lightDir, 0.02f, 20.5f, 0.5f), 0.45f,
            1.f);

        const float diffuseIntensity =
            std::min(1.0f,
                     minLight + (ambient * ao)
                         + (material.kd * diffuse * ao * sh));
        const float specularIntensity = 0.25f * material.ks * specular;
        const int r = std::min(255,
                               static_cast<int>(material.color.r * diffuseIntensity
                                                + 180.0f * specularIntensity));
        const int g = std::min(255,
                               static_cast<int>(material.color.g * diffuseIntensity
                                                + 180.0f * specularIntensity));
        const int b = std::min(255,
                               static_cast<int>(material.color.b * diffuseIntensity
                                                + 180.0f * specularIntensity));

        return Colors(r, g, b);
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
                hitPoint = addVec(p, rayDir * SURFACE_APPROX);
                break;
            }
            t += std::max(dist, 0.001f);
        }

        if (hit)
        {
            const SDF* hitObject = scene.closestObject(hitPoint);
            const MaterialInfo material =
                hitObject ? hitObject->getMaterial(hitPoint)
                          : MaterialInfo(0.85f, 0.25f, 20.0f,
                                         Colors(235, 110, 85));
            return shadePixel(scene, hitPoint, rayDir, material);
        }

        const float gradient = pixelY / static_cast<float>(HEIGHT - 1);
        return Colors(static_cast<int>(45 + gradient * 45),
                      static_cast<int>(90 + gradient * 70),
                      static_cast<int>(155 + gradient * 70));
    }
} // namespace

namespace ray_marching
{
    void render(const SDF& scene, const std::string& outputPath)
    {
        Image image(WIDTH, HEIGHT);

        Camera camera(Vector3(0.0f, -2.0f, -3.0f), Vector3(0.0f, -2.0f, 0.0f),
                      Vector3(0.0f, 1.0f, 0.0f), 2.4f, 1.8f, 1.0f);

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
