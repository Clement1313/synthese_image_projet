
#ifndef STALACTITES_HH
#define STALACTITES_HH
#include "../../vector3/Vector3.hh"

#include <optional>
#include <queue>
#include <random>
#include <set>
#include <vector>

// réalisation de l'algorithme de Poison Disk
// Sampling:http://devmag.org.za/2009/05/03/poisson-disk-sampling/
namespace  stalactites {

  struct Stalactite {
    Vector3 coord;
    float height;
    float radius;
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

  // Signature: generateStalactites: (float, float, float) -> void
  // Description: génère les stalactites et renvoie un vector 2D (z = 0)
  std::vector<Vector3> generateStalactites(float min_dist = 0.3, float heigh = 1,
                                           float width = 1,
                                           size_t new_points_count = 33);

  float sdf_stalactite(Vector3& p, float height, float radius);

};



#endif //STALACTITES_HH
