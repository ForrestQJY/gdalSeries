#ifndef GB_HPP
#define GB_HPP

/*******************************************************************************
 * Copyright 2014 GeoData <geodata@soton.ac.uk>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *******************************************************************************/

/**
 * @file gb.hpp
 * @brief All required definitions for working with `libctb`
 *
 * @mainpage Cesium Terrain Builder Library (libctb)
 *
 * `libctb` is a C++ library used to create terrain tiles for use in the
 * [Cesium JavaScript library](http://cesiumjs.org).  Terrain tiles are created
 * according to the [heightmap-1.0 terrain
 * format](http://cesiumjs.org/data-and-assets/terrain/formats/heightmap-1.0.html).
 * The library does not provide a way of serving up or storing the resulting
 * tiles: this is application specific.  Its aim is simply to take a
 * [GDAL](http://www.gdal.org) compatible raster representing a Digital Terrain
 * Model (DTM) and convert this to terrain tiles.  See the tools provided with
 * the library (e.g. `gb-tile`) for an example on how the the library is used
 * to achieve this.
 *
 * To use the library include `gb.hpp` e.g.
 *
 * \code
 * // test.cpp
 * #include <iostream>
 * #include "gb.hpp"
 *
 * using namespace std;
 *
 * int main() {
 *
 *   cout << "Using libctb version "
 *        << gb::version.major << "."
 *        << gb::version.minor << "."
 *        << gb::version.patch << endl;
 *
 *   return 0;
 * }
 * \endcode
 *
 * Assuming the library is installed on the system and you are using the `g++`
 * compiler, the above can be compiled using:
 *
 * \code{.sh}
 * g++ -lctb -o test test.cpp
 * \endcode
 *
 * ## Implementation overview
 * 
 * The concept of a grid is implemented in the `gb::Grid` class.  The TMS
 * Global Geodetic and Global Merdcator profiles are specialisations of the grid
 * implemented in the `gb::GlobalGeodetic` and `gb::GlobalMercator` classes.
 * These classes define the tiling scheme which is then used to cut up GDAL
 * rasters into the output tiles.
 * 
 * The `gb::GDALTiler` and `gb::TerrainTiler` classes composes an instance of
 * a grid with a GDAL raster dataset.  They use the dataset to determine the
 * native raster resolution and extent.  Once this is known the appropriate zoom
 * levels and tile coverage can be calculated from the grid.  For each tile an
 * in memory GDAL [Virtual Raster](http://www.gdal.org/gdal_vrttut.html) (VRT)
 * can then be generated.  This is a lightweight representation of the relevant
 * underlying data necessary to create populate the tile.  The VRT can then be
 * used to generate an actual `gb::TerrainTile` instance or raster dataset
 * which can then be stored as required by the application.
 * 
 * There are various iterator classes providing convenient iteration over
 * tilesets created by grids and tilers.  For instance, the
 * `gb::TerrainIterator` class provides a simple interface for iterating over
 * all valid tiles represented by a `gb::TerrainTiler`, and likewise the
 * `gb::RasterIterator` over a `gb::GDALTiler` instance.
 *
 * See the `README.md` file distributed with the source code for further
 * details.
 */

#include "gb/Bounds.hpp"
#include "gb/Coordinate.hpp"
#include "gb/CTBException.hpp"
#include "gb/GDALTile.hpp"
#include "gb/GDALTiler.hpp"
#include "gb/GlobalGeodetic.hpp"
#include "gb/GlobalMercator.hpp"
#include "gb/Grid.hpp"
#include "gb/GridIterator.hpp"
#include "gb/RasterIterator.hpp"
#include "gb/RasterTiler.hpp"
#include "gb/TerrainIterator.hpp"
#include "gb/TerrainTile.hpp"
#include "gb/TerrainTiler.hpp"
#include "gb/TileCoordinate.hpp"
#include "gb/Tile.hpp"
#include "gb/TilerIterator.hpp"
#include "gb/types.hpp"

#endif /* CTB_HPP */
