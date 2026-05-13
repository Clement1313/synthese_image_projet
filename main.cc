#include <iostream>
#include <memory>

#include "generation/detail/StalactiteSDF.hh"
#include "generation/detail/objects.hh"
#include "generation/grotte/cavern.hh"
#include "generation/perlin.hh"
#include "ray_marching/SDFPrimitives.hh"
#include "ray_marching/rayMarching.hh"
#include "scene/scene.hh"
#include "texture/PerlinTexture.hh"

using namespace std;

int main()
{
    Vector3 cam{0.0f,-2.0f,-3.0f};
    scene world;
    // world.addObject(std::make_shared<SphereSDF>(Vector3(0.0f, 0.0f,

    // 0.0f), 1.0f));
    auto cave = std::make_shared<Cavern>(0.1, 3, 0.12);
    cave->setTexture(std::make_shared<PerlinTexture>(
        MaterialInfo(0.82f, 0.04f, 8.0f, Colors(86, 78, 66)),
        MaterialInfo(0.68f, 0.14f, 22.0f, Colors(164, 154, 138)), 0.16f, 5,
        2.15f, 0.54f, -8.0f, 8.0f, 2.2f, 0.28f));
    world.addObject(cave);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> h_distance(1.2f,2.8f);
    std::uniform_real_distribution<float> r_distance(0.10f,0.30f);
    auto points = objects::generateCoordObject(*cave,30,2.0f,true);
    std::cout<< "points:" << points.size() << "\n";
    for (const auto& p: points ) {
        if ((p - cam).norm() > 3.0f) {
            cave->addStalactite(std::make_shared<StalactiteSDF>(p,h_distance(rng),r_distance(rng)));
        }
    }


    /*
    points = objects::generateCoordObject(*cave,60,2.0f,false);
    for (const auto& p: points ) {
        if ((p - cam).norm() > 3.0f) {
            cave->addStalactite(std::make_shared<StalactiteSDF>(p,h_distance(rng),r_distance(rng),true));
        }
    }
    */


    ray_marching::render(world, "cavern.ppm");
    return 0;
}
