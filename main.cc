#include <iostream>
#include <memory>

#include "generation/grotte/cavern.hh"
#include "generation/perlin.hh"
#include "ray_marching/SDFPrimitives.hh"
#include "ray_marching/rayMarching.hh"
#include "scene/scene.hh"
#include "texture/PerlinTexture.hh"

using namespace std;

int main() {
    scene world;
    // world.addObject(std::make_shared<SphereSDF>(Vector3(0.0f, 0.0f, 0.0f), 1.0f));
    auto cave = std::make_shared<Cavern>(0.1,3,0.12);
    cave->setTexture(std::make_shared<PerlinTexture>(
        MaterialInfo(0.80f, 0.03f, 6.0f, Colors(92, 76, 63)),
        MaterialInfo(0.72f, 0.07f, 10.0f, Colors(130, 108, 90)),
        0.28f,
        5,
        2.1f,
        0.52f));
    world.addObject(cave);

    ray_marching::render(world, "cavern.ppm");
    return 0;
}
