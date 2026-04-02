#include "perlin.hh"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

std::vector<int> hash(256,-1);
std::vector<Vector3> gradient_list =
  {Vector3(1,1,0),Vector3(-1,1,0),Vector3(1,-1,0),Vector3(-1,-1,0),
  Vector3(1,0,1),Vector3(-1,0,1),Vector3(1,0,-1),Vector3(-1,0,-1),
  Vector3(0,1,1),Vector3(0,-1,1),Vector3(0,1,-1),Vector3(0,-1,-1)};


// méthode pour lisser le décimal
float lissage_decimal(const float value) {
    return static_cast<float>(pow(value,5) * 6 - pow(value,4) * 15 + pow(value,3)  * 10);
}

// fait la dérivée directionnel entre le gradient et le vecteur directionnels
float derivee_directionnel(const Vector3& gradient, const Vector3& vecteur_directionnel) {
  return gradient.scal(vecteur_directionnel) ;
}



//choix de la valeur hash
int hash_value(const int x ,const int y , const int z) {
  return hash[(x + hash[(y + hash[z & 255]) & 255 ]) & 255] & 15;
}

// choix du gradient
float gradient_application(const int x , const int y , const int z) {
     return derivee_directionnel(gradient_list[hash_value(x,y,z)],Vector3((float)x,(float)y,(float)z));
}

void generate_hash() {
  for (int i = 0; i < 256;i++) {
    hash[i] = i;
  }

  // randomization de la valeur pour le hash
  std::random_device rd;
  std::mt19937 g(rd());

  // mélange du hash
  std::shuffle(hash.begin(),hash.end(),g);
}

// étape de l'application du gradient sur chacun des paramètre
float gradient_effect(float ix, float iy, float iz, int x, int y, int z,
  float x_decimal,float y_decimal, float z_decimal) {
  Vector3& gradient = gradient_list[hash_value(x,y,z)];
  Vector3 vecteur_distance = Vector3((x_decimal - ix),(y_decimal - iy),(z_decimal - iz));
  return derivee_directionnel(gradient,vecteur_distance);

}

float lerp(float a, float b, float t) {
  return a + t * (b - a);
}

float perlin_noise::perlin_noise(const Vector3 &point) {


  // check if hash n'est pas bien mis
  // si vraie remplie aléatoirement
  if (hash.at(0) < 0) {
    generate_hash();
  }

  // partie entière des valeurs et grille de la cellule ou se trouve notre point
  // min de la cellule
  int x0  = static_cast<int>(floor(static_cast<double>(point.x)));
  int y0 = static_cast<int>(floor(static_cast<double>(point.y)));
  int z0 = static_cast<int>(floor(static_cast<double>(point.z)));

  // max de la cellule
  int x1 = x0 + 1;
  int y1 = y0 + 1;
  int z1 = z0 + 1;

  // partie décimal des valeurs
  float x_decimal =  point.x - static_cast<float>(x0);
  float y_decimal = point.y - static_cast<float>(y0);
  float z_decimal = point.z - static_cast<float>(z0);

  // lissage des valeurs décimal
  float x_lissage = lissage_decimal(x_decimal);
  float y_lissage = lissage_decimal(y_decimal);
  float z_lissage = lissage_decimal(z_decimal);

  // réalisation des opérations sur chaque paramètre
  float g000 = gradient_effect(0,0,0,x0,y0,z0,x_decimal,y_decimal,z_decimal);
  float g100 = gradient_effect(1,0,0,x1,y0,z0,x_decimal,y_decimal,z_decimal);
  float g010 = gradient_effect(0,1,0,x0,y1,z0,x_decimal,y_decimal,z_decimal);
  float g110 = gradient_effect(1,1,0,x1,y1,z0,x_decimal,y_decimal,z_decimal);
  float g001 = gradient_effect(0,0,1,x0,y0,z1,x_decimal,y_decimal,z_decimal);
  float g101 = gradient_effect(1,0,1,x1,y0,z1,x_decimal,y_decimal,z_decimal);
  float g011 = gradient_effect(0,1,1,x0,y1,z1,x_decimal,y_decimal,z_decimal);
  float g111 = gradient_effect(1,1,1,x1,y1,z1,x_decimal,y_decimal,z_decimal);


  // interpolation entre les valeurs
  float x0z0 = lerp(g000,g100,x_lissage);
  float x1z0 = lerp(g010,g110,x_lissage);
  float x0z1 = lerp(g001,g101,x_lissage);
  float x1z1 = lerp(g011,g111,x_lissage);

  float y0_ = lerp(x0z0,x1z0,y_lissage);
  float y1_ = lerp(x0z1,x1z1,y_lissage);


  return lerp(y0_,y1_,z_lissage);
}
