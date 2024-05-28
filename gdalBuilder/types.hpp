#ifndef GBTYPES_HPP
#define GBTYPES_HPP


#include <cstdint>              // uint16_t

#include "Bounds.hpp"
#include "Coordinate3D.hpp"

namespace gb {

  // Simple types
  typedef unsigned int i_pixel;       ///< A pixel value
  typedef unsigned int i_tile;        ///< A tile coordinate
  typedef unsigned short int i_zoom;  ///< A zoom level
  typedef uint16_t i_terrain_height;  ///< A terrain tile height

  // Complex types
  typedef Bounds<i_tile> TileBounds;      ///< Tile extents in tile coordinates
  typedef Coordinate<i_pixel> PixelPoint; ///< The location of a pixel
  typedef Coordinate<double> CRSPoint;    ///< A Coordinate Reference System coordinate
  typedef Coordinate3D<double> CRSVertex; ///< A 3D-Vertex of a mesh or tile in CRS coordinates
  typedef Bounds<double> CRSBounds;       ///< Extents in CRS coordinates
  typedef Coordinate<i_tile> TilePoint;   ///< The location of a tile

}

#endif
