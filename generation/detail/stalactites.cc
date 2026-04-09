#include "stalactites.hh"

#include <random>



std::optional<Vector3> stalactites::RandomQueu::remove_element() {
  if (element_.empty()) {
    return std::nullopt;
  }
  std::uniform_int_distribution<size_t> index_distribution(0,element_.size() -1);
  size_t index = index_distribution(g_);
  Vector3 copy = element_[element_.size() -1];
  element_[element_.size() -1] = element_[index];
  element_[index] = copy;
  Vector3 result = element_[element_.size() - 1];
  element_.pop_back();
  return result;
}


// génération d'un point a une distance minimum de  min_dist
Vector3 generateRandPointAround(Vector3& point, float min_dist,std::mt19937& g) {

  std::uniform_real_distribution<float> variation(0,1);

  float angle = 2.0F * M_PI *  variation(g); // calcul d'un angle pour la distance
  float longeur = min_dist * (variation(g) + 1);
  return {point.x + longeur * std::cos(angle), 0, point.z + longeur * std::sin(angle)};

}

bool inRectangle(const Vector3& point, float heigh, float width)  {
  return point.x >= 0 && point.z >= 0 && point.x < width && point.z < heigh;
}

bool inNeighbourhood(std::vector<int> &grid, Vector3 &point, float min_dist,
                     float cellsize,int with, int heigh,std::vector<Vector3>& samplePoint) {
  // coordonnée dans la grille
  int grid_x  = floor(point.x / cellsize);
  int grid_z = floor(point.z / cellsize);

  // vérification au voisinage de point (5x5)
  int x0 = std::max(0,grid_x - 2);
  int x1 = std::min(grid_x + 2, with - 1 );

  int z0 = std::max(0, grid_z -2);
  int z1 = std::min(grid_z + 2, heigh -1);


  for (int x = x0; x <= x1;x++) {
    for (int z = z0; z <= z1; z++) {
      int pointIndex = grid[x + z * with];
      if (grid[x + z * with] != -1) {
        float variation_x = point.x - samplePoint[pointIndex].x;
        float variation_z = point.z - samplePoint[pointIndex].z;
        if (std::sqrt(variation_x * variation_x + variation_z * variation_z) < min_dist) {
          return false;
        }
      }
    }

  }
  return true;


}

std::vector<Vector3>
stalactites::generateStalactites(float min_dist, float heigh, float width,
                                 size_t new_points_count) {
  float cellsize = min_dist / std::sqrt(2.0f);
  int grid_with = ceil(width/cellsize);
  int grid_heigh = ceil(heigh/cellsize);

  std::vector<int> grid(grid_with *grid_heigh,-1);

  RandomQueu processList;

  std::vector<Vector3> samplePoints;
  // randomization des valeurs
  std::random_device rd;
  std::mt19937 g(rd());
  std::uniform_real_distribution<float> width_distribution(0,width);
  std::uniform_real_distribution<float> heigh_distribution(0,heigh);

  Vector3 firstpoint = Vector3(width_distribution(g),0,heigh_distribution(g)); // vecteur 2d

  processList.element_.push_back(firstpoint);
  samplePoints.push_back(firstpoint);

  int index_x = floor(firstpoint.x / cellsize);
  int index_z = floor(firstpoint.z / cellsize);

  grid[index_x + index_z * grid_with] = 0; // formule de parcours d'un tableau 1D a 2 dimensions

  while (!processList.element_.empty()) {
    Vector3 point = processList.remove_element().value();
    for (size_t i = 0; i < new_points_count; i++) {
      Vector3 newPoint = generateRandPointAround(point,min_dist,g) ;
      if (inRectangle(newPoint,heigh,width) && !inNeighbourhood(grid,newPoint,min_dist,cellsize,grid_with,grid_heigh,samplePoints)) {
        samplePoints.push_back(newPoint);
        processList.element_.push_back(newPoint);
        int x = floor(newPoint.x / cellsize);
        int z = floor(newPoint.z / cellsize);
        grid[x + z * grid_with] = samplePoints.size() - 1;
      }
    }
  }


  return samplePoints;
}


float stalactites::sdf_stalactite(Vector3 &p, float height, float radius) {

  float distance_horizontal = std::sqrt(p.x * p.x + p.z * p.z);
  float current_radius = std::max(0.0f,radius * (1 - (std::abs(p.y) / height)));
  float distance_cone = distance_horizontal - current_radius;

  distance_cone = std::max(distance_cone,p.y);

  return std::max(distance_cone,-p.y - height);
}

