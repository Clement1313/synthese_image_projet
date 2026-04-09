#include <iostream>
#include <memory>

#include "generation/grotte/cavern.hh"
#include "generation/perlin.hh"
#include "ray_marching/SDFPrimitives.hh"
#include "ray_marching/rayMarching.hh"
#include "scene/scene.hh"

using namespace std;

int main() {
    scene world;
    // world.addObject(std::make_shared<SphereSDF>(Vector3(0.0f, 0.0f, 0.0f), 1.0f));
    world.addObject(std::make_shared<Cavern>(0.1,3,0.12));

    ray_marching::render(world, "cavern.ppm");
    return 0;
}
