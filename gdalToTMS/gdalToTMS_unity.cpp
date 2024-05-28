#include "gdalToTMS_unity.h"


void gdalToTMS_unity::setTif(tmsInfo& ti, Grid& grid)
{
	ServiceMetadata* metadata = new ServiceMetadata();

	buildServer(ti, &grid, metadata);
	// CesiumJS friendly?
	buildJson(ti, grid, metadata);
	// Write Json metadata file?
	if (metadata) {
		if (strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "terrain") == 0 ||
			strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "mesh") == 0) {
			metadata->writeJsonFile(ti);
		}
		else if (
			strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "jpg") == 0 ||
			strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "png") == 0 ||
			strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "tiff") == 0) {
			metadata->writeXmlFile(ti);
		}
		delete metadata;
	}
}

void gdalToTMS_unity::buildServer(tmsInfo ti, Grid* grid, ServiceMetadata* metadata)
{
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(ti.i.inputFilePath_UTF8.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法用GDAL识别数据:" + io_log::appendBracket(ti.i.inputFilePath));
		throw GBException("Error: could not open GDAL dataset");
	}

	// Metadata of only this thread, it will be joined to global later
	ServiceMetadata* threadMetadata = metadata ? new ServiceMetadata() : NULL;

	// Choose serializer of tiles (Directory of files, MBTiles store...)
	GBFileTileSerializer serializer(ti.o.outputFolderPath + DirSeparator, pStatic::u_Param.f_Basic.OverlayFile);

	try {
		serializer.startSerialization();

		if (pStatic::u_Param.f_TMSConfig.Metadata) {
			const RasterTiler tiler(poDataset, *grid);
			buildMetadata(ti, tiler, threadMetadata);
		}
		else if (strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "terrain") == 0) {
			const TerrainTiler tiler(poDataset, *grid);
			buildTerrain(ti, serializer, tiler, threadMetadata);
		}
		else if (strcmp(pStatic::u_Param.f_TMSConfig.TMSFormat, "mesh") == 0) {
			const MeshTiler tiler(poDataset, *grid, ti.specifiedHeight, 1.0);
			buildMesh(ti, serializer, tiler, threadMetadata);
		}
		else {
			const RasterTiler tiler(poDataset, *grid);
			buildGDAL(ti, serializer, tiler, threadMetadata);
		}

	}
	catch (GBException& e) {
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, e.what());
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

void gdalToTMS_unity::buildGDAL(tmsInfo ti, GDALSerializer& serializer, const RasterTiler& tiler, ServiceMetadata* metadata)
{
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pStatic::u_Param.f_TMSConfig.TMSFormat);

	if (poDriver == NULL) {
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法检索GDAL驱动程序");
		throw GBException("Could not retrieve GTiff GDAL driver");
	}

	if (poDriver->pfnCreateCopy == NULL) {
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法检索GDAL驱动程序。GDAL驱动程序必须启用读写功能。");
		throw GBException("The GDAL driver must be write enabled, specifically supporting 'CreateCopy'");
	}

	const char* extension = poDriver->GetMetadataItem(GDAL_DMD_EXTENSION);
	i_zoom startZoom = (pStatic::u_Param.f_TMSConfig.StartZoom < 0) ? tiler.maxZoomLevel() : pStatic::u_Param.f_TMSConfig.StartZoom;
	i_zoom endZoom = (pStatic::u_Param.f_TMSConfig.EndZoom < 0) ? 0 : pStatic::u_Param.f_TMSConfig.EndZoom;

	RasterIterator iter(tiler, startZoom, endZoom);
	int completedCount = ti.incrementIterator(iter, 0);
	ti.setTaskCount(iter);
	CPLStringList creationOptions;
	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);

		if (serializer.mustSerializeCoordinate(coordinate)) {
			GDALTile* tile = *iter;
			serializer.serializeTile(tile, poDriver, extension, creationOptions);
			delete tile;
		}

		completedCount = ti.incrementIterator(iter, completedCount);
		reportProgress(ti, completedCount);
	}
}

void gdalToTMS_unity::buildMesh(tmsInfo ti, MeshSerializer& serializer, const MeshTiler& tiler, ServiceMetadata* metadata)
{
	i_zoom startZoom = (pStatic::u_Param.f_TMSConfig.StartZoom < 0) ? tiler.maxZoomLevel() : pStatic::u_Param.f_TMSConfig.StartZoom;
	i_zoom endZoom = (pStatic::u_Param.f_TMSConfig.EndZoom < 0) ? 0 : pStatic::u_Param.f_TMSConfig.EndZoom;

	MeshIterator iter(tiler, startZoom, endZoom);
	int completedCount = ti.incrementIterator(iter, 0);
	ti.setTaskCount(iter);
	GDALDatasetReaderWithOverviews reader(tiler);

	while (!iter.exhausted()) {
		std::string filePath = "";
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);

		if (serializer.mustSerializeCoordinate(coordinate)) {
			MeshTile* tile = iter.operator*(&reader);
			serializer.serializeTile(tile, ti.gzip, ti.writeVertexNormals);
			delete tile;
		}

		completedCount = ti.incrementIterator(iter, completedCount);
		reportProgress(ti, completedCount);
	}
}

void gdalToTMS_unity::buildMetadata(tmsInfo ti, const RasterTiler& tiler, ServiceMetadata* metadata)
{
	i_zoom startZoom = (pStatic::u_Param.f_TMSConfig.StartZoom < 0) ? tiler.maxZoomLevel() : pStatic::u_Param.f_TMSConfig.StartZoom;
	i_zoom endZoom = (pStatic::u_Param.f_TMSConfig.EndZoom < 0) ? 0 : pStatic::u_Param.f_TMSConfig.EndZoom;

	RasterIterator iter(tiler, startZoom, endZoom);
	int completedCount = ti.incrementIterator(iter, 0);
	ti.setTaskCount(iter);

	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);
		completedCount = ti.incrementIterator(iter, completedCount);
		reportProgress(ti, completedCount);
	}
}

void gdalToTMS_unity::buildTerrain(tmsInfo ti, TerrainSerializer& serializer, const TerrainTiler& tiler, ServiceMetadata* metadata)
{

	i_zoom startZoom = (pStatic::u_Param.f_TMSConfig.StartZoom < 0) ? tiler.maxZoomLevel() : pStatic::u_Param.f_TMSConfig.StartZoom;
	i_zoom endZoom = (pStatic::u_Param.f_TMSConfig.EndZoom < 0) ? 0 : pStatic::u_Param.f_TMSConfig.EndZoom;


	TerrainIterator iter(tiler, startZoom, endZoom);
	int completedCount = ti.incrementIterator(iter, 0);
	ti.setTaskCount(iter);
	GDALDatasetReaderWithOverviews reader(tiler);

	while (!iter.exhausted()) {
		const TileCoordinate* coordinate = iter.GridIterator::operator*();
		if (metadata) metadata->add(tiler, coordinate);

		if (serializer.mustSerializeCoordinate(coordinate)) {
			TerrainTile* tile = iter.operator*(&reader);
			serializer.serializeTile(tile, pStatic::u_Param.f_TMSConfig.Gzip);
			delete tile;
		}

		completedCount = ti.incrementIterator(iter, completedCount);
		reportProgress(ti, completedCount);
	}
}

void gdalToTMS_unity::buildJson(tmsInfo ti, Grid grid, ServiceMetadata* metadata)
{
	if (pStatic::u_Param.f_TMSConfig.CesiumFriendly == 1 && (strcmp(pStatic::u_Param.f_TMSConfig.Profile, "geodetic") == 0) && pStatic::u_Param.f_TMSConfig.EndZoom <= 0) {

		// Create missing root tiles if it is necessary
		if (pStatic::u_Param.f_TMSConfig.Metadata == 1) {
			std::string dirName0 = ti.o.outputFolderPath + DirSeparator + "0" + DirSeparator + "0";
			std::string dirName1 = ti.o.outputFolderPath + DirSeparator + "0" + DirSeparator + "1";
			std::string tileName0 = dirName0 + DirSeparator + "0.terrain";
			std::string tileName1 = dirName1 + DirSeparator + "0.terrain";

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
				pStatic::u_Param.f_TMSConfig.StartZoom = 0;
				pStatic::u_Param.f_TMSConfig.EndZoom = 0;
				missingTileName = createEmptyRootElevationFile(missingTileName, grid, missingTileCoord);
				buildServer(ti, &grid, NULL);
				VSIUnlink(missingTileName.c_str());
			}
		}

		// Fix available indexes.
		if (metadata && metadata->levels.size() > 0) {
			ServiceMetadata::LevelInfo& level = metadata->levels.at(0);
			level.startX = 0;
			level.startY = 0;
			level.finalX = 1;
			level.finalY = 0;
		}
	}

}

std::string gdalToTMS_unity::createEmptyRootElevationFile(std::string& fileName, const Grid& grid, const TileCoordinate& coord)
{
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	if (poDriver == NULL) {
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法检索GDAL驱动程序");
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
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法创建EPSG:4326空间参考");
		throw GBException("Could not create EPSG:4326 spatial reference");
	}
	char* pszDstWKT = NULL;
	if (oSRS.exportToWkt(&pszDstWKT) != OGRERR_NONE) {
		CPLFree(pszDstWKT);
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法创建EPSG:4326 WKT字符串");
		throw GBException("Could not create EPSG:4326 WKT string");
	}

	// Create the GTiff file
	fileName += ".tif";
	GDALDataset* poDataset = poDriver->Create(fileName.c_str(), tileSize, tileSize, 1, GDT_Float32, NULL);

	// Set the projection
	if (poDataset->SetProjection(pszDstWKT) != CE_None) {
		poDataset->Release();
		CPLFree(pszDstWKT);
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法在临时高程文件上设置投影");
		throw GBException("Could not set projection on temporary elevation file");
	}
	CPLFree(pszDstWKT);

	// Apply the geo transform
	if (poDataset->SetGeoTransform(adfGeoTransform) != CE_None) {
		poDataset->Release();
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法在临时高程文件上设置投影");
		throw GBException("Could not set projection on temporary elevation file");
	}

	// Finally write the height data
	float* rasterHeights = (float*)CPLCalloc(tileSize * tileSize, sizeof(float));
	GDALRasterBand* heightsBand = poDataset->GetRasterBand(1);
	if (heightsBand->RasterIO(GF_Write, 0, 0, tileSize, tileSize,
		(void*)rasterHeights, tileSize, tileSize, GDT_Float32,
		0, 0) != CE_None) {
		CPLFree(rasterHeights);
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法在临时高程文件上写入高度");
		throw GBException("Could not write heights on temporary elevation file");
	}
	CPLFree(rasterHeights);

	poDataset->FlushCache();
	poDataset->Release();
	return fileName;
}

void gdalToTMS_unity::reportProgress(tmsInfo ti, int completedCount)
{
	mtx.lock();
	std::string message = "线程:" + io_log::appendBracket(ti.threadIndex) + "已完成:" + io_log::appendBracket(std::to_string(completedCount) + "/" + std::to_string(ti.taskCount));
	io_log::writeLog(pStatic::callback_Originator, LOG_MESSAGE, message);

	CallbackProgress callbackProgress;
	callbackProgress.ReferenceClass = GDALTOTMS_NAME;
	callbackProgress.ReferenceDisplayUId = pStatic::u_Param.f_Basic.UId;
	callbackProgress.Completed = completedCount;
	callbackProgress.Total = ti.taskCount;
	pStatic::callback_Originator.sendCallback<DelegateProgress>("Delegate_Progress", &callbackProgress);
	mtx.unlock();
}
