#include <iostream>
#include <memory>

#include "generation/grotte/cavern.hh"
#include "generation/perlin.hh"
#include "ray_marching/SDFPrimitives.hh"
#include "ray_marching/rayMarching.hh"
#include "scene/scene.hh"
#include "texture/PerlinTexture.hh"

using namespace std;

int main()
{
    scene world;
    // world.addObject(std::make_shared<SphereSDF>(Vector3(0.0f, 0.0f,
    // 0.0f), 1.0f));
    auto cave = std::make_shared<Cavern>(0.1, 3, 0.12);
    cave->setTexture(std::make_shared<PerlinTexture>(
        MaterialInfo(0.82f, 0.04f, 8.0f, Colors(86, 78, 66)),
        MaterialInfo(0.68f, 0.14f, 22.0f, Colors(164, 154, 138)), 0.16f, 5,
        2.15f, 0.54f, -8.0f, 8.0f, 2.2f, 0.28f));
    world.addObject(cave);

    ray_marching::render(world, "cavern.ppm");
    return 0;
}
