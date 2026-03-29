#pragma once

#include <string>

#include "SDF.hh"

namespace ray_marching
{
    void render(const SDF& scene, const std::string& outputPath);
}
