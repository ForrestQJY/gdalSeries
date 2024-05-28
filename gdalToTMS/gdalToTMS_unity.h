#ifndef _GDALTOTMS_UNITY_H_
#define _GDALTOTMS_UNITY_H_


#include <mutex>

#include <json/json.h>

#include <future>

#include <GBFileTileSerializer.hpp>
#include <GDALDatasetReader.hpp>
#include <GDALSerializer.hpp>
#include <GlobalGeodetic.hpp>
#include <GlobalMercator.hpp>
#include <Grid.hpp>
#include <MeshIterator.hpp>
#include <MeshSerializer.hpp>
#include <MeshTiler.hpp>
#include <RasterIterator.hpp>
#include <TerrainTiler.hpp>
#include <TerrainIterator.hpp>
#include <TerrainSerializer.hpp>

#include <io_constant.h>
#include <io_log.h>
#include <io_file.h>
#include <json_helper.h>
#include <util_algorithm.h>

#include "gdalToTMS_static.h"

using namespace gb;


class ServiceMetadata {
public:
	ServiceMetadata() {
	}

	// Defines the valid tile indexes of a level in a Tileset
	struct LevelInfo {
	public:
		LevelInfo() {
			startX = startY = std::numeric_limits<int>::max();
			finalX = finalY = std::numeric_limits<int>::min();
		}
		int startX, startY;
		int finalX, finalY;

		inline void add(const TileCoordinate* coordinate) {
			startX = std::min(startX, (int)coordinate->x);
			startY = std::min(startY, (int)coordinate->y);
			finalX = std::max(finalX, (int)coordinate->x);
			finalY = std::max(finalY, (int)coordinate->y);
		}
		inline void add(const LevelInfo& level) {
			startX = std::min(startX, level.startX);
			startY = std::min(startY, level.startY);
			finalX = std::max(finalX, level.finalX);
			finalY = std::max(finalY, level.finalY);
		}
	};
	std::vector<LevelInfo> levels;

	// Defines the bounding box covered by the Terrain
	CRSBounds bounds;
	CRSBounds validBounds;
	i_zoom startZoom;
	i_zoom endZoom;
	// Add metadata of the specified Coordinate
	void add(const GDALTiler& tiler, const TileCoordinate* coordinate) {

		CRSBounds _tileBounds = tiler.grid().tileBounds(*coordinate);
		CRSBounds _validBounds = tiler.bounds();
		i_zoom zoom = coordinate->zoom;

		startZoom = (pStatic::u_Param.f_TMSConfig.StartZoom < 0) ? tiler.maxZoomLevel() : pStatic::u_Param.f_TMSConfig.StartZoom;
		endZoom = (pStatic::u_Param.f_TMSConfig.EndZoom < 0) ? 0 : pStatic::u_Param.f_TMSConfig.EndZoom;

		if ((1 + zoom) > levels.size()) {
			levels.resize(1 + zoom, LevelInfo());
		}
		LevelInfo& level = levels[zoom];
		level.add(coordinate);

		if (bounds.getMaxX() == bounds.getMinX()) {
			bounds = _tileBounds;
		}
		else {
			bounds.setMinX(std::min(bounds.getMinX(), _tileBounds.getMinX()));
			bounds.setMinY(std::min(bounds.getMinY(), _tileBounds.getMinY()));
			bounds.setMaxX(std::max(bounds.getMaxX(), _tileBounds.getMaxX()));
			bounds.setMaxY(std::max(bounds.getMaxY(), _tileBounds.getMaxY()));
		}
		if (validBounds.getMaxX() == validBounds.getMinX()) {
			validBounds = _validBounds;
		}
		else {
			validBounds.setMinX(std::min(validBounds.getMinX(), _validBounds.getMinX()));
			validBounds.setMinY(std::min(validBounds.getMinY(), _validBounds.getMinY()));
			validBounds.setMaxX(std::max(validBounds.getMaxX(), _validBounds.getMaxX()));
			validBounds.setMaxY(std::max(validBounds.getMaxY(), _validBounds.getMaxY()));
		}
	}

	// Add metadata info
	void add(const ServiceMetadata& otherMetadata) {
		if (otherMetadata.levels.size() > 0) {
			const CRSBounds& _tileBounds = otherMetadata.bounds;
			const CRSBounds& _validBounds = otherMetadata.validBounds;

			if (otherMetadata.levels.size() > levels.size())
			{
				levels.resize(otherMetadata.levels.size(), LevelInfo());
			}
			for (size_t i = 0; i < otherMetadata.levels.size(); i++) {
				levels[i].add(otherMetadata.levels[i]);
			}
			startZoom = (pStatic::u_Param.f_TMSConfig.StartZoom < 0) ? otherMetadata.levels.size() - 1 : pStatic::u_Param.f_TMSConfig.StartZoom;
			endZoom = (pStatic::u_Param.f_TMSConfig.EndZoom < 0) ? 0 : pStatic::u_Param.f_TMSConfig.EndZoom;

			if (bounds.getMaxX() == bounds.getMinX()) {
				bounds = _tileBounds;
			}
			else {
				bounds.setMinX(std::min(bounds.getMinX(), _tileBounds.getMinX()));
				bounds.setMinY(std::min(bounds.getMinY(), _tileBounds.getMinY()));
				bounds.setMaxX(std::max(bounds.getMaxX(), _tileBounds.getMaxX()));
				bounds.setMaxY(std::max(bounds.getMaxY(), _tileBounds.getMaxY()));
			}
			if (validBounds.getMaxX() == validBounds.getMinX()) {
				validBounds = _validBounds;
			}
			else {
				validBounds.setMinX(std::min(validBounds.getMinX(), _validBounds.getMinX()));
				validBounds.setMinY(std::min(validBounds.getMinY(), _validBounds.getMinY()));
				validBounds.setMaxX(std::max(validBounds.getMaxX(), _validBounds.getMaxX()));
				validBounds.setMaxY(std::max(validBounds.getMaxY(), _validBounds.getMaxY()));
			}
		}
	}

	/// Output the layer.json metadata file
	/// http://help.agi.com/TerrainServer/RESTAPIGuide.html
	/// Example:
	/// https://assets.agi.com/stk-terrain/v1/tilesets/world/tiles/layer.json
	void writeJsonFile(tmsInfo ti) const {
		double validMinX = validBounds.getMinX();
		double validMinY = validBounds.getMinY();
		double validMaxX = validBounds.getMaxX();
		double validMaxY = validBounds.getMaxY();

		bool ziped = pStatic::u_Param.f_TMSConfig.Gzip == 1;
		std::string strEpsgCode;
		int iEpsgCode = 0;

		if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
			strEpsgCode = "EPSG:4326";
			iEpsgCode = 4326;
		}
		else if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "mercator") == 0) {
			strEpsgCode = "EPSG:3857";
			iEpsgCode = 3857;
		}
		else if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "custom") == 0) {
			std::string strWKT = pStatic::u_Param.f_TMSConfig.CustomWKT;
			strWKT = io_file::toLower(strWKT);
			if (strWKT.find("epsg") != std::string::npos) {
				strEpsgCode = strWKT;
				iEpsgCode = std::stoi(io_file::replace(strWKT, "epsg:", ""));
			}
			else {
				strEpsgCode = "PROJ";
			}
		}

		Json::Value valueLayer;
		valueLayer["attribution"] = "AgCIM Studio";
		valueLayer["contentType"] = ziped ? "gzip" : "";
		valueLayer["description"] = "AgCIM Studio Terrain Server";
		if (pStatic::u_Param.f_TMSConfig.WriteVertexNormals) {
			valueLayer["extensions"].append("octvertexnormals");
		}
		else {
			valueLayer["extensions"].append(util_algorithm::getJsonEmptyArray());
		}
		valueLayer["extensions"].clear();
		if (strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "terrain") == 0) {
			valueLayer["format"] = "heightmap-1.0";
		}
		else if (strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "mesh") == 0) {
			valueLayer["format"] = "quantized-mesh-1.0";
		}
		else {
			valueLayer["format"] = "GDAL";
		}
		valueLayer["name"] = ti.i.inputFileName_WithoutExtension;
		if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
			valueLayer["projection"] = "EPSG:4326";
		}
		else {
			valueLayer["projection"] = "EPSG:3857";
		}
		valueLayer["scheme"] = "tms";
		valueLayer["tilejson"] = "2.1.0";
		valueLayer["tiles"].append("{z}/{x}/{y}.terrain?v={version}");
		valueLayer["version"] = "1.1.0";
		valueLayer["bounds"].append(validMinX);
		valueLayer["bounds"].append(validMinY);
		valueLayer["bounds"].append(validMaxX);
		valueLayer["bounds"].append(validMaxY);
		for (size_t i = 0, icount = levels.size(); i < icount; i++) {
			const LevelInfo& level = levels[i];
			if (level.finalX >= level.startX) {

				Json::Value valueAvailableTemp;
				Json::Value valueAvailable;
				valueAvailable["endX"] = level.finalX;
				valueAvailable["endY"] = level.finalY;
				valueAvailable["startX"] = level.startX;
				valueAvailable["startY"] = level.startY;
				valueAvailableTemp.append(valueAvailable);
				valueLayer["available"].append(valueAvailableTemp);
			}
		}

		Json::Value valueMeta;
		valueMeta["bounds"]["west"] = validMinX;
		valueMeta["bounds"]["south"] = validMinY;
		valueMeta["bounds"]["east"] = validMaxX;
		valueMeta["bounds"]["north"] = validMaxY;
		valueMeta["latLonBounds"]["west"] = validMinX;
		valueMeta["latLonBounds"]["south"] = validMinY;
		valueMeta["latLonBounds"]["east"] = validMaxX;
		valueMeta["latLonBounds"]["north"] = validMaxY;
		valueMeta["maxzoom"] = startZoom;
		valueMeta["minzoom"] = endZoom;
		if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
			valueMeta["proj"] = 4326;
		}
		else {
			valueMeta["proj"] = 3857;
		}
		valueMeta["tiletrans"] = "tms";
		valueMeta["type"] = pStatic::u_Param.f_TMSConfig.TMSFormat;
		valueMeta["ziped"] = ziped;

		std::string layer_Path = ti.o.outputFolderPath + DirSeparator + "layer.json";
		json_helper::writeJson(layer_Path, valueLayer);

		std::string meta_Path = ti.o.outputFolderPath + DirSeparator + "meta.json";
		json_helper::writeJson(meta_Path, valueMeta);
	}
	void writeXmlFile(tmsInfo ti) const {
		OGRSpatialReference sOGRSpatialReference;
		OGRSpatialReference tOGRSpatialReference;
		sOGRSpatialReference.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		tOGRSpatialReference.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRErr sOGRErr = sOGRSpatialReference.importFromEPSG(3857);
		OGRErr tOGRErr = tOGRSpatialReference.importFromEPSG(4326);
		OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(&sOGRSpatialReference, &tOGRSpatialReference);
		if (!transform)return;



		double validMinX = validBounds.getMinX();
		double validMinY = validBounds.getMinY();
		double validMaxX = validBounds.getMaxX();
		double validMaxY = validBounds.getMaxY();
		double validZ = 0;


		transform->Transform(1, &validMinX, &validMinY, &validZ);
		transform->Transform(1, &validMaxX, &validMaxY, &validZ);

		std::string strMinX = std::to_string(validMinX);
		std::string strMinY = std::to_string(validMinY);
		std::string strMaxX = std::to_string(validMaxX);
		std::string strMaxY = std::to_string(validMaxY);
		std::string strEpsgCode;
		int iEpsgCode = 0;
		if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
			strEpsgCode = "EPSG:4326";
			iEpsgCode = 4326;
		}
		else if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "mercator") == 0) {
			strEpsgCode = "EPSG:3857";
			iEpsgCode = 3857;
		}
		else if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "custom") == 0) {
			std::string strWKT = pStatic::u_Param.f_TMSConfig.CustomWKT;
			strWKT = io_file::toLower(strWKT);
			if (strWKT.find("epsg") != std::string::npos) {
				strEpsgCode = strWKT;
				iEpsgCode = std::stoi(io_file::replace(strWKT, "epsg:", ""));
			}
			else {
				strEpsgCode = "PROJ";
			}
		}
		std::string strCTBFormat = pStatic::u_Param.f_TMSConfig.TMSFormat;
		std::string strTileSize = std::to_string(pStatic::u_Param.f_TMSConfig.TileSize);
		std::string strMimeType = strCTBFormat == "jpg" ? "image/jpeg" : strCTBFormat == "png" ? "image/png" : "image/tiff";
		std::string strProfile = pStatic::u_Param.f_TMSConfig.Profile;


		std::string xml;

		xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		xml.append("\n");
		xml.append("<TileMap version=\"1.0.0\" tilemapservice=\"http://tms.osgeo.org/1.0.0\">");
		xml.append("\n");
		xml.append("<Title>" + ti.i.inputFileName_WithinExtension + "</Title>");
		xml.append("\n");
		xml.append("<Abstract></Abstract>");
		xml.append("\n");
		xml.append("<SRS>" + strEpsgCode + "</SRS>");
		xml.append("\n");
		xml.append("<BoundingBox minx=\"" + strMinX + "\" miny=\"" + strMinY + "\" maxx=\"" + strMaxX + "\" maxy=\"" + strMaxY + "\"/>");
		xml.append("\n");
		xml.append("<Origin x=\"" + strMinX + "\" y=\"" + strMinY + "\"/>");
		xml.append("\n");
		xml.append("<TileFormat width=\"" + strTileSize + "\" height=\"" + strTileSize + "\" mime-type=\"" + strMimeType + "\" extension=\"" + strCTBFormat + "\"/>");
		xml.append("\n");
		xml.append("<TileSets profile=\"" + strProfile + "\">");
		xml.append("\n");

		for (int i = 0; i < levels.size(); i++)
		{
			std::string strIndex = std::to_string(i);
			std::string strPixel = "";
			if (iEpsgCode == 4326) {
				double pixel = 0.703125 / std::pow(2, i);
				strPixel = std::to_string(pixel);
			}
			else if (iEpsgCode == 3857) {
				double pixel = 78271.516 / std::pow(2, i);
				strPixel = std::to_string(pixel);
			}
			xml.append("<TileSet href=\"" + strIndex + "\" units-per-pixel=\"" + strPixel + "\" order=\"" + strIndex + "\"/>");
			xml.append("\n");
		}
		xml.append("</TileSets>");
		xml.append("\n");
		xml.append("</TileMap>");


		std::string xml_Path = ti.o.outputFolderPath + DirSeparator + "tilemapresource.xml";
		std::ofstream outfile(xml_Path);
		if (outfile) {
			outfile << xml << std::endl;
			outfile.close();
		}
	}
};

class gdalToTMS_unity
{
public:
	gdalToTMS_unity() {};
	~gdalToTMS_unity() {};

public:
	void setTif(tmsInfo& ti, Grid& grid);
private:
	void buildServer(tmsInfo ti, Grid* grid, ServiceMetadata* metadata);
	void buildGDAL(tmsInfo ti, GDALSerializer& serializer, const RasterTiler& tiler, ServiceMetadata* metadata);
	void buildMesh(tmsInfo ti, MeshSerializer& serializer, const MeshTiler& tiler, ServiceMetadata* metadata);
	void buildMetadata(tmsInfo ti, const RasterTiler& tiler, ServiceMetadata* metadata);
	void buildTerrain(tmsInfo ti, TerrainSerializer& serializer, const TerrainTiler& tiler, ServiceMetadata* metadata);
	void buildJson(tmsInfo ti, Grid grid, ServiceMetadata* metadata);
	std::string createEmptyRootElevationFile(std::string& fileName, const Grid& grid, const TileCoordinate& coord);
	void reportProgress(tmsInfo ti, int completedCount);
private:

	std::mutex mtx;
private:
	int globalIteratorIndex = 0;
};

#endif
