#pragma once

#include "../Colors.hh"
#include "../vector3/Vector3.hh"


struct MaterialInfo {
    float kd, ks, ns;
    Colors color;

    MaterialInfo(): kd(0.), ks(0.), ns(0.), color(Colors(0, 0, 0)) {};
    MaterialInfo(float kd, float ks, float ns, Colors color): kd(kd), ks(ks), ns(ns), color(color) {};
};

class TextureMaterial {
public:
    virtual MaterialInfo getMaterial(const Vector3& p) const = 0;
};

