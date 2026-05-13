
#ifndef STALACTITES_HH
#define STALACTITES_HH
#include "../../ray_marching/SDF.hh"
#include "../../vector3/Vector3.hh"

#include <functional>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <vector>

class StalactiteSDF;
// réalisation de l'algorithme de Poison Disk
// Sampling:http://devmag.org.za/2009/05/03/poisson-disk-sampling/
namespace  objects {

  struct Particle {
    Vector3 position;
    float sigma; // rayon de la répulsion
    Vector3 vitesse; // dérivée de p du pas précédent
  };
  class RandomQueu {
  private:
    std::mt19937 g_;
  public:
    std::vector<Vector3> element_;

    std::optional<Vector3> remove_element();
    RandomQueu(): g_(std::random_device{}()) {
    }
  };

  // Signature: generateObjects : (float, float, float) -> void
  // Description: génère les points pour les objets à pauser sur la cavern
  std::vector<Vector3> generateCoordObject( const
    SDF& sdf,size_t  = 33,float = 1.5f,bool = false
    );

  float sdf_stalactite(Vector3& p, float height, float radius);
std::vector<StalactiteSDF> generateCluster (const Vector3& center, std::mt19937& rng);
};



#endif //STALACTITES_HH
