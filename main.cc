#include <iostream>
#include <memory>

#include "generation/perlin.hh"
#include "ray_marching/SDFPrimitives.hh"
#include "ray_marching/rayMarching.hh"
#include "scene/scene.hh"

using namespace std;

int main() {
    scene world;
    world.addObject(std::make_shared<SphereSDF>(Vector3(0.0f, 0.0f, 0.0f), 1.0f));

    ray_marching::render(world, "sphere.ppm");
    return 0;
}
