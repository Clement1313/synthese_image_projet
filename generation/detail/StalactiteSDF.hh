//
// Created by marce on 07/05/2026.
//

#ifndef STALACTITESDF_HH
#define STALACTITESDF_HH

#include "../../ray_marching/SDF.hh"
#include "../../texture/PerlinTexture.hh"
#include "../../texture/UniformTexture.hh"
#include "objects.hh"
#include <memory>

class Vector3;
class StalactiteSDF: public SDF{
  Vector3 position_;
  float height_, radius_;
  bool inverse_;
  std::shared_ptr<TextureMaterial> texture_;
public:
  [[nodiscard]] float get_height() const {
    return height_;
  }
  [[nodiscard]] float get_radius() const  {
    return radius_;
  }
  [[nodiscard]] const Vector3& getPosition() const {
    return position_;
  }
  void setPosition(const Vector3& position) {
    position_.x = position.x;
    position_.y = position.y;
    position_.z = position.z;
  }
  StalactiteSDF(Vector3 p , float height,float radius, bool inverse = false):
  position_(p),height_(height), radius_(radius), inverse_(inverse),texture_(std::make_shared<PerlinTexture>(
        MaterialInfo(0.82f, 0.04f, 8.0f, Colors(86, 78, 66)),
        MaterialInfo(0.68f, 0.14f, 22.0f, Colors(164, 154, 138)), 0.16f, 5,
        2.15f, 0.54f, -8.0f, 8.0f, 2.2f, 0.28f)) {
  }
  float distance(const Vector3 &p) const override {
    Vector3 local = p - position_;
    if (inverse_) {
      local = Vector3(local.x,-local.y,local.z);
    }
    return objects::sdf_stalactite(local,height_,radius_);
  }
  Colors getColor() const override {
    return {255,0,0};
  }

  MaterialInfo getMaterial(const Vector3 & point) const override {
    if (texture_)
    {
      return texture_->getMaterial(point);
    }
    return {0.88f, 0.18f, 14.0f, Colors(155, 92, 60)};
  }


};



#endif //STALACTITESDF_HH
