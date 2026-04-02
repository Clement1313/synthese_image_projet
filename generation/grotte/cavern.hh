#ifndef CAVERN_HH
#define CAVERN_HH

#include "../../ray_marching/SDF.hh"
#include "../../vector3/Vector3.hh"

class  Cavern : public SDF {
  float seuil_ = 0.5;
  float nombre_octaves_ = 3;
  float frequence_multiplicateur_ = 0.75;


 // réalise les octaves de la carne
  float octave_cavern(const Vector3& point) const;
public:


  /*
   * Signature: distance: (Vector3) -> float
   * Défintion: réalise le rendu de la grotte
   * Décomposition:
   *      - Signature: noise_cavern(Vector3) -> float
   *        Description: retourne le Perlin noise en
   *
   *      - Signature: octave_cavern_forme(Vector3) -> float
   *        Description: retourne le perlin noise pour la forme de la grotte
   *
   *      - Signature: octave_cavern_couleur(Vector3, float) -> Vector
   *        Description: retourne la couleur de la grotte en fonction d'un seuil et de la position
   */
   float distance(const Vector3& point) const override;



  float sdf_cavern(const Vector3& point) const;
  /*
   * Signature: rendu_détaillée (Vector3) -> float
   * Définition: prend le point
   */

  Cavern(float seuil,float nombre_octaves,float frequence_multiplieur);
  Cavern() = default;


};



#endif //CAVERN_HH
