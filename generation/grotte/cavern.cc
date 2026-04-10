#include "cavern.hh"

#include "../perlin.hh"
#include "../../texture/UniformTexture.hh"

#include <complex>
#include <iostream>
#include <vector>
#include "../detail/stalactites.hh"
Vector3 camera_position = Vector3(0,0,-4) ;
//std::vector<Vector3> list_stalactites = stalactites::generateStalactites(3,600,800);

Cavern::Cavern(float seuil, float nombre_octaves, float frequence_multiplieur) :
seuil_(seuil), nombre_octaves_(nombre_octaves), frequence_multiplicateur_(frequence_multiplieur),
texture_(std::make_shared<UniformTexture>(MaterialInfo(0.88f, 0.18f, 14.0f, Colors(155, 92, 60)))){
}

void Cavern::setTexture(const std::shared_ptr<TextureMaterial>& texture) {
  texture_ = texture;
}



// application des octaves
float Cavern::octave_cavern(const Vector3& point) const {
  float resultat = 0;
  float frequence = 1;
  float amplitude = 1 ;
  for (int i = 0; i < nombre_octaves_; i++) {
    resultat += amplitude * (1.0f - std::fabs(perlin_noise::perlin_noise(point * frequence)));
    frequence *= frequence_multiplicateur_;
    amplitude *=  0.5;
  }
  return  resultat;
}


float Cavern::sdf_cavern(const Vector3 &point)  const{
  return seuil_ - octave_cavern(point);
}

float masquage_camera(const Vector3& point) { // sdf d'une sphère pour le masquage
  Vector3 camera_position = Vector3(0,0,-4) ;
  return ((point - camera_position).norm() - 0.5) / 4;
}

float get_abs(float p ) {
  return p * (p > 0)  - p * (p < 0);
}

void do_max(Vector3& vector3, float value) {
  vector3.x = std::max(vector3.x,value);
  vector3.y = std::max(vector3.y,value);
  vector3.z = std::max(vector3.z, value);
}

void do_min(Vector3& vector3, float value) {
  vector3.x = std::min(vector3.x,value);
  vector3.y = std::min(vector3.y,value);
  vector3.z = std::min(vector3.z, value);
}

float sdfRoundBox(Vector3 p, Vector3 b, float r) {
  Vector3 q =  Vector3(get_abs(p.x),get_abs(p.y),get_abs(p.z)) - b;
  q = Vector3(q.x - r, q.y - r, q.z - r);
  float x = q.x;
  float y = q.y;
  float z = q.z;
  do_max(q,0.0f);
  float value = std::min(std::max(x,std::max(y,z)),0.0f);
  return q.norm() + value;
}
Vector3 vector = Vector3(26,14,26);

float sdfEllipsoid(Vector3 p, Vector3 r) {
  float k0 = (p/r).norm();
  float k1 = (p/ (r * r )).norm();
  return k0 * (k0 - 1.0) / k1;
}


Vector3 point_perlin_noise(const Vector3& point) {
  return
  point -
      Vector3(-perlin_noise::perlin_noise(point) * 0.75f
      ,-perlin_noise::perlin_noise(point  - (Vector3(-1.7,-9.2,-3.4)) ) * 0.25f,
      -perlin_noise::perlin_noise((point) - Vector3(-8.3,-2.8,-5.1)) * 0.25f);
}

float sdf_caverne_salle(const Vector3& point) {
  return -(sdfEllipsoid(point_perlin_noise(point),
    vector))   ;

}

float Cavern::distance(const Vector3 &point) const {
  return sdf_caverne_salle(point);
}


Colors Cavern::getColor() const {
  return Colors(255,0,0);
}

MaterialInfo Cavern::getMaterial(const Vector3& point) const {
  if (texture_) {
    return texture_->getMaterial(point);
  }
  return MaterialInfo(0.88f, 0.18f, 14.0f, Colors(155, 92, 60));
}
