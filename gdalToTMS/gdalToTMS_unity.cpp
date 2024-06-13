#include "gdalToTMS_unity.h"


void gdalToTMS_unity::set(U_TMS u_param, callback cb)
{
	u_Param = u_param;
	m_callback = cb;
}
void gdalToTMS_unity::setTif(entity_tms& et, Grid& grid)
{
	gdalToTMS_metadata* metadata = new gdalToTMS_metadata();
	metadata->set(u_Param, m_callback);

	buildServer(et, &grid, metadata);
	buildJson(et, grid, metadata);
	if (metadata) {
		if (io_utily::equals(u_Param.f_TMSConfig.TMSFormat, "terrain") ||
			io_utily::equals(u_Param.f_TMSConfig.TMSFormat, "mesh")) {
			metadata->writeJsonFile(et);
		}
		else if (
			io_utily::equals(u_Param.f_TMSConfig.TMSFormat, "jpg") ||
			io_utily::equals(u_Param.f_TMSConfig.TMSFormat, "png") ||
			io_utily::equals(u_Param.f_TMSConfig.TMSFormat, "tiff")) {
			metadata->writeXmlFile(et);
		}
		delete metadata;
	}
}

void gdalToTMS_unity::buildServer(entity_tms et, Grid* grid, gdalToTMS_metadata* metadata)
{
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(et.i.inputFilePath_UTF8.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		m_callback.sendError("无法用GDAL识别数据:" + io_utily::appendBracket(et.i.inputFilePath));
		throw GBException("Error: could not open GDAL dataset");
	}
	gdalToTMS_metadata* threadMetadata = NULL;
	if (metadata) {
		threadMetadata = new gdalToTMS_metadata();
		threadMetadata->set(u_Param, m_callback);
	}

	GBFileTileSerializer serializer(et.o.outputFolderPath + symbol_dir, u_Param.f_Basic.OverlayFile);

	try {
		serializer.startSerialization();

		if (u_Param.f_TMSConfig.Metadata) {
			const RasterTiler tiler(poDataset, *grid);
			buildMetadata(et, tiler, threadMetadata);
		}
		else if (io_utily::equals(u_Param.f_TMSConfig.TMSFormat, "terrain")) {
			const TerrainTiler tiler(poDataset, *grid);
			buildTerrain(et, serializer, tiler, threadMetadata);
		}
		else if (io_utily::equals(u_Param.f_TMSConfig.TMSFormat, "mesh")) {
			const MeshTiler tiler(poDataset, *grid, et.specifiedHeight, 1.0);
			buildMesh(et, serializer, tiler, threadMetadata);
		}
		else {
			const RasterTiler tiler(poDataset, *grid);
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

void gdalToTMS_unity::buildGDAL(entity_tms et, GDALSerializer& serializer, const RasterTiler& tiler, gdalToTMS_metadata* metadata)
{
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(u_Param.f_TMSConfig.TMSFormat);

	if (poDriver == NULL) {
		m_callback.sendError("无法检索GDAL驱动程序");
		throw GBException("Could not retrieve GTiff GDAL driver");
	}

	if (poDriver->pfnCreateCopy == NULL) {
		m_callback.sendError("无法检索GDAL驱动程序。GDAL驱动程序必须启用读写功能。");
		throw GBException("The GDAL driver must be write enabled, specifically supporting 'CreateCopy'");
	}

	const char* extension = poDriver->GetMetadataItem(GDAL_DMD_EXTENSION);
	i_zoom maxZoom = (u_Param.f_TMSConfig.MaxZoom < 0) ? tiler.maxZoomLevel() : u_Param.f_TMSConfig.MaxZoom;
	i_zoom minZoom = (u_Param.f_TMSConfig.MinZoom < 0) ? 0 : u_Param.f_TMSConfig.MinZoom;

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

void gdalToTMS_unity::buildMesh(entity_tms et, MeshSerializer& serializer, const MeshTiler& tiler, gdalToTMS_metadata* metadata)
{
	i_zoom maxZoom = (u_Param.f_TMSConfig.MaxZoom < 0) ? tiler.maxZoomLevel() : u_Param.f_TMSConfig.MaxZoom;
	i_zoom minZoom = (u_Param.f_TMSConfig.MinZoom < 0) ? 0 : u_Param.f_TMSConfig.MinZoom;

	MeshIterator iter(tiler, maxZoom, minZoom);
	int completedCount = et.incrementIterator(iter, 0);
	et.setTaskCount(iter);
	GDALDatasetReaderWithOverviews reader(tiler);

	while (!iter.exhausted()) {
		std::string filePath = "";
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

void gdalToTMS_unity::buildMetadata(entity_tms et, const RasterTiler& tiler, gdalToTMS_metadata* metadata)
{
	i_zoom maxZoom = (u_Param.f_TMSConfig.MaxZoom < 0) ? tiler.maxZoomLevel() : u_Param.f_TMSConfig.MaxZoom;
	i_zoom minZoom = (u_Param.f_TMSConfig.MinZoom < 0) ? 0 : u_Param.f_TMSConfig.MinZoom;

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

void gdalToTMS_unity::buildTerrain(entity_tms et, TerrainSerializer& serializer, const TerrainTiler& tiler, gdalToTMS_metadata* metadata)
{

	i_zoom maxZoom = (u_Param.f_TMSConfig.MaxZoom < 0) ? tiler.maxZoomLevel() : u_Param.f_TMSConfig.MaxZoom;
	i_zoom minZoom = (u_Param.f_TMSConfig.MinZoom < 0) ? 0 : u_Param.f_TMSConfig.MinZoom;


	TerrainIterator iter(tiler, maxZoom, minZoom);
	int completedCount = et.incrementIterator(iter, 0);
	et.setTaskCount(iter);
	GDALDatasetReaderWithOverviews reader(tiler);

	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);

		if (serializer.mustSerializeCoordinate(coordinate)) {
			TerrainTile* tile = iter.operator*(&reader);
			serializer.serializeTile(tile, u_Param.f_TMSConfig.Gzip);
			delete tile;
		}

		completedCount = et.incrementIterator(iter, completedCount);
		reportProgress(et, completedCount);
	}
}

void gdalToTMS_unity::buildJson(entity_tms et, Grid grid, gdalToTMS_metadata* metadata)
{
	if (u_Param.f_TMSConfig.CesiumFriendly == 1 && io_utily::equals(u_Param.f_TMSConfig.Profile, "geodetic") && u_Param.f_TMSConfig.MinZoom <= 0) {

		// Create missing root tiles if it is necessary
		if (u_Param.f_TMSConfig.Metadata == 1) {
			std::string dirName0 = et.o.outputFolderPath + symbol_dir + "0" + symbol_dir + "0";
			std::string dirName1 = et.o.outputFolderPath + symbol_dir + "0" + symbol_dir + "1";
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
				u_Param.f_TMSConfig.MaxZoom = 0;
				u_Param.f_TMSConfig.MinZoom = 0;
				missingTileName = createEmptyRootElevationFile(missingTileName, grid, missingTileCoord);
				buildServer(et, &grid, NULL);
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

std::string gdalToTMS_unity::createEmptyRootElevationFile(std::string& fileName, const Grid& grid, const TileCoordinate& coord)
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

void gdalToTMS_unity::reportProgress(entity_tms et, int completedCount)
{
	mtx.lock();
	std::string strMessage = "线程:" + io_utily::appendBracket(et.threadIndex) + "已完成:" + io_utily::appendBracket(io_utily::toString(completedCount) + "/" + io_utily::toString(et.taskCount));
	m_callback.sendMessage(strMessage);
	m_callback.sendProgress(completedCount, taskCount);
	mtx.unlock();
}
