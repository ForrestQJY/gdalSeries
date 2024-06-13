#include "gdalToTMS_metadata.h"

void gdalToTMS_metadata::set(U_TMS u_param, callback cb)
{
	u_Param = u_param;
	m_callback = cb;
}

void gdalToTMS_metadata::add(const GDALTiler& tiler, const TileCoordinate* coordinate)
{
	CRSBounds _tileBounds = tiler.grid().tileBounds(*coordinate);
	CRSBounds _validBounds = tiler.bounds();
	i_zoom zoom = coordinate->zoom;

	maxZoom = (u_Param.f_TMSConfig.MaxZoom < 0) ? tiler.maxZoomLevel() : u_Param.f_TMSConfig.MaxZoom;
	minZoom = (u_Param.f_TMSConfig.MinZoom < 0) ? 0 : u_Param.f_TMSConfig.MinZoom;

	if ((1 + zoom) > vec_levelInfo.size()) {
		vec_levelInfo.resize(1 + zoom, levelInfo());
	}
	levelInfo& li = vec_levelInfo[zoom];
	li.add(coordinate);

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

void gdalToTMS_metadata::add(const gdalToTMS_metadata& otherMetadata)
{
	if (otherMetadata.vec_levelInfo.size() > 0) {
		const CRSBounds& _tileBounds = otherMetadata.bounds;
		const CRSBounds& _validBounds = otherMetadata.validBounds;

		if (otherMetadata.vec_levelInfo.size() > vec_levelInfo.size()) {
			vec_levelInfo.resize(otherMetadata.vec_levelInfo.size(), levelInfo());
		}
		for (size_t i = 0; i < otherMetadata.vec_levelInfo.size(); i++) {
			vec_levelInfo[i].add(otherMetadata.vec_levelInfo[i]);
		}
		maxZoom = (u_Param.f_TMSConfig.MaxZoom < 0) ? otherMetadata.vec_levelInfo.size() - 1 : u_Param.f_TMSConfig.MaxZoom;
		minZoom = (u_Param.f_TMSConfig.MinZoom < 0) ? 0 : u_Param.f_TMSConfig.MinZoom;

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

void gdalToTMS_metadata::writeJsonFile(entity_tms ti)
{
	double validMinX = validBounds.getMinX();
	double validMinY = validBounds.getMinY();
	double validMaxX = validBounds.getMaxX();
	double validMaxY = validBounds.getMaxY();

	bool ziped = u_Param.f_TMSConfig.Gzip == 1;
	std::string strEpsgCode;
	int iEpsgCode = 0;

	if (strcmp(u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
		strEpsgCode = "EPSG:4326";
		iEpsgCode = 4326;
	}
	else if (strcmp(u_Param.f_TMSConfig.Profile, "mercator") == 0) {
		strEpsgCode = "EPSG:3857";
		iEpsgCode = 3857;
	}
	else if (strcmp(u_Param.f_TMSConfig.Profile, "custom") == 0) {
		std::string strWKT = u_Param.f_TMSConfig.CustomWKT;
		strWKT = io_file::toLower(strWKT);
		if (io_utily::find(strWKT, "epsg")) {
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
	if (u_Param.f_TMSConfig.WriteVertexNormals) {
		valueLayer["extensions"].append("octvertexnormals");
	}
	else {
		valueLayer["extensions"].append(util_algorithm::getJsonEmptyArray());
	}
	valueLayer["extensions"].clear();
	if (strcmp(u_Param.f_TMSConfig.TMSFormat, "terrain") == 0) {
		valueLayer["format"] = "heightmap-1.0";
	}
	else if (strcmp(u_Param.f_TMSConfig.TMSFormat, "mesh") == 0) {
		valueLayer["format"] = "quantized-mesh-1.0";
	}
	else {
		valueLayer["format"] = "GDAL";
	}
	valueLayer["name"] = ti.i.inputFileName_WithoutExtension;
	if (strcmp(u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
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
	for (size_t i = 0, icount = vec_levelInfo.size(); i < icount; i++) {
		const levelInfo& level = vec_levelInfo[i];
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
	valueMeta["maxzoom"] = maxZoom;
	valueMeta["minzoom"] = minZoom;
	if (strcmp(u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
		valueMeta["proj"] = 4326;
	}
	else {
		valueMeta["proj"] = 3857;
	}
	valueMeta["tiletrans"] = "tms";
	valueMeta["type"] = u_Param.f_TMSConfig.TMSFormat;
	valueMeta["ziped"] = ziped;

	std::string layer_Path = ti.o.outputFolderPath + symbol_dir + "layer" + symbol_ext + format_json;
	json_helper::writeJson(layer_Path, valueLayer);

	std::string meta_Path = ti.o.outputFolderPath + symbol_dir + "meta" + symbol_ext + format_json;
	json_helper::writeJson(meta_Path, valueMeta);
}

void gdalToTMS_metadata::writeXmlFile(entity_tms ti)
{
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

	std::string strMinX = io_utily::toString(validMinX);
	std::string strMinY = io_utily::toString(validMinY);
	std::string strMaxX = io_utily::toString(validMaxX);
	std::string strMaxY = io_utily::toString(validMaxY);
	std::string strEpsgCode;
	int iEpsgCode = 0;
	if (io_utily::find(u_Param.f_TMSConfig.Profile, "geodetic")) {
		strEpsgCode = "EPSG:4326";
		iEpsgCode = 4326;
	}
	else if (io_utily::find(u_Param.f_TMSConfig.Profile, "mercator")) {
		strEpsgCode = "EPSG:3857";
		iEpsgCode = 3857;
	}
	else if (io_utily::find(u_Param.f_TMSConfig.Profile, "custom")) {
		std::string strWKT = u_Param.f_TMSConfig.CustomWKT;
		strWKT = io_file::toLower(strWKT);
		if (io_utily::find(strWKT, "epsg")) {
			strEpsgCode = strWKT;
			iEpsgCode = std::stoi(io_file::replace(strWKT, "epsg:", ""));
		}
		else {
			strEpsgCode = "PROJ";
		}
	}
	std::string strCTBFormat = u_Param.f_TMSConfig.TMSFormat;
	std::string strTileSize = io_utily::toString(u_Param.f_TMSConfig.TileSize);
	std::string strMimeType = strCTBFormat == format_jpg ? "image/jpeg" : strCTBFormat == format_png ? "image/png" : "image/tiff";
	std::string strProfile = u_Param.f_TMSConfig.Profile;


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

	for (int i = 0; i < vec_levelInfo.size(); i++)
	{
		std::string strIndex = io_utily::toString(i);
		std::string strPixel = "";
		if (iEpsgCode == 4326) {
			double pixel = 0.703125 / std::pow(2, i);
			strPixel = io_utily::toString(pixel);
		}
		else if (iEpsgCode == 3857) {
			double pixel = 78271.516 / std::pow(2, i);
			strPixel = io_utily::toString(pixel);
		}
		xml.append("<TileSet href=\"" + strIndex + "\" units-per-pixel=\"" + strPixel + "\" order=\"" + strIndex + "\"/>");
		xml.append("\n");
	}
	xml.append("</TileSets>");
	xml.append("\n");
	xml.append("</TileMap>");


	std::string xml_Path = ti.o.outputFolderPath + symbol_dir + "tilemapresource" + symbol_ext + format_xml;
	std::ofstream outfile(xml_Path);
	if (outfile) {
		outfile << xml << std::endl;
		outfile.close();
	}
}
