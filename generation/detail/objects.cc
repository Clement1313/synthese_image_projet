#define _USE_MATH_DEFINES
#include "objects.hh"
#include <cmath>

#include "StalactiteSDF.hh"

#include <algorithm>
#include <iostream>
#include <random>

namespace  objects {
// Paramètre
constexpr float SURFACE_APPROX = 0.1f;

std::optional<Vector3> RandomQueu::remove_element() {
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
  std::uniform_real_distribution<float> angle (0,2 * M_PI);
  std::uniform_real_distribution<float> rayonDistance(min_dist,2* min_dist);
  std::uniform_real_distribution<float> cosTheta(-1,1);
  float rayon = rayonDistance(g);
  float theta = angle(g);
  float phi = std::acos(cosTheta(g));
  return point +  Vector3(
    rayon * std::sin(phi ) * std::cos(theta),
    rayon * std::cos(phi),
    rayon * std::sin(phi) * std::sin(theta)
    );
}

bool isInCoordonate(float elt, float max,float min) {
  return elt >= min && elt < max;
}

bool inRectangle(const Vector3& point, Vector3& min_coord, Vector3& max_coord)  {
  return isInCoordonate(point.x,max_coord.x,min_coord.x)
  && isInCoordonate(point.y,max_coord.y,min_coord.y)
  && isInCoordonate(point.z, max_coord.z,min_coord.z);
}

bool inNeighbourhood(std::vector<int> &grid, Vector3 &point, float min_dist,
                     float cellsize,int with, int heigh,int depth,std::vector<Vector3>& samplePoint) {
  // coordonnée dans la grille
  int grid_x  = floor(point.x / cellsize);
  int grid_y = floor(point.y / cellsize);
  int grid_z = floor(point.z / cellsize);

  // vérification au voisinage de point (5x5)
  int x0 = std::max(0,grid_x - 2);
  int x1 = std::min(grid_x + 2, with - 1 );

  int y0 = std::max(0,grid_y - 2);
  int y1 = std::min(grid_y + 2, heigh -1);

  int z0 = std::max(0, grid_z -2);
  int z1 = std::min(grid_z + 2, depth -1);


  for (int x = x0; x <= x1;x++) {
    for (int z = z0; z <= z1; z++) {
      for (int y = y0; y <= y1;y++) {
        int pointIndex = grid[x + y * with + z * with * heigh];
        if (grid[x + y *with + z * with * heigh] != -1) {
          Vector3 difference = point - samplePoint[pointIndex];
          if (difference.norm() < min_dist)
          {
            return true;
          }
        }

      }
    }

  }
  return false;
}


// objectif de la méthode retourner une projection du point sur la parois
// descente de gradient

Vector3 projectOnSurface(Vector3& point,const std::function<float(Vector3)>& sdf,Vector3& min_coord, Vector3& max_coord) {
  for (int i = 0; i < 50; i++) { // Max des iteration = 50
    float d = sdf(point);
    if (std::abs(d)  < 0.01f) { // condition d'arret
      break;
    }
    float espacement = 0.001f;
    Vector3 gradien = Vector3(
      sdf(point + Vector3(espacement,0,0)) - sdf(point - Vector3(espacement,0,0)),
      sdf(point + Vector3(0,espacement,0)) - sdf(point - Vector3(0,espacement,0)),
      sdf(point + Vector3(0,0,espacement)) - sdf(point - Vector3(0,0,espacement))
      ) / (2*espacement);
    if (gradien.norm() < 1e-6f) { // gradient à nul
      break;
    }
    point = point - gradien.normalized() * d * 0.5f;
  }

  return {std::clamp(point.x, min_coord.x, max_coord.x), std::clamp(point.y,min_coord.y,max_coord.y),
  std::clamp(point.z,min_coord.z,max_coord.z)} ;
}

/* Ancienne méthod edu generateCoordObject ne fonctionnant pas
 *
std::vector<Vector3>
generateCoordObject(const std::function<float(Vector3)>& sdf,Vector3 min_coord, Vector3 max_coord,
float min_dist, size_t new_points_count) {
  float cellsize = min_dist / std::sqrt(3.0f);
  int grid_with = ceil((max_coord.x - min_coord.x)/cellsize);
  int grid_heigh = ceil((max_coord.y - min_coord.y)/cellsize);
  int grid_depth = ceil((max_coord.z - min_coord.z)/cellsize);


  std::vector<int> grid(grid_with *grid_heigh * grid_depth,-1);

  RandomQueu processList;

  std::vector<Vector3> samplePoints;
  // randomization des valeurs
  std::random_device rd;
  std::mt19937 g(rd());
  std::uniform_real_distribution<float> width_distribution(min_coord.x,max_coord.x);
  std::uniform_real_distribution<float> heigh_distribution(min_coord.y,max_coord.y);
  std::uniform_real_distribution<float> depth_distribution(min_coord.z,max_coord.z);

  Vector3 firstpoint = Vector3(width_distribution(g),heigh_distribution(g),depth_distribution(g));
  firstpoint = projectOnSurface(firstpoint,sdf,min_coord,max_coord);
  processList.element_.push_back(firstpoint);
  samplePoints.push_back(firstpoint);

  int index_x = floor((firstpoint.x - min_coord.x) / cellsize);
  int index_y = floor((firstpoint.y - min_coord.y)/ cellsize);
  int index_z = floor((firstpoint.z - min_coord.z)/ cellsize);

  grid[index_x + index_y * grid_with + index_z * grid_with * grid_heigh] = 0; // formule de parcours d'un tableau 1D a 2 dimensions

  while (!processList.element_.empty() && samplePoints.size() < new_points_count) {
    std::cout << "processList size: " << processList.element_.size() << "\n";
    Vector3 point = processList.remove_element().value();
    for (size_t i = 0; i < new_points_count; i++) {
      Vector3 newPoint = generateRandPointAround(point,min_dist,g) ;
      newPoint = projectOnSurface(newPoint,sdf,min_coord,max_coord);
      if (inRectangle(newPoint,min_coord,max_coord) &&
        !inNeighbourhood(grid,newPoint,min_dist,cellsize,grid_with,grid_heigh,grid_depth,samplePoints)) {
        samplePoints.push_back(newPoint);
        processList.element_.push_back(newPoint);
        int x = floor((newPoint.x - min_coord.x) / cellsize);
        int y = floor((newPoint.y  - min_coord.y) / cellsize);
        int z = floor((newPoint.z - min_coord.z) / cellsize);
        grid[x + y * grid_with +z * grid_with * grid_heigh] = samplePoints.size() - 1;
      }
    }
  }
  return samplePoints;
}
*/

bool inNeightbourhood(const std::vector<Vector3>& points, const Vector3& point_actuel,const float min_dist) {
  for (const auto& point: points) {
    if ( (point_actuel - point).norm() < min_dist) {
      return true;
    }
  }
  return false;
}


std::vector<Vector3> generateCoordObject( const SDF& sdf,size_t new_points_count,float min_dist,
  bool search_ceilling) {
  std::vector<Vector3> samplePoints;
  std::random_device rd;
  std::mt19937 g(rd());
  std::uniform_real_distribution<float> angle_dist(0.0f,2.0f * M_PI);
  std::uniform_real_distribution<float> y_dist(0.2f,1.0f);

  std::uniform_real_distribution<float> z_dist(5.0f,30.0f);
  std::uniform_real_distribution<float> x_dist(-2.0f,2.0f);



  float d = 0.001f;
  Vector3 v1 {d,0,0};
  Vector3 v2 {0,d,0};
  Vector3 v3 {0,0,d};
  std::uniform_real_distribution<float> distance(-1.0f,1.0f);


  for (size_t i = 0; samplePoints.size() < new_points_count && i < new_points_count * 50; i++) {
    float theta = angle_dist(g);
    float y_dir = y_dist(g);

    // si on cherche le sol on inverse la direcition
    if (!search_ceilling) {
      y_dir *= -1;
    }
    float r = std::sqrt(1.0f - y_dir * y_dir);
    Vector3 dir(r * std::cos(theta), y_dir,r* std::sin(theta));
    dir = dir.normalized();
    float t = 0.0f;
    bool hit = false;


    Vector3 origin {x_dist(g),-2.0f,z_dist(g)};
    Vector3 p = origin;
    for (int step = 0; step < 100; step++) {
      float d = sdf.distance(p);
      if (d < 0.05f) {
        hit = true;
        break;
      }
      t += std::max(d,0.01f);
      p = origin + dir * t;
      if (t > 50.0f) break;
    }
    if (hit) {
      bool good_surface = search_ceilling ? (p.y > 1.5f) : (p.y < -3.0f);
      if (good_surface && !inNeightbourhood(samplePoints,p,min_dist)
        ) {
        samplePoints.push_back(p);
        }
    }
    }

  return samplePoints;
}

float sdf_stalactite(Vector3 &p, float height, float radius) {
  float factor = std::clamp(-p.y / height,0.0f,1.0f);
  float t ;

  if (factor < 0.25f) {
    t = 1.0f - std::pow(factor / 0.25f,0.45f) * 0.55f;
  }
  else {
    float s = (factor - 0.25f) / 0.75f;
    t = (1.0f - 0.55f) * std::pow(1.0f - s,1.4f);
  }

  float current_radius= radius * t;

  auto body = static_cast<float>(std::sin(factor * M_PI));

  current_radius += 0.008f * radius * body * std::sin(p.y * 22.0f);


  float distance_horizontal = std::sqrt(p.x * p.x + p.z * p.z);
  float distance_cone = distance_horizontal - current_radius;
  distance_cone = std::max(distance_cone,p.y);
  return std::max(distance_cone, -p.y -height);
}


}