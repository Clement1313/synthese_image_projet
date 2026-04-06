#include <iostream>
#include <memory>

#include "generation/perlin.hh"
#include "ray_marching/SDFPrimitives.hh"
#include "ray_marching/rayMarching.hh"
#include "scene/scene.hh"

using namespace std;

int main() {
    scene world;
    world.addObject(std::make_shared<SphereSDF>(
        Vector3(0.0f, 0.0f, 0.0f), 1.0f, Colors(235, 110, 85)));
    world.addObject(std::make_shared<PlaneSDF>(
        Vector3(0.0f, -1.f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Colors(150, 150, 155)));

    ray_marching::render(world, "sphere.ppm");
    return 0;
}
