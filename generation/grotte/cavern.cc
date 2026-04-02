#include "cavern.hh"

#include "../perlin.hh"

Cavern::Cavern(float seuil, float nombre_octaves, float frequence_multiplieur) :
seuil_(seuil), nombre_octaves_(nombre_octaves), frequence_multiplicateur_(frequence_multiplieur){
}



// application des octaves
float Cavern::octave_cavern(const Vector3& point) const {
  float resultat = 0;
  float frequence = 1;
  float amplitude = 1 ;
  for (int i = 0; i < nombre_octaves_; i++) {
    resultat += frequence * perlin_noise::perlin_noise(point * amplitude);
    frequence *= frequence_multiplicateur_;
    amplitude *= (1 / frequence_multiplicateur_);
  }
  return  resultat;
}


float Cavern::sdf_cavern(const Vector3 &point)  const{
  return (seuil_ - octave_cavern(point)) /4;
}

float masquage_camera(const Vector3& point) { // sdf d'une sphère pour le masquage
  Vector3 camera_position = Vector3(0,0,-4) ;
  return ((point - camera_position).norm() - 0.5) / 4;
}
float Cavern::distance(const Vector3 &point) const {
  return std::max(sdf_cavern(point), -masquage_camera(point));
}
