/*******************************************************************************
 * Copyright 2018 GeoData <geodata@soton.ac.uk>
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
  * @file MeshTiler.cpp
  * @brief This defines the `MeshTiler` class
  * @author Alvaro Huarte <ahuarte47@yahoo.es>
  */

#include "GBException.hpp"
#include "MeshTiler.hpp"
#include "HeightFieldChunker.hpp"
#include "GDALDatasetReader.hpp"

using namespace gb;

////////////////////////////////////////////////////////////////////////////////

/**
 * Implementation of gb::chunk::mesh for gb::Mesh class.
 */
class WrapperMesh : public gb::chunk::mesh {
private:
	CRSBounds& mBounds;
	Mesh& mMesh;
	double mCellSizeX;
	double mCellSizeY;

	std::map<int, int> mIndicesMap;
	Coordinate<int> mTriangles[3];
	bool mTriOddOrder;
	int mTriIndex;

public:
	WrapperMesh(CRSBounds& bounds, Mesh& mesh, i_tile tileSizeX, i_tile tileSizeY) :
		mMesh(mesh),
		mBounds(bounds),
		mTriOddOrder(false),
		mTriIndex(0) {
		mCellSizeX = (bounds.getMaxX() - bounds.getMinX()) / (double)(tileSizeX - 1);
		mCellSizeY = (bounds.getMaxY() - bounds.getMinY()) / (double)(tileSizeY - 1);
	}


	virtual void clear() {
		mMesh.vertices.clear();
		mMesh.indices.clear();
		mIndicesMap.clear();
		mTriOddOrder = false;
		mTriIndex = 0;
	}
	virtual void emit_vertex(const gb::chunk::heightfield& heightfield, int x, int y) {
		mTriangles[mTriIndex].x = x;
		mTriangles[mTriIndex].y = y;
		mTriIndex++;

		if (mTriIndex == 3) {
			mTriOddOrder = !mTriOddOrder;

			if (mTriOddOrder) {
				appendVertex(heightfield, mTriangles[0].x, mTriangles[0].y);
				appendVertex(heightfield, mTriangles[1].x, mTriangles[1].y);
				appendVertex(heightfield, mTriangles[2].x, mTriangles[2].y);
			}
			else {
				appendVertex(heightfield, mTriangles[1].x, mTriangles[1].y);
				appendVertex(heightfield, mTriangles[0].x, mTriangles[0].y);
				appendVertex(heightfield, mTriangles[2].x, mTriangles[2].y);
			}
			mTriangles[0].x = mTriangles[1].x;
			mTriangles[0].y = mTriangles[1].y;
			mTriangles[1].x = mTriangles[2].x;
			mTriangles[1].y = mTriangles[2].y;
			mTriIndex--;
		}
	}
	void appendVertex(const gb::chunk::heightfield& heightfield, int x, int y) {
		int iv;
		int index = heightfield.indexOfGridCoordinate(x, y);

		std::map<int, int>::iterator it = mIndicesMap.find(index);

		if (it == mIndicesMap.end()) {
			iv = mMesh.vertices.size();

			double xmin = mBounds.getMinX();
			double ymax = mBounds.getMaxY();
			double height = heightfield.height(x, y);

			mMesh.vertices.push_back(CRSVertex(xmin + (x * mCellSizeX), ymax - (y * mCellSizeY), height));
			mIndicesMap.insert(std::make_pair(index, iv));
		}
		else {
			iv = it->second;
		}
		mMesh.indices.push_back(iv);
	}
};

////////////////////////////////////////////////////////////////////////////////

void
gb::MeshTiler::prepareSettingsOfTile(MeshTile* terrainTile, GDALDataset* dataset, const TileCoordinate& coord, float* rasterHeights, gb::i_tile tileSizeX, gb::i_tile tileSizeY) const {
	const gb::i_tile TILE_SIZE = tileSizeX;

	// Number of tiles in the horizontal direction at tile level zero.
	double resolutionAtLevelZero = mGrid.resolution(0);
	int numberOfTilesAtLevelZero = (int)(mGrid.getExtent().getWidth() / (tileSizeX * resolutionAtLevelZero));
	// Default quality of terrain created from heightmaps (TerrainProvider.js).
	double heightmapTerrainQuality = 0.25;
	// Earth semi-major-axis in meters.
	const double semiMajorAxis = 6378137.0;
	// Appropriate geometric error estimate when the geometry comes from a heightmap (TerrainProvider.js).
	double maximumGeometricError = MeshTiler::getEstimatedLevelZeroGeometricErrorForAHeightmap(
		semiMajorAxis,
		heightmapTerrainQuality * mMeshQualityFactor,
		TILE_SIZE,
		numberOfTilesAtLevelZero
	);
	// Geometric error for current Level.
	maximumGeometricError /= (double)(1 << coord.zoom);

	// Convert the raster grid into an irregular mesh applying the Chunked LOD strategy by 'Thatcher Ulrich'.
	// http://tulrich.com/geekstuff/chunklod.html
	//
	gb::chunk::heightfield heightfield(rasterHeights, TILE_SIZE);
	heightfield.applyGeometricError(maximumGeometricError, coord.zoom <= 6);
	//
	// Propagate the geometric error of neighbours to avoid gaps in borders.
	if (coord.zoom > 6) {
		gb::CRSBounds datasetBounds = bounds();

		for (int borderIndex = 0; borderIndex < 4; borderIndex++) {
			bool okNeighborCoord = true;
			gb::TileCoordinate neighborCoord = gb::chunk::heightfield::neighborCoord(mGrid, coord, borderIndex, okNeighborCoord);
			if (!okNeighborCoord)
				continue;

			gb::CRSBounds neighborBounds = mGrid.tileBounds(neighborCoord);

			if (datasetBounds.overlaps(neighborBounds)) {
				float* neighborHeights = gb::GDALDatasetReader::readRasterHeights(*this, dataset, neighborCoord, mGrid.tileSize(), mGrid.tileSize());

				gb::chunk::heightfield neighborHeightfield(neighborHeights, TILE_SIZE);
				neighborHeightfield.applyGeometricError(maximumGeometricError);
				heightfield.applyBorderActivationState(neighborHeightfield, borderIndex);

				CPLFree(neighborHeights);
			}
		}
	}
	gb::CRSBounds mGridBounds = mGrid.tileBounds(coord);
	Mesh& tileMesh = terrainTile->getMesh();
	WrapperMesh mesh(mGridBounds, tileMesh, tileSizeX, tileSizeY);
	heightfield.generateMesh(mesh, 0);
	heightfield.clear();

	// If we are not at the maximum zoom level we need to set child flags on the
	// tile where child tiles overlap the dataset bounds.
	if (coord.zoom != maxZoomLevel()) {
		CRSBounds tileBounds = mGrid.tileBounds(coord);

		if (!(bounds().overlaps(tileBounds))) {
			terrainTile->setAllChildren(false);
		}
		else {
			if (bounds().overlaps(tileBounds.getSW())) {
				terrainTile->setChildSW();
			}
			if (bounds().overlaps(tileBounds.getNW())) {
				terrainTile->setChildNW();
			}
			if (bounds().overlaps(tileBounds.getNE())) {
				terrainTile->setChildNE();
			}
			if (bounds().overlaps(tileBounds.getSE())) {
				terrainTile->setChildSE();
			}
		}
	}
}

MeshTile*
gb::MeshTiler::createMesh(GDALDataset* dataset, const TileCoordinate& coord) const {
	// Copy the raster data into an array
	float* rasterHeights = gb::GDALDatasetReader::readRasterHeights(*this, dataset, coord, mGrid.tileSize(), mGrid.tileSize());

	// Get a mesh tile represented by the tile coordinate
	MeshTile* terrainTile = new MeshTile(coord);
	prepareSettingsOfTile(terrainTile, dataset, coord, rasterHeights, mGrid.tileSize(), mGrid.tileSize());
	CPLFree(rasterHeights);

	return terrainTile;
}

MeshTile*
gb::MeshTiler::createMesh(GDALDataset* dataset, const TileCoordinate& coord, gb::GDALDatasetReader* reader) const {
	// Copy the raster data into an array
	float* rasterHeights = reader->readRasterHeights(dataset, coord, mGrid.tileSize(), mGrid.tileSize());

	// Get a mesh tile represented by the tile coordinate
	MeshTile* terrainTile = new MeshTile(coord);
	prepareSettingsOfTile(terrainTile, dataset, coord, rasterHeights, mGrid.tileSize(), mGrid.tileSize());
	CPLFree(rasterHeights);

	return terrainTile;
}

MeshTiler&
gb::MeshTiler::operator=(const MeshTiler& other) {
	TerrainTiler::operator=(other);

	return *this;
}

double gb::MeshTiler::getEstimatedLevelZeroGeometricErrorForAHeightmap(
	double maximumRadius,
	double heightmapTerrainQuality,
	int tileWidth,
	int numberOfTilesAtLevelZero)
{
	double error = maximumRadius * 2 * M_PI * heightmapTerrainQuality;
	error /= (double)(tileWidth * numberOfTilesAtLevelZero);
	return error;
}
