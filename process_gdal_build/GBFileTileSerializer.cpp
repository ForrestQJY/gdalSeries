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
  * @file CTBFileTileSerializer.cpp
  * @brief This defines the `CTBFileTileSerializer` class
  */

#include <stdio.h>
#include <string.h>
#include <mutex>

#include "concat.hpp"
#include "cpl_vsi.h"
#include "GBException.hpp"
#include "GBFileTileSerializer.hpp"

#include "GBFileOutputStream.hpp"
#include "GBZOutputStream.hpp"
#include <io_file.h>
#include <io_utily.h>

using namespace std;
using namespace gb;

#ifdef _WIN32
static const char* osDirSep = "\\";
#else
static const char* osDirSep = "/";
#endif


/// Create a fileName for a tile coordinate
std::string
gb::GBFileTileSerializer::getTileFilename(const TileCoordinate* coord, const string folderName, const char* extension) {
	static mutex mutex;
	string fileName = concat(folderName, coord->zoom, osDirSep, coord->x);

	lock_guard<std::mutex> lock(mutex);

	// Check whether the `{zoom}/{x}` directory exists or not	
	if (!io_file::exists(fileName)) {
		fileName = concat(folderName, coord->zoom);
		if (!io_file::exists(fileName)) {
			io_file::mkdirs(fileName);
		}
		fileName += concat(osDirSep, coord->x);
		io_file::mkdirs(fileName);
	}

	fileName += concat(osDirSep, coord->y);
	if (extension != NULL) {
		fileName += ".";
		fileName += extension;
	}

	return fileName;
}

/// Check if file exists
static bool
fileExists(const std::string& fileName) {
	VSIStatBufL statbuf;
	return VSIStatExL(fileName.c_str(), &statbuf, VSI_STAT_EXISTS_FLAG) == 0;
}


/**
 * @details
 * Returns if the specified Tile Coordinate should be serialized
 */
bool gb::GBFileTileSerializer::mustSerializeCoordinate(const gb::TileCoordinate* coordinate) {
	if (!overlayFile)
		return true;

	const string fileName = getTileFilename(coordinate, path_folder, "terrain");
	return !fileExists(fileName);
}

/**
 * @details
 * Serialize a GDALTile to the Directory store
 */
bool
gb::GBFileTileSerializer::serializeTile(const gb::GDALTile* tile, GDALDriver* driver, const char* extension, CPLStringList& creationOptions) {
	const TileCoordinate* coordinate = tile;
	const string fileName = getTileFilename(coordinate, path_folder, extension);
	if (io_file::exists(fileName)) {
		if (!overlayFile)
		{
			return true;
		}
		else {
			io_file::deleteFile(fileName);
		}
	}
	GDALDataset* poDstDS;
	std::string fileName_UTF8 = io_file::stringToUTF8(fileName);
	poDstDS = driver->CreateCopy(fileName_UTF8.c_str(), tile->dataset, FALSE, creationOptions, NULL, NULL);

	// Close the datasets, flushing data to destination
	if (poDstDS == NULL) {
		throw GBException("Could not create GDAL tile");
	}
	GDALClose(poDstDS);
	return true;
}

/**
 * @details
 * Serialize a TerrainTile to the Directory store
 */
bool
gb::GBFileTileSerializer::serializeTile(const gb::TerrainTile* tile, bool isGzip) {
	const TileCoordinate* coordinate = tile;
	const string fileName = getTileFilename(coordinate, path_folder, "terrain");
	if (io_file::exists(fileName)) {
		if (!overlayFile)
		{
			return true;
		}
		else {
			io_file::deleteFile(fileName);
		}
	}
	if (isGzip) {
		GBZFileOutputStream ostream(fileName.c_str());
		tile->writeFile(ostream);
		ostream.close();
	}
	else {
		FILE* f;
		fopen_s(&f, fileName.c_str(), "wb");
		if (!f) return false;
		GBFileOutputStream ostream(f);
		tile->writeFile(ostream);
		fclose(f);
	}
	return true;
}

/**
 * @details
 * Serialize a MeshTile to the Directory store
 */
bool
gb::GBFileTileSerializer::serializeTile(const gb::MeshTile* tile, bool isGzip, bool writeVertexNormals) {
	const TileCoordinate* coordinate = tile;
	const string fileName = getTileFilename(coordinate, path_folder, "terrain");
	if (io_file::exists(fileName)) {
		if (!overlayFile)
		{
			return true;
		}
		else {
			io_file::deleteFile(fileName);
		}
	}
	if (isGzip) {
		GBZFileOutputStream ostream(fileName.c_str());
		tile->writeFile(ostream, writeVertexNormals);
		ostream.close();
	}
	else {
		FILE* f;
		fopen_s(&f, fileName.c_str(), "wb");
		if (!f) return false;
		GBFileOutputStream ostream(f);
		tile->writeFile(ostream, writeVertexNormals);
		fclose(f);
	}
	return true;
}
