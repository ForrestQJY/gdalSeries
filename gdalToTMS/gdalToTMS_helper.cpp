#include "gdalToTMS_helper.h"
gdalToTMS_helper::gdalToTMS_helper()
{
	carryStart();
	m_gdal.gdalRegister();
}

void gdalToTMS_helper::initialize(U_TMS u_param)
{
	m_param = io_transfer::transfer<U_TMS, param_TMS>(u_param);
	init("gdalToTMS", m_param);
}


bool gdalToTMS_helper::convert()
{
	getTifFiles();
	buildFiles();
	carryEnd(m_param.pBasic.output);
	return true;
}

void gdalToTMS_helper::getTifFiles()
{
	io_composition::getList<entity_tms, std::string>(vec_entityTMS, m_param.pBasic.input, m_param.pBasic.output, format_tif, "");


	for (entity_tms& et : vec_entityTMS) {
		et.gzip = m_param.pTMS.gzip == 1;
		et.writeVertexNormals = m_param.pTMS.writeVertexNormals == 1;
	}
	taskCount = vec_entityTMS.size();
	m_param.pBasic.runnableThread = runnableThread = std::min(m_param.pBasic.runnableThread, taskCount);
	m_param.printTMS(m_callback);
}


void gdalToTMS_helper::buildFiles()
{
	std::function<bool(int)> func = [&](int taskIndex) {
		if (map_taskInterval.count(taskIndex) > 0) {
			std::vector<int> vec_taskInterval = map_taskInterval[taskIndex];
			for (size_t i = 0; i < vec_taskInterval.size(); i++) {
				int index = vec_taskInterval[i];
				std::string result = "";
				entity_tms& et = vec_entityTMS[index];
				if (m_param.pBasic.overlayFile) {
					io_file::deleteFolder(et.o.outputFolderPath);
				}
				io_file::mkdirs(et.o.outputFolderPath);
				et.threadIndex = taskIndex;
				Grid grid;
				getGrid(grid);

				gdalToTMS_unity unity;
				unity.set(m_param, m_callback);
				unity.setTif(et, grid);

				progress(taskIndex, et.o.outputFileName_WithinExtension);
			}
		}
		return true;
		};
	async(func);
}

void gdalToTMS_helper::getGrid(Grid& grid)
{
	if (io_utily::find(m_param.pTMS.profile, "geodetic")) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		if (!m_param.pTMS.customSpatial.empty()) {
			OGRErr result;
			std::string strCoordinate = io_file::toLower(m_param.pTMS.customSpatial);
			if (io_utily::find(strCoordinate, "epsg")) {
				int epsgCode = 0;
				util_coordinate::splitEPSG(epsgCode, strCoordinate);
				result = srs.importFromEPSG(epsgCode);
			}
			else {
				result = srs.importFromWkt(strCoordinate.c_str());
			}
			if (result != OGRERR_NONE) return;
		}
		else {
			OGRErr OGRErr_f = srs.importFromEPSG(4326);
		}
		int tileSize = (m_param.pTMS.tileSize < 1) ? 65 : m_param.pTMS.tileSize;
		m_param.pTMS.tileSize = tileSize;
		grid = GlobalGeodetic(srs, tileSize);
	}
	else if (io_utily::find(m_param.pTMS.profile, "mercator")) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		if (!m_param.pTMS.customSpatial.empty()) {
			OGRErr result;
			std::string strCoordinate = io_file::toLower(m_param.pTMS.customSpatial);
			if (io_utily::find(strCoordinate, "epsg")) {
				int epsgCode = 0;
				util_coordinate::splitEPSG(epsgCode, strCoordinate);
				result = srs.importFromEPSG(epsgCode);
			}
			else {
				result = srs.importFromWkt(strCoordinate.c_str());
			}
			if (result != OGRERR_NONE) return;
		}
		else {
			OGRErr OGRErr_f = srs.importFromEPSG(3857);
		}
		int tileSize = (m_param.pTMS.tileSize < 1) ? 256 : m_param.pTMS.tileSize;
		m_param.pTMS.tileSize = tileSize;
		grid = GlobalMercator(srs, tileSize);
	}
}
