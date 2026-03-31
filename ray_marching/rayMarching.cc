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
    constexpr float SURFACE_APPROX = 0.001f;

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

    Colors shadePixel(const SDF& scene, const Vector3& hitPoint,
                      const Vector3& rayDir)
    {
        const Vector3 normal = estimateNormal(scene, hitPoint);
        const Vector3 lightPos(3.0f, 4.0f, -1.0f);
        const Vector3 lightDir = (lightPos - hitPoint).normalized();
        const Vector3 viewDir = (rayDir * -1.0f).normalized();

        const float diffuse = std::max(0.0f, normal.dot(lightDir));
        const float ambient = 0.15f;
        const Vector3 halfVector = addVec(lightDir, viewDir).normalized();
        const float specular =
            std::pow(std::max(0.0f, normal.dot(halfVector)), 80.0f);
        const float diffuseIntensity =
            std::min(1.0f, ambient + 0.85f * diffuse);
        const float specularIntensity = 0.55f * specular;
        // pow(max(dot(reflect(-lightDirection,normal),normalize(ro-p)),0.), 50.);

        const Colors base(235, 110, 85);
        const int r = std::min(255,
                               static_cast<int>(base.r * diffuseIntensity
                                                + 255.0f * specularIntensity));
        const int g = std::min(255,
                               static_cast<int>(base.g * diffuseIntensity
                                                + 255.0f * specularIntensity));
        const int b = std::min(255,
                               static_cast<int>(base.b * diffuseIntensity
                                                + 255.0f * specularIntensity));

        return Colors(r, g, b);
    }
} // namespace

namespace ray_marching
{
    void render(const SDF& scene, const std::string& outputPath)
    {
        Image image(WIDTH, HEIGHT);

        Camera camera(Vector3(0.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f),
                      Vector3(0.0f, 1.0f, 0.0f), 1.2f, 0.9f, 1.0f);

        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                const Vector3 pixelCenter =
                    camera.pixelCenter(x, HEIGHT - 1 - y, WIDTH, HEIGHT);
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
                        hitPoint = p;
                        break;
                    }
                    t += dist;
                }

                if (hit)
                {
                    image.setPixel(shadePixel(scene, hitPoint, rayDir), x, y);
                }
                else
                {
                    const float gradient =
                        static_cast<float>(y) / static_cast<float>(HEIGHT - 1);
                    image.setPixel(
                        Colors(static_cast<int>(45 + gradient * 45),
                               static_cast<int>(90 + gradient * 70),
                               static_cast<int>(155 + gradient * 70)),
                        x, y);
                }
            }
        }

        image.savePPM(outputPath);
    }
} // namespace ray_marching
