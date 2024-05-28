#ifndef GBMESH_HPP
#define GBMESH_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include "types.hpp"
#include "GBException.hpp"

namespace gb {
  class Mesh;
}

/**
 * @brief An abstract base class for a mesh of triangles
 */
class gb::Mesh
{
public:

  /// Create an empty mesh
  Mesh()
  {}

public:

  /// The array of shared vertices of a mesh
  std::vector<CRSVertex> vertices;

  /// The index collection for each triangle in the mesh (3 for each triangle)
  std::vector<uint32_t> indices;

  /// Write mesh data to a WKT file
  void writeWktFile(const char *fileName) const {
    FILE *fp;
    fopen_s(&fp, fileName, "W");
    if (fp == NULL) {
      throw GBException("Failed to open file");
    }

    char wktText[512];
    memset(wktText, 0, sizeof(wktText));

    for (int i = 0, icount = indices.size(); i < icount; i += 3) {
      CRSVertex v0 = vertices[indices[i]];
      CRSVertex v1 = vertices[indices[i+1]];
      CRSVertex v2 = vertices[indices[i+2]];
      sprintf_s(wktText, "(%.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f)",
          v0.x, v0.y, v0.z,
          v1.x, v1.y, v1.z,
          v2.x, v2.y, v2.z,
          v0.x, v0.y, v0.z);
      //sprintf(wktText, "(%.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f, %.8f %.8f %f)",
      //  v0.x, v0.y, v0.z,
      //  v1.x, v1.y, v1.z,
      //  v2.x, v2.y, v2.z,
      //  v0.x, v0.y, v0.z);
      fprintf(fp, "POLYGON Z(%s)\n", wktText);
    }
    fclose(fp);
  };
};

#endif
