
#ifndef PERLIN_HH
#define PERLIN_HH

#include "../vector3/Vector3.hh"

namespace  perlin_noise {

/*
 * Signature: perlin_noise: (Vector3) -> float
 * Défintion: prend un point et retourne une valeur entre -1 et 1
 */
float  perlin_noise(const Vector3& point);
};



#endif //PERLIN_HH
