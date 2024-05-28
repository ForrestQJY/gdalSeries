#include "gdalToWMTS_helper.h"

gdalToWMTS_helper::gdalToWMTS_helper()
{
	geo_plugins::gdalRegister();
}

void gdalToWMTS_helper::initialize(U_WMTS u_WMTS)
{
	time_helper::timeStart();//记录开始时间
	pStatic::setParameters(u_WMTS);
}


bool gdalToWMTS_helper::convert()
{
	getTifFiles();
	buildFiles();
	printLog();
	return true;
}

void gdalToWMTS_helper::getTifFiles()
{
	io_composition::getList<wmtsInfo, std::string>(vec_wmtsInfo, pStatic::u_Param.f_Basic.Input, pStatic::u_Param.f_Basic.Output, tif_NoDot, "");
	for (wmtsInfo& wi : vec_wmtsInfo) {

	}
	taskCount = vec_wmtsInfo.size();
	runnableThread = taskCount >= pStatic::u_Param.f_Basic.RunnableThread ? pStatic::u_Param.f_Basic.RunnableThread : taskCount;
}


void gdalToWMTS_helper::buildFiles()
{
	CallbackProgress callbackProgress(GDALTOWMTS_NAME, pStatic::u_Param.f_Basic.UId, completedCount, taskCount);


	std::mutex mtx;
	std::function<bool(int)> runTerrainServer = [&](int taskIndex) {
		if (map_taskInterval.count(taskIndex) > 0) {
			std::vector<int> vec_taskInterval = map_taskInterval[taskIndex];
			for (size_t i = 0; i < vec_taskInterval.size(); i++) {
				int index = vec_taskInterval[i];
				std::string result = "";
				wmtsInfo& wi = vec_wmtsInfo[index];
				if (pStatic::u_Param.f_Basic.OverlayFile) {
					io_file::deleteFolder(wi.o.outputFolderPath);
				}
				io_file::mkdirs(wi.o.outputFolderPath);

				wmtsUnity.setTif(wi);

				//mtx.lock();
				//completedCount++;
				//std::string strCompletedCount = std::to_string(completedCount);
				//std::string strTaskCount = std::to_string(taskCount);
				//std::string strSupplement(strTaskCount.length() - strCompletedCount.length(), '0');
				//strCompletedCount = strSupplement + strCompletedCount;
				//std::string message = io_log::appendBracket(wi.i.inputFileName_WithoutExtension, bracketEnum::parenthesis) + " 已完成 " + io_log::appendBracket(strCompletedCount + "/" + strTaskCount, bracketEnum::parenthesis);
				//io_log::writeLog(pStatic::callback_Originator, LOG_MESSAGE, message);

				//callbackProgress.Completed = completedCount;
				//pStatic::callback_Originator.sendCallback<DelegateProgress>("Delegate_Progress", &callbackProgress);
				//mtx.unlock();
			}
		}
		return true;
		};
	io_thread threadHelper;
	threadHelper.Run(runnableThread, taskCount, map_taskInterval, runTerrainServer);
}

void gdalToWMTS_helper::printLog()
{
	std::string message = "运行时间:" + io_log::appendBracket(time_helper::secondToTime(time_helper::timeEnd()));
	io_log::writeLog(pStatic::callback_Originator, LOG_MESSAGE, message);
}
