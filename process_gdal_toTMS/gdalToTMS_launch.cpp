#include "gdalToTMS_launch.h"
gdalToTMS_launch::gdalToTMS_launch()
{
	carryStart();
	m_gdal.gdalRegister();
}

void gdalToTMS_launch::initialize(U_TMS u_param)
{
	m_param = io_transfer::transfer<U_TMS, param_TMS>(u_param);
	init("gdalToTMS", m_param);
}

bool gdalToTMS_launch::toImagery()
{
	getTifFiles();
	buildFiles();
	carryEnd();
	return true;
}


bool gdalToTMS_launch::toTerrain()
{
	getTifFiles();
	buildFiles();
	carryEnd();
	return true;
}

void gdalToTMS_launch::getTifFiles()
{
	vec_entityTMS = io_composition::getList<entity_tms>(m_param.pBasic, format_tif, "");
	for (entity_tms& et : vec_entityTMS) {
		et.gzip = m_param.pTMS.gzip == 1;
		et.writeVertexNormals = m_param.pTMSQuality.writeVertexNormals == 1;
	}
	taskCount = vec_entityTMS.size();
	m_param.pBasic.runnableThread = runnableThread = std::min(m_param.pBasic.runnableThread, taskCount);
	if (m_param.pTMS.maxZoom < m_param.pTMS.minZoom || m_param.pTMS.maxZoom < 0 || m_param.pTMS.minZoom < 0) {
		m_param.pTMS.maxZoom = 0;
		m_param.pTMS.minZoom = 0;
	}
	m_param.printTMS(m_callback);
}
void gdalToTMS_launch::buildFiles()
{
	for (entity_tms et : vec_entityTMS) {
		if (m_param.pBasic.overlayFile) {
			io_file::deleteFolder(et.o.path_folder);
		}
		io_file::mkdirs(et.o.path_folder);
		Grid grid;
		getGrid(grid);
		gdalToTMS_unity unity;
		unity.set(m_param, m_callback);
		unity.setTif(et, grid);
	}


	// Run the tilers in separate threads
	//Grid grid;
	//getGrid(grid);
	//std::vector<std::future<int>> tasks;
	//int threadCount = 1;
	////(command.threadCount > 0) ? command.threadCount : CPLGetNumCPUs()
	//// Calculate metadata?
	//const std::string dirname = m_param.pBasic.output + symbol_dir;
	//const std::string filename = dirname + "layer.json";
	//gdalToTMS_metadata* metadata = new gdalToTMS_metadata();

	//// Instantiate the threads using futures from a packaged_task
	//for (int i = 0; i < threadCount; ++i) {
	//	std::packaged_task<int(param_TMS*, Grid*, gdalToTMS_metadata*)> task(runTiler); // wrap the function
	//	tasks.push_back(task.get_future()); // get a future
	//	std::thread(std::move(task), &m_param, &grid, metadata).detach(); // launch on a thread
	//}

	//// Synchronise the completion of the threads
	//for (auto& task : tasks) {
	//	task.wait();
	//}

	//// Get the value from the futures
	//for (auto& task : tasks) {
	//	int retval = task.get();

	//	// return on the first encountered problem
	//	if (retval) {
	//		delete metadata;
	//		return retval;
	//	}
	//}

	/*std::mutex mtx;
	std::function<bool(int)> func = [&](int taskIndex) {
		if (map_taskQueue.count(taskIndex) > 0) {
			std::vector<int> vec_taskQueue = map_taskQueue[taskIndex];
			for (size_t i = 0; i < vec_taskQueue.size(); i++) {
				int index = vec_taskQueue[i];
				std::string result = "";
				entity_tms& et = vec_entityTMS[index];
				if (m_param.pBasic.overlayFile) {
					io_file::deleteFolder(et.o.path_folder);
				}
				io_file::mkdirs(et.o.path_folder);
				et.threadIndex = taskIndex;
				Grid grid;
				getGrid(grid);

				gdalToTMS_unity unity;
				unity.set(m_param, m_callback);
				unity.setTif(et, grid);

				std::unique_lock<std::mutex> lock(mtx);
				taskEnd(taskIndex, et.o.file_name_within_extension);
				lock.unlock();
			}
		}
		return true;
		};
	async(func);*/
}

void gdalToTMS_launch::getGrid(Grid& grid)
{
	if (io_utily::find(m_param.pTMS.geographicProjectionFormat, "geodetic")) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		if (!m_param.pTMS.customSpatial.empty()) {
			OGRErr result;
			std::string strCoordinate = io_utily::toLower(m_param.pTMS.customSpatial);
			if (io_utily::find(strCoordinate, "epsg")) {
				int epsgCode = 0;
				m_spatial.splitEPSG(epsgCode, strCoordinate);
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
	else if (io_utily::find(m_param.pTMS.geographicProjectionFormat, "mercator")) {
		OGRSpatialReference srs;
		srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		if (!m_param.pTMS.customSpatial.empty()) {
			OGRErr result;
			std::string strCoordinate = io_utily::toLower(m_param.pTMS.customSpatial);
			if (io_utily::find(strCoordinate, "epsg")) {
				int epsgCode = 0;
				m_spatial.splitEPSG(epsgCode, strCoordinate);
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
//static int
//runTiler(const char* inputFilename, param_TMS* param, Grid* grid, gdalToTMS_metadata* metadata) {
//	GDALDataset* poDataset = (GDALDataset*)GDALOpen(inputFilename, GA_ReadOnly);
//	if (poDataset == NULL) {
//		return 1;
//	}
//
//	// Metadata of only this thread, it will be joined to global later
//	gdalToTMS_metadata* threadMetadata = metadata ? new gdalToTMS_metadata() : NULL;
//
//	// Choose serializer of tiles (Directory of files, MBTiles store...)
//	GBFileTileSerializer serializer(param->pBasic.output, param->pBasic.overlayFile);
//
//	serializer.startSerialization();
//
//	if (command->metadata) {
//		const RasterTiler tiler(poDataset, *grid, command->tilerOptions);
//		buildMetadata(tiler, command, threadMetadata);
//	}
//	else if (strcmp(command->outputFormat, "Terrain") == 0) {
//		const TerrainTiler tiler(poDataset, *grid);
//		buildTerrain(serializer, tiler, command, threadMetadata);
//	}
//	else if (strcmp(command->outputFormat, "Mesh") == 0) {
//		const MeshTiler tiler(poDataset, *grid, command->tilerOptions, command->meshQualityFactor);
//		buildMesh(serializer, tiler, command, threadMetadata, command->vertexNormals);
//	}
//	else {                    // it's a GDAL format
//		const RasterTiler tiler(poDataset, *grid, command->tilerOptions);
//		buildGDAL(serializer, tiler, command, threadMetadata);
//	}
//
//
//	serializer.endSerialization();
//
//	GDALClose(poDataset);
//
//	// Pass metadata to global instance.
//	if (threadMetadata) {
//		static std::mutex mutex;
//		std::lock_guard<std::mutex> lock(mutex);
//
//		metadata->add(*threadMetadata);
//		delete threadMetadata;
//	}
//	return 0;
//}