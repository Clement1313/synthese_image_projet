#pragma once

#include "TextureMaterial.hh"

class UniformTexture: public TextureMaterial
{
public:
    MaterialInfo texture;

    UniformTexture(MaterialInfo& texture): texture(texture) {}

    MaterialInfo getMaterial(const Vector3&) const override {
        return texture;
    }
};
    