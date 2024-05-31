#include "gdalToTMS_helper.h"

gdalToTMS_helper::gdalToTMS_helper()
{
	io_time::timeStart();
	geo_plugins::gdalRegister();
}

void gdalToTMS_helper::initialize(U_TMS u_param)
{
	u_Param.f_Basic.Input = u_param.f_Basic.Input;
	u_Param.f_Basic.OverlayFile = u_param.f_Basic.OverlayFile == 1 ? 1 : 0;
	u_Param.f_Basic.Output = u_param.f_Basic.Output;
	u_Param.f_Basic.OutputFormat = u_param.f_Basic.OutputFormat;
	u_Param.f_Basic.RunnableThread = u_param.f_Basic.RunnableThread < 1 ? 1 : u_param.f_Basic.RunnableThread;
	u_Param.f_Basic.UId = u_param.f_Basic.UId == NULL ? DEFAULT_UID : u_param.f_Basic.UId;

	u_Param.f_TMSConfig = u_param.f_TMSConfig;

	u_Param.f_Delegation = u_param.f_Delegation;


	m_callback.init("osgbToLod", u_Param.f_Basic.UId);
	m_callback.set("Delegate_Message", u_Param.f_Delegation.Delegate_Message);
	m_callback.set("Delegate_Progress", u_Param.f_Delegation.Delegate_Progress);
	m_callback.set("Delegate_Value", u_Param.f_Delegation.Delegate_Value);
}


bool gdalToTMS_helper::convert()
{
	getTifFiles();
	buildFiles();
	printLog();
	return true;
}

void gdalToTMS_helper::getTifFiles()
{
	io_composition::getList<entity_tms, std::string>(vec_entityTMS, u_Param.f_Basic.Input, u_Param.f_Basic.Output, tif_NoDot, "");


	for (entity_tms& et : vec_entityTMS) {
		et.gzip = u_Param.f_TMSConfig.Gzip == 1;
		et.writeVertexNormals = u_Param.f_TMSConfig.WriteVertexNormals == 1;
		et.specifiedHeight = u_Param.f_TMSConfig.SpecifiedHeight == 1;
		et.heightValue = u_Param.f_TMSConfig.HeightValue;
	}
	taskCount = vec_entityTMS.size();
	runnableThread = taskCount >= u_Param.f_Basic.RunnableThread ? u_Param.f_Basic.RunnableThread : taskCount;
}


void gdalToTMS_helper::buildFiles()
{
	std::mutex mtx;
	std::function<bool(int)> func = [&](int taskIndex) {
		if (map_taskInterval.count(taskIndex) > 0) {
			std::vector<int> vec_taskInterval = map_taskInterval[taskIndex];
			for (size_t i = 0; i < vec_taskInterval.size(); i++) {
				int index = vec_taskInterval[i];
				std::string result = "";
				entity_tms& et = vec_entityTMS[index];
				if (u_Param.f_Basic.OverlayFile) {
					io_file::deleteFolder(et.o.outputFolderPath);
				}
				io_file::mkdirs(et.o.outputFolderPath);
				et.threadIndex = taskIndex;
				Grid grid;
				getGrid(grid);

				gdalToTMS_unity unity;
				unity.set(u_Param, m_callback);
				unity.setTif(et, grid);

				mtx.lock();
				completedCount++;
				std::string strComplete = calculateComplete();
				std::string strMessage = io_log::appendBracket(et.i.inputFileName_WithoutExtension, bracketEnum::parenthesis) + " 已完成 " + strComplete;
				m_callback.sendMessage(strMessage);
				m_callback.sendProgress(completedCount, taskCount);
				mtx.unlock();
			}
		}
		return true;
		};
	async(func);
}

void gdalToTMS_helper::printLog()
{
	std::string message = "运行时间:" + io_log::appendBracket(io_time::secondToTime(io_time::timeEnd()));
	m_callback.sendMessage(message);
}

void gdalToTMS_helper::getGrid(Grid& grid)
{
	if (strcmp(u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRErr OGRErr_f = srs.importFromEPSG(4326);
		int tileSize = (u_Param.f_TMSConfig.TileSize < 1) ? 65 : u_Param.f_TMSConfig.TileSize;
		u_Param.f_TMSConfig.TileSize = tileSize;
		grid = GlobalGeodetic(srs, tileSize);
	}
	else if (strcmp(u_Param.f_TMSConfig.Profile, "mercator") == 0) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRErr OGRErr_f = srs.importFromEPSG(3857);
		int tileSize = (u_Param.f_TMSConfig.TileSize < 1) ? 256 : u_Param.f_TMSConfig.TileSize;
		u_Param.f_TMSConfig.TileSize = tileSize;
		grid = GlobalMercator(srs, tileSize);
	}
	else if (strcmp(u_Param.f_TMSConfig.Profile, "custom") == 0) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRErr result;
		std::string strCoordinate = io_file::toLower(u_Param.f_TMSConfig.CustomWKT);
		if (strCoordinate.find("epsg") == 0) {
			int epsgCode = 0;
			util_coordinate::splitEPSG(epsgCode, strCoordinate);
			result = srs.importFromEPSG(epsgCode);
		}
		else {
			result = srs.importFromWkt(strCoordinate.c_str());
		}
		if (result != OGRERR_NONE) return;
		int tileSize = (u_Param.f_TMSConfig.TileSize < 1) ? 256 : u_Param.f_TMSConfig.TileSize;
		u_Param.f_TMSConfig.TileSize = tileSize;
		grid = GlobalMercator(srs, tileSize);
	}
}
