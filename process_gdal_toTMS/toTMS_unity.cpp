#include "toTMS_unity.h"


void toTMS_unity::set(param_TMS p, callback cb)
{
	m_param = p;
	m_callback = cb;
}


void toTMS_unity::setTif(entity_tms& et, Grid& grid)
{
	int threadCount = m_param.pBasic.runnableThread > 0 ? m_param.pBasic.runnableThread : CPLGetNumCPUs();
	gb::TilerOptions tilerOptions;
	toTMS_metadata* metadata = new toTMS_metadata();
	metadata->set(m_param, m_callback);
	std::function<void()> func = [&]() {
		metadata->set(m_param, m_callback);
		GDALDataset* poDataset = (GDALDataset*)GDALOpen(et.i.path_file_utf8.c_str(), GA_ReadOnly);
		if (poDataset == NULL) {
			m_callback.sendError("无法用GDAL识别数据:" + io_utily::appendBracket(et.i.path_file));
			throw GBException("Error: could not open GDAL dataset");
		}
		toTMS_metadata* threadMetadata = NULL;
		if (metadata) {
			threadMetadata = new toTMS_metadata();
			threadMetadata->set(m_param, m_callback);
		}
		GBFileTileSerializer serializer(et.o.path_folder, m_param.pBasic.overlayFile);
		try {
			serializer.startSerialization();
			if (m_param.pTMSQuality.cesiumMetadata) {
				const RasterTiler tiler(poDataset, grid, tilerOptions);
				buildMetadata(et, tiler, threadMetadata);
			}
			else if (m_param.pTMS.tmsFormat == "terrain") {
				const TerrainTiler tiler(poDataset, grid, tilerOptions);
				buildTerrain(et, serializer, tiler, threadMetadata);
			}
			else if (m_param.pTMS.tmsFormat == "mesh") {
				const MeshTiler tiler(poDataset, grid, tilerOptions, m_param.pTMSQuality.meshQualityFactor);
				buildMesh(et, serializer, tiler, threadMetadata);
			}
			else {
				const RasterTiler tiler(poDataset, grid, tilerOptions);
				buildGDAL(et, serializer, tiler, threadMetadata);
			}

		}
		catch (GBException& e) {
			m_callback.sendError(e.what());
		}

		serializer.endSerialization();
		GDALClose(poDataset);
		if (threadMetadata) {
			static std::mutex mutex;
			std::lock_guard<std::mutex> lock(mutex);

			metadata->add(*threadMetadata);
			delete threadMetadata;
		}
		};

	std::vector<std::future<void>> tasks;
	for (int i = 0; i < threadCount; ++i) {
		std::packaged_task<void()> task(func);
		tasks.push_back(task.get_future());
		std::thread(std::move(task)).detach();
	}
	for (auto& task : tasks) {
		task.wait();
	}

	buildJson(et, grid, metadata);
	if (metadata) {
		if (m_param.pTMS.tmsFormat == "terrain" ||
			m_param.pTMS.tmsFormat == "mesh") {
			metadata->writeJsonFile(et);
		}
		else if (
			m_param.pTMS.tmsFormat == "jpg" ||
			m_param.pTMS.tmsFormat == "png" ||
			m_param.pTMS.tmsFormat == "tiff") {
			metadata->writeXmlFile(et);
		}
		delete metadata;
	}
}

void toTMS_unity::buildServer(entity_tms et, Grid grid, toTMS_metadata* metadata)
{
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(et.i.path_file_utf8.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		m_callback.sendError("无法用GDAL识别数据:" + io_utily::appendBracket(et.i.path_file));
		throw GBException("Error: could not open GDAL dataset");
	}
	toTMS_metadata* threadMetadata = NULL;
	if (metadata) {
		threadMetadata = new toTMS_metadata();
		threadMetadata->set(m_param, m_callback);
	}

	GBFileTileSerializer serializer(et.o.path_folder, m_param.pBasic.overlayFile);

	try {
		serializer.startSerialization();

		if (m_param.pTMSQuality.cesiumMetadata) {
			const RasterTiler tiler(poDataset, grid);
			buildMetadata(et, tiler, threadMetadata);
		}
		else if (m_param.pTMS.tmsFormat == "terrain") {
			const TerrainTiler tiler(poDataset, grid);
			buildTerrain(et, serializer, tiler, threadMetadata);
		}
		else if (m_param.pTMS.tmsFormat == "mesh") {
			const MeshTiler tiler(poDataset, grid, m_param.pTMSQuality.meshQualityFactor);
			buildMesh(et, serializer, tiler, threadMetadata);
		}
		else {
			const RasterTiler tiler(poDataset, grid);
			buildGDAL(et, serializer, tiler, threadMetadata);
		}

	}
	catch (GBException& e) {
		m_callback.sendError(e.what());
	}

	serializer.endSerialization();
	GDALClose(poDataset);
	if (threadMetadata) {
		static std::mutex mutex;
		std::lock_guard<std::mutex> lock(mutex);

		metadata->add(*threadMetadata);
		delete threadMetadata;
	}
	return;
}

void toTMS_unity::buildGDAL(entity_tms et, GDALSerializer& serializer, const RasterTiler& tiler, toTMS_metadata* metadata)
{
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(m_param.pTMS.tmsFormat.c_str());

	if (poDriver == NULL) {
		m_callback.sendError("无法检索GDAL驱动程序");
		throw GBException("Could not retrieve GTiff GDAL driver");
	}

	if (poDriver->pfnCreateCopy == NULL) {
		m_callback.sendError("无法检索GDAL驱动程序。GDAL驱动程序必须启用读写功能。");
		throw GBException("The GDAL driver must be write enabled, specifically supporting 'CreateCopy'");
	}

	const char* extension = poDriver->GetMetadataItem(GDAL_DMD_EXTENSION);
	
	i_zoom maxZoom = tiler.maxZoomLevel();
	i_zoom minZoom = 0;
	if (m_param.pTMS.maxZoom > 0) {
		maxZoom = m_param.pTMS.maxZoom;
	}
	if (m_param.pTMS.minZoom > 0) {
		minZoom = m_param.pTMS.minZoom;
	}

	RasterIterator iter(tiler, maxZoom, minZoom);
	int completedCount = et.incrementIterator(iter, 0);
	et.setTaskCount(iter);
	CPLStringList creationOptions;
	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);

		if (serializer.mustSerializeCoordinate(coordinate)) {
			GDALTile* tile = *iter;
			serializer.serializeTile(tile, poDriver, extension, creationOptions);
			delete tile;
		}
		completedCount = et.incrementIterator(iter, completedCount);
		reportProgress(et, completedCount);
	}
}

void toTMS_unity::buildMesh(entity_tms et, MeshSerializer& serializer, const MeshTiler& tiler, toTMS_metadata* metadata)
{
	i_zoom maxZoom = tiler.maxZoomLevel();
	i_zoom minZoom = 0;
	if (m_param.pTMS.maxZoom > 0) {
		maxZoom = m_param.pTMS.maxZoom;
	}
	if (m_param.pTMS.minZoom > 0) {
		minZoom = m_param.pTMS.minZoom;
	}
	MeshIterator iter(tiler, maxZoom, minZoom);
	int completedCount = et.incrementIterator(iter, 0);
	et.setTaskCount(iter);
	GDALDatasetReaderWithOverviews reader(tiler);

	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);

		if (serializer.mustSerializeCoordinate(coordinate)) {
			MeshTile* tile = iter.operator*(&reader);
			serializer.serializeTile(tile, et.gzip, et.writeVertexNormals);
			delete tile;
		}
		completedCount = et.incrementIterator(iter, completedCount);
		reportProgress(et, completedCount);
	}
}

void toTMS_unity::buildMetadata(entity_tms et, const RasterTiler& tiler, toTMS_metadata* metadata)
{
	i_zoom maxZoom = tiler.maxZoomLevel();
	i_zoom minZoom = 0;
	if (m_param.pTMS.maxZoom > 0) {
		maxZoom = m_param.pTMS.maxZoom;
	}
	if (m_param.pTMS.minZoom > 0) {
		minZoom = m_param.pTMS.minZoom;
	}
	RasterIterator iter(tiler, maxZoom, minZoom);
	int completedCount = et.incrementIterator(iter, 0);
	et.setTaskCount(iter);

	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);
		completedCount = et.incrementIterator(iter, completedCount);
		reportProgress(et, completedCount);
	}
}

void toTMS_unity::buildTerrain(entity_tms et, TerrainSerializer& serializer, const TerrainTiler& tiler, toTMS_metadata* metadata)
{
	i_zoom maxZoom = tiler.maxZoomLevel();
	i_zoom minZoom = 0;
	if (m_param.pTMS.maxZoom > 0) {
		maxZoom = m_param.pTMS.maxZoom;
	}
	if (m_param.pTMS.minZoom > 0) {
		minZoom = m_param.pTMS.minZoom;
	}

	TerrainIterator iter(tiler, maxZoom, minZoom);
	int completedCount = et.incrementIterator(iter, 0);
	et.setTaskCount(iter);
	GDALDatasetReaderWithOverviews reader(tiler);

	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);

		if (serializer.mustSerializeCoordinate(coordinate)) {
			TerrainTile* tile = iter.operator*(&reader);
			serializer.serializeTile(tile, m_param.pTMS.gzip);
			delete tile;
		}
		completedCount = et.incrementIterator(iter, completedCount);
		reportProgress(et, completedCount);
	}
}

void toTMS_unity::buildJson(entity_tms et, Grid grid, toTMS_metadata* metadata)
{
	if (m_param.pTMSQuality.cesiumFriendly && m_param.pTMS.geographicProjectionFormat == "geodetic" && m_param.pTMS.minZoom <= 0) {

		// Create missing root tiles if it is necessary
		if (m_param.pTMSQuality.cesiumMetadata) {
			std::string dirName0 = et.o.path_folder + symbol_dir + "0" + symbol_dir + "0";
			std::string dirName1 = et.o.path_folder + symbol_dir + "0" + symbol_dir + "1";
			std::string tileName0 = dirName0 + symbol_dir + "0" + symbol_ext + format_terrain;
			std::string tileName1 = dirName1 + symbol_dir + "0" + symbol_ext + format_terrain;

			i_zoom missingZoom = 65535;
			gb::TileCoordinate missingTileCoord(missingZoom, 0, 0);
			std::string missingTileName;

			if (io_file::exists(tileName0) && !io_file::exists(tileName1)) {
				io_file::mkdirs(dirName1);
				missingTileCoord = gb::TileCoordinate(0, 1, 0);
				missingTileName = tileName1;
			}
			else
				if (!io_file::exists(tileName0) && io_file::exists(tileName1)) {
					io_file::mkdirs(dirName0);
					missingTileCoord = gb::TileCoordinate(0, 0, 0);
					missingTileName = tileName0;
				}
			if (missingTileCoord.zoom != missingZoom) {
				globalIteratorIndex = 0; // reset global iterator index
				m_param.pTMS.maxZoom = 0;
				m_param.pTMS.minZoom = 0;
				missingTileName = createEmptyRootElevationFile(missingTileName, grid, missingTileCoord);
				buildServer(et, grid, NULL);
				VSIUnlink(missingTileName.c_str());
			}
		}

		// Fix available indexes.
		if (metadata && metadata->vec_levelInfo.size() > 0) {
			levelInfo& li = metadata->vec_levelInfo.at(0);
			li.startX = 0;
			li.startY = 0;
			li.finalX = 1;
			li.finalY = 0;
		}
	}

}

std::string toTMS_unity::createEmptyRootElevationFile(std::string& fileName, const Grid& grid, const TileCoordinate& coord)
{
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	if (poDriver == NULL) {
		m_callback.sendError("无法检索GDAL驱动程序");
		throw GBException("Could not retrieve GTiff GDAL driver");
	}

	// Create the geo transform for this temporary elevation tile.
	// We apply an 1-degree negative offset to avoid problems in borders.
	CRSBounds tileBounds = grid.tileBounds(coord);
	tileBounds.setMinX(tileBounds.getMinX() + 1);
	tileBounds.setMinY(tileBounds.getMinY() + 1);
	tileBounds.setMaxX(tileBounds.getMaxX() - 1);
	tileBounds.setMaxY(tileBounds.getMaxY() - 1);
	const i_tile tileSize = grid.tileSize() - 2;
	const double resolution = tileBounds.getWidth() / tileSize;
	double adfGeoTransform[6] = { tileBounds.getMinX(), resolution, 0, tileBounds.getMaxY(), 0, -resolution };

	// Create the spatial reference system for the file
	OGRSpatialReference oSRS;

#if ( GDAL_VERSION_MAJOR >= 3 )
	oSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif

	if (oSRS.importFromEPSG(4326) != OGRERR_NONE) {
		m_callback.sendError("无法创建EPSG:4326空间参考");
		throw GBException("Could not create EPSG:4326 spatial reference");
	}
	char* pszDstWKT = NULL;
	if (oSRS.exportToWkt(&pszDstWKT) != OGRERR_NONE) {
		CPLFree(pszDstWKT);
		m_callback.sendError("无法创建EPSG:4326 WKT字符串");
		throw GBException("Could not create EPSG:4326 WKT string");
	}

	// Create the GTiff file
	fileName += ".tif";
	GDALDataset* poDataset = poDriver->Create(fileName.c_str(), tileSize, tileSize, 1, GDT_Float32, NULL);

	// Set the projection
	if (poDataset->SetProjection(pszDstWKT) != CE_None) {
		poDataset->Release();
		CPLFree(pszDstWKT);
		m_callback.sendError("无法在临时高程文件上设置投影");
		throw GBException("Could not set projection on temporary elevation file");
	}
	CPLFree(pszDstWKT);

	// Apply the geo transform
	if (poDataset->SetGeoTransform(adfGeoTransform) != CE_None) {
		poDataset->Release();
		m_callback.sendError("无法在临时高程文件上设置投影");
		throw GBException("Could not set projection on temporary elevation file");
	}

	// Finally write the height data
	float* rasterHeights = (float*)CPLCalloc(tileSize * tileSize, sizeof(float));
	GDALRasterBand* heightsBand = poDataset->GetRasterBand(1);
	if (heightsBand->RasterIO(GF_Write, 0, 0, tileSize, tileSize,
		(void*)rasterHeights, tileSize, tileSize, GDT_Float32,
		0, 0) != CE_None) {
		CPLFree(rasterHeights);
		m_callback.sendError("无法在临时高程文件上写入高度");
		throw GBException("Could not write heights on temporary elevation file");
	}
	CPLFree(rasterHeights);

	poDataset->FlushCache();
	poDataset->Release();
	return fileName;
}

void toTMS_unity::reportProgress(entity_tms et, int completedCount)
{
	mtx.lock();
	std::string strTask = io_utily::lengthSplicing(completedCount, et.taskCount, "/");
	std::string strMessage = "已完成 " + strTask;
	m_callback.sendMessage(strMessage);
	m_callback.sendProgress(completedCount, et.taskCount);
	mtx.unlock();
}