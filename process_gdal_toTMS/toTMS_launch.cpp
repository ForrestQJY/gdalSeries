#include "toTMS_launch.h"

toTMS_launch::toTMS_launch()
{
	carryStart();
	m_gdal.gdalRegister();
}

void toTMS_launch::initialize(U_TMS u_param)
{
	m_param = io_transfer::transfer<U_TMS, param_TMS>(u_param);
	init("gdalToTMS", m_param);
}

bool toTMS_launch::toImagery()
{
	getTifFiles();
	buildFiles();
	carryEnd();
	return true;
}


bool toTMS_launch::toTerrain()
{
	getTifFiles();
	buildFiles();
	carryEnd();
	return true;
}

void toTMS_launch::getTifFiles()
{
	vec_entityTMS = io_composition::getList<entity_tms>(m_param.pBasic, format_tif, "");
	for (entity_tms& et : vec_entityTMS) {
		et.gzip = m_param.pTMS.gzip == 1;
		et.writeVertexNormals = m_param.pTMSQuality.writeVertexNormals;
	}
	taskCount = vec_entityTMS.size();
	m_param.pBasic.runnableThread = runnableThread = std::min(m_param.pBasic.runnableThread, taskCount);
	if (m_param.pTMS.maxZoom < m_param.pTMS.minZoom || m_param.pTMS.maxZoom < 0 || m_param.pTMS.minZoom < 0) {
		m_param.pTMS.maxZoom = 0;
		m_param.pTMS.minZoom = 0;
	}
	m_param.printTMS(m_callback);
}
void toTMS_launch::buildFiles()
{
	for (entity_tms et : vec_entityTMS) {
		if (m_param.pBasic.overlayFile) {
			io_file::deleteFolder(et.o.path_folder);
		}
		io_file::mkdirs(et.o.path_folder);
		Grid grid;
		if (!getGrid(grid)) {
			m_callback.sendError("无法初始化目标地理坐标系参考。");
		}
		toTMS_unity unity;
		unity.set(m_param, m_callback);
		unity.setTif(et, grid);
	}
}

bool toTMS_launch::getGrid(Grid& grid)
{
	if (io_utily::find(m_param.pTMS.geographicProjectionFormat, "geodetic")) {
		OGRSpatialReference srs;
		OGRErr result;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		if (!m_param.pTMS.customSpatial.empty()) {
			std::string strCoordinate = io_utily::toLower(m_param.pTMS.customSpatial);
			if (io_utily::find(strCoordinate, "epsg")) {
				int epsgCode = 0;
				m_spatial.splitEPSG(epsgCode, strCoordinate);
				result = srs.importFromEPSG(epsgCode);
			}
			else {
				result = srs.importFromWkt(strCoordinate.c_str());
			}
			if (result != OGRERR_NONE) return false;
		}
		else {
			result = srs.importFromEPSG(4326);
			if (result != OGRERR_NONE) {
				OGRErr result = srs.SetWellKnownGeogCS("WGS84");
			}
		}
		int tileSize = (m_param.pTMS.tileSize < 1) ? 65 : m_param.pTMS.tileSize;
		m_param.pTMS.tileSize = tileSize;
		grid = GlobalGeodetic(srs, tileSize);
		return true;
	}
	else if (io_utily::find(m_param.pTMS.geographicProjectionFormat, "mercator")) {
		OGRSpatialReference srs;
		OGRErr result;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		if (!m_param.pTMS.customSpatial.empty()) {
			std::string strCoordinate = io_utily::toLower(m_param.pTMS.customSpatial);
			if (io_utily::find(strCoordinate, "epsg")) {
				int epsgCode = 0;
				m_spatial.splitEPSG(epsgCode, strCoordinate);
				result = srs.importFromEPSG(epsgCode);
			}
			else {
				result = srs.importFromWkt(strCoordinate.c_str());
			}
			if (result != OGRERR_NONE) return false;
		}
		else {
			result = srs.importFromEPSG(3857);
		}
		int tileSize = (m_param.pTMS.tileSize < 1) ? 256 : m_param.pTMS.tileSize;
		m_param.pTMS.tileSize = tileSize;
		grid = GlobalMercator(srs, tileSize);
		return true;
	}
	return false;
}
