#include "gdalToTMS_helper.h"

gdalToTMS_helper::gdalToTMS_helper()
{
	geo_plugins::gdalRegister();
}

void gdalToTMS_helper::initialize(U_TMS u_TMS)
{
	io_time::timeStart();//记录开始时间
	pStatic::setParameters(u_TMS);
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
	io_composition::getList<tmsInfo, std::string>(vec_tmsInfo, pStatic::u_Param.f_Basic.Input, pStatic::u_Param.f_Basic.Output, tif_NoDot, "");


	for (tmsInfo& ti : vec_tmsInfo) {
		ti.gzip = pStatic::u_Param.f_TMSConfig.Gzip == 1;
		ti.writeVertexNormals = pStatic::u_Param.f_TMSConfig.WriteVertexNormals == 1;
		ti.specifiedHeight = pStatic::u_Param.f_TMSConfig.SpecifiedHeight == 1;
		ti.heightValue = pStatic::u_Param.f_TMSConfig.HeightValue;
	}
	taskCount = vec_tmsInfo.size();
	runnableThread = taskCount >= pStatic::u_Param.f_Basic.RunnableThread ? pStatic::u_Param.f_Basic.RunnableThread : taskCount;
}


void gdalToTMS_helper::buildFiles()
{
	CallbackProgress callbackProgress(GDALTOTMS_NAME, pStatic::u_Param.f_Basic.UId, completedCount, taskCount);


	std::mutex mtx;
	std::function<bool(int)> runTerrainServer = [&](int taskIndex) {
		if (map_taskInterval.count(taskIndex) > 0) {
			std::vector<int> vec_taskInterval = map_taskInterval[taskIndex];
			for (size_t i = 0; i < vec_taskInterval.size(); i++) {
				int index = vec_taskInterval[i];
				std::string result = "";
				tmsInfo& ti = vec_tmsInfo[index];
				if (pStatic::u_Param.f_Basic.OverlayFile) {
					io_file::deleteFolder(ti.o.outputFolderPath);
				}
				io_file::mkdirs(ti.o.outputFolderPath);
				ti.threadIndex = taskIndex;
				Grid grid;
				getGrid(grid);
				tmsUnity.setTif(ti, grid);

				mtx.lock();
				completedCount++;
				std::string strCompletedCount = std::to_string(completedCount);
				std::string strTaskCount = std::to_string(taskCount);
				std::string strSupplement(strTaskCount.length() - strCompletedCount.length(), '0');
				strCompletedCount = strSupplement + strCompletedCount;
				std::string message = io_log::appendBracket(ti.i.inputFileName_WithoutExtension, bracketEnum::parenthesis) + " 已完成 " + io_log::appendBracket(strCompletedCount + "/" + strTaskCount, bracketEnum::parenthesis);
				io_log::writeLog(pStatic::callback_Originator, LOG_MESSAGE, message);

				callbackProgress.Completed = completedCount;
				pStatic::callback_Originator.sendCallback<DelegateProgress>("Delegate_Progress", &callbackProgress);
				mtx.unlock();
			}
		}
		return true;
		};
	io_thread ioThread;
	ioThread.Run(runnableThread, taskCount, map_taskInterval, runTerrainServer);
}

void gdalToTMS_helper::printLog()
{
	std::string message = "运行时间:" + io_log::appendBracket(io_time::secondToTime(io_time::timeEnd()));
	io_log::writeLog(pStatic::callback_Originator, LOG_MESSAGE, message);
}

void gdalToTMS_helper::getGrid(Grid& grid)
{
	if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "geodetic") == 0) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRErr OGRErr_f = srs.importFromEPSG(4326);
		int tileSize = (pStatic::u_Param.f_TMSConfig.TileSize < 1) ? 65 : pStatic::u_Param.f_TMSConfig.TileSize;
		pStatic::u_Param.f_TMSConfig.TileSize = tileSize;
		grid = GlobalGeodetic(srs, tileSize);
	}
	else if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "mercator") == 0) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRErr OGRErr_f = srs.importFromEPSG(3857);
		int tileSize = (pStatic::u_Param.f_TMSConfig.TileSize < 1) ? 256 : pStatic::u_Param.f_TMSConfig.TileSize;
		pStatic::u_Param.f_TMSConfig.TileSize = tileSize;
		grid = GlobalMercator(srs, tileSize);
	}
	else if (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "custom") == 0) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		OGRErr result;
		std::string strCoordinate = io_file::toLower(pStatic::u_Param.f_TMSConfig.CustomWKT);
		if (strCoordinate.find("epsg") == 0) {
			int epsgCode = 0;
			util_coordinate::splitEPSG(epsgCode, strCoordinate);
			result = srs.importFromEPSG(epsgCode);
		}
		else {
			result = srs.importFromWkt(strCoordinate.c_str());
		}
		if (result != OGRERR_NONE) return;
		int tileSize = (pStatic::u_Param.f_TMSConfig.TileSize < 1) ? 256 : pStatic::u_Param.f_TMSConfig.TileSize;
		pStatic::u_Param.f_TMSConfig.TileSize = tileSize;
		grid = GlobalMercator(srs, tileSize);
	}
}
