#include "gdalToWMTS_unity.h"

void gdalToWMTS_unity::setTif(wmtsInfo& wi)
{
	wi.maxZoom = pStatic::u_Param.f_WMTSConfig.MaxZoom;
	wi.minZoom = pStatic::u_Param.f_WMTSConfig.MinZoom;
	wi.resampling = pStatic::u_Param.f_WMTSConfig.ResampleAlg;
	wi.querySize = 4 * pStatic::u_Param.f_WMTSConfig.TileSize;
	wi.tileSize = pStatic::u_Param.f_WMTSConfig.TileSize;
	buildServer(wi);
	buildBaseTiles(wi);
	createBaseTile(wi);
}

void gdalToWMTS_unity::buildServer(wmtsInfo& wi)
{
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(wi.i.inputFilePath_UTF8.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		io_log::writeLog(pStatic::callback_Originator, LOG_ERROR, "无法用GDAL识别数据:" + io_log::appendBracket(wi.i.inputFilePath));
		throw gb::GBException("Error: could not open GDAL dataset");
	}
	wi.dataBandsCount = poDataset->GetRasterCount();
	double geoTransform[6];
	if (poDataset->GetGeoTransform(geoTransform) == CE_None) {
		std::vector<double> result(std::begin(geoTransform), std::end(geoTransform));
		wi.vec_geoTransform = std::move(result);
	}
	wi.rasterXSize = poDataset->GetRasterXSize();
	wi.rasterYSize = poDataset->GetRasterYSize();

	GDALClose(poDataset);
	if (wi.maxZoom < wi.minZoom)return;
	for (size_t i = wi.minZoom; i <= wi.maxZoom; i++) {
		double xMin = wi.vec_geoTransform[0];
		double yMin = wi.vec_geoTransform[3] - wi.rasterYSize * wi.vec_geoTransform[1];
		double xMax = wi.vec_geoTransform[0] + wi.rasterXSize * wi.vec_geoTransform[1];
		double yMax = wi.vec_geoTransform[3];

		std::vector<int> tileNumbers;
		buildTileNumbersFromCoords(tileNumbers, xMin, yMin, xMax, yMax, wi.tileSize, i);

		int tileMinX = tileNumbers[0];
		int tileMinY = tileNumbers[1];
		int tileMaxX = tileNumbers[2];
		int tileMaxY = tileNumbers[3];

		//tileMinX = std::max(0, tileMinX);
		//tileMinY = std::max(0, tileMinY);
		//tileMaxX = std::min((int)(std::pow(2, i + 1)) - 1, tileMaxX);
		//tileMaxY = std::min((int)(std::pow(2, i)) - 1, tileMaxY);
		std::vector<int> tileValue{ tileMinX, tileMinY, tileMaxX, tileMaxY };
		wi.map_lodMinMax.emplace(i, tileValue);
	}
}

void gdalToWMTS_unity::buildTileNumbersFromCoords(std::vector<int>& tileNumbers, double xMin, double yMin, double xMax, double yMax, double tileSize, int zoom)
{
	double resolution = 180.0 / tileSize / std::pow(2, zoom);

	int xMinR = std::numeric_limits<int>::max();
	int xMaxR = std::numeric_limits<int>::min();
	int yMinR = std::numeric_limits<int>::max();
	int yMaxR = std::numeric_limits<int>::min();

	xMinR = std::ceil((180.0 + xMin) / resolution / tileSize) - 1.0;
	xMaxR = std::ceil((180.0 + xMax) / resolution / tileSize) - 1.0;


	yMinR = std::ceil((90 + yMin) / resolution / tileSize) - 1.0;
	yMaxR = std::ceil((90 + yMax) / resolution / tileSize) - 1.0;

	tileNumbers.push_back(std::min(xMinR, xMaxR));
	tileNumbers.push_back(std::min(yMinR, yMaxR));
	tileNumbers.push_back(std::max(xMinR, xMaxR));
	tileNumbers.push_back(std::max(yMinR, yMaxR));
}

void gdalToWMTS_unity::buildBaseTiles(wmtsInfo& wi)
{

	for (int currentY = wi.map_lodMinMax[wi.maxZoom][1]; currentY <= wi.map_lodMinMax[wi.maxZoom][3]; currentY++)
	{
		for (int currentX = wi.map_lodMinMax[wi.maxZoom][0]; currentX <= wi.map_lodMinMax[wi.maxZoom][2]; currentX++)
		{
			std::string outputFolder = wi.o.outputFolderPath + DirSeparator + std::to_string(wi.maxZoom) + DirSeparator + std::to_string(currentX);
			io_file::mkdirs(outputFolder);
			std::vector<double> tileBounds;
			buildTileBounds(tileBounds, currentX, currentY, wi.tileSize, wi.maxZoom);
			std::vector<std::vector<int>> geoQuerys;
			buildGeoQuery(geoQuerys, wi, tileBounds[0], tileBounds[3], tileBounds[2], tileBounds[1]);

			std::map<std::string, int> metadatas;
			metadatas.emplace("TileX", currentX);
			metadatas.emplace("TileX", currentX);
			metadatas.emplace("TileY", currentY);
			metadatas.emplace("TileZoom", wi.maxZoom);
			metadatas.emplace("ReadPosX", geoQuerys[0][0]);
			metadatas.emplace("ReadPosY", geoQuerys[0][1]);
			metadatas.emplace("ReadXSize", geoQuerys[0][2]);
			metadatas.emplace("ReadYSize", geoQuerys[0][3]);
			metadatas.emplace("WritePosX", geoQuerys[1][0]);
			metadatas.emplace("WritePosY", geoQuerys[1][1]);
			metadatas.emplace("WriteXSize", geoQuerys[1][2]);
			metadatas.emplace("WriteYSize", geoQuerys[1][3]);
			metadatas.emplace("QuerySize", wi.querySize);
			wi.vec_metadata.push_back(metadatas);
		}
	}
}

void gdalToWMTS_unity::buildTileBounds(std::vector<double>& TileBounds, int tileX, int tileY, int tileSize, int zoom)
{
	double resolution = 180.0 / tileSize / std::pow(2, zoom);
	TileBounds.push_back(tileX * tileSize * resolution - 180.0);
	TileBounds.push_back(tileY * tileSize * resolution - 90.0);
	TileBounds.push_back((tileX + 1) * tileSize * resolution - 180.0);
	TileBounds.push_back((tileY + 1) * tileSize * resolution - 90.0);
}

void gdalToWMTS_unity::buildGeoQuery(std::vector<std::vector<int>>& geoQuerys, const wmtsInfo wi, double upperLeftX, double upperLeftY, double lowerRightX, double lowerRightY)
{
	double readXPos = (upperLeftX - wi.vec_geoTransform[0]) / wi.vec_geoTransform[1] + 0.001;
	double readYPos = (upperLeftY - wi.vec_geoTransform[3]) / wi.vec_geoTransform[5] + 0.001;
	double readXSize = (lowerRightX - upperLeftX) / wi.vec_geoTransform[1] + 0.5;
	double readYSize = (lowerRightY - upperLeftY) / wi.vec_geoTransform[5] + 0.5;
	double writeXSize = wi.querySize;
	double writeYSize = wi.querySize;

	double writeXPos = 0.0;
	if (readXPos < 0.0) {
		double readXShift = std::abs(readXPos);
		writeXPos = writeXSize * readXShift / readXSize;
		writeXSize = writeXSize - writeXPos;
		readXSize = readXSize - readXSize * readXShift / readXSize;
		readXPos = 0.0;
	}

	if (readXPos + readXSize > wi.rasterXSize) {
		writeXSize = writeXSize * (wi.rasterXSize - readXPos) / readXSize;
		readXSize = wi.rasterXSize - readXPos;
	}

	double writeYPos = 0.0;
	if (readYPos < 0.0) {
		double readYShift = std::abs(readYPos);
		writeYPos = writeYSize * readYShift / readYSize;
		writeYSize = writeYSize - writeYPos;
		readYSize = readYSize - readYSize * readYShift / readYSize;
		readYPos = 0.0;
	}

	if (readYPos + readYSize > wi.rasterYSize) {
		writeYSize = writeYSize * (wi.rasterYSize - readYPos) / readYSize;
		readYSize = wi.rasterYSize - readYPos;
	}
	std::vector<int> r1{ (int)readXPos, (int)readYPos, (int)readXSize, (int)readYSize };
	std::vector<int> r2{ (int)writeXPos,(int)writeYPos,(int)writeXSize,(int)writeYSize };
	geoQuerys.push_back(r1);
	geoQuerys.push_back(r2);
}

void gdalToWMTS_unity::createBaseTile(wmtsInfo& wi)
{
	GDALDriver* poDriverMEM = GetGDALDriverManager()->GetDriverByName("MEM");
	int metadataSize = wi.vec_metadata.size();
	std::map<int, std::vector<int>> map_taskInterval;

	std::function<bool(int)> runServer = [&](int taskIndex) {
		if (map_taskInterval.count(taskIndex) > 0) {
			std::vector<int> vec_taskInterval = map_taskInterval[taskIndex];
			for (size_t i = 0; i < vec_taskInterval.size(); i++) {
				int index = vec_taskInterval[i];
				std::string result = "";
				std::map<std::string, int> metadata = wi.vec_metadata[index];

				GDALDataset* tileDataset = poDriverMEM->Create("", wi.tileSize, wi.tileSize, wi.dataBandsCount + 1, GDT_Byte, NULL);

				byte* data = (byte*)CPLCalloc(metadata["WriteXSize"] * metadata["WriteYSize"] * wi.dataBandsCount, sizeof(byte));
				byte* alpha = (byte*)CPLCalloc(metadata["WriteXSize"] * metadata["WriteYSize"], sizeof(byte));

				int* dataBandsArray = new int[wi.dataBandsCount];
				for (size_t i = 0; i < wi.dataBandsCount; i++) {
					dataBandsArray[i] = i + 1;
				}
				int* dataBandsArrayNew = new int[1];
				dataBandsArrayNew[0] = wi.dataBandsCount + 1;

				if (metadata["ReadXSize"] != 0 && metadata["ReadYSize"] != 0 && metadata["WriteXSize"] != 0 && metadata["WriteYSize"] != 0) {

					GDALDataset* inputDataset = (GDALDataset*)GDALOpen(wi.i.inputFilePath_UTF8.c_str(), GA_ReadOnly);

					inputDataset->RasterIO(GF_Read, metadata["ReadPosX"], metadata["ReadPosY"], metadata["ReadXSize"], metadata["ReadYSize"], (void*)data, metadata["WriteXSize"], metadata["WriteYSize"], GDT_Byte, wi.dataBandsCount, dataBandsArray, 0, 0, 0
					);
					GDALRasterBand* heightsBand = inputDataset->GetRasterBand(1);
					heightsBand->RasterIO(GF_Read, metadata["ReadPosX"], metadata["ReadPosY"], metadata["ReadXSize"], metadata["ReadYSize"], (void*)alpha, metadata["WriteXSize"], metadata["WriteYSize"], GDT_Byte, 0, 0
					);

				}
				if (data == NULL)return false;
				GDALDataset* queryDataset = poDriverMEM->Create("", metadata["QuerySize"], metadata["QuerySize"], wi.dataBandsCount + 1, GDT_Byte, NULL);


				queryDataset->RasterIO(GF_Write, metadata["WritePosX"], metadata["WritePosY"], metadata["WriteXSize"], metadata["WriteYSize"], data, metadata["WriteXSize"], metadata["WriteYSize"], GDT_Byte, wi.dataBandsCount, dataBandsArray, 0, 0, 0);

				queryDataset->RasterIO(GF_Write, metadata["WritePosX"], metadata["WritePosY"], metadata["WriteXSize"], metadata["WriteYSize"], alpha, metadata["WriteXSize"], metadata["WriteYSize"], GDT_Byte, 1, dataBandsArrayNew, 0, 0, 0);
				createScaleQueryToTile(queryDataset, tileDataset, wi);


				GDALDriver* poDriverPNG = GetGDALDriverManager()->GetDriverByName("PNG");
				std::string tilePath = wi.o.outputFolderPath + DirSeparator + std::to_string(metadata["TileZoom"]) + std::to_string(metadata["TileX"]) + std::to_string(metadata["TileY"]) + ExtSeparator + pStatic::u_Param.f_Basic.OutputFormat;

				std::string tilePath_UTF8 = io_file::stringToUTF8(tilePath);
				poDriverPNG->CreateCopy(tilePath_UTF8.c_str(), tileDataset, 0, NULL, NULL, NULL);
			}
		}
		return true;
		};

	int size = map_taskInterval.size();
	if (size <= 0) {
		for (size_t i = 0; i < 4; i++) {
			std::vector<int> vec_indexs;
			map_taskInterval.emplace(i, vec_indexs);
			for (size_t j = i; j < metadataSize; j += 4) {
				if (map_taskInterval.count(i) > 0) {
					map_taskInterval[i].push_back(j);
				}
			}
		}
	}
	std::vector<std::thread> vec_threads;
	for (size_t i = 0; i < 4; i++) {
		vec_threads.push_back(std::thread(runServer, i));
	}

	std::for_each(vec_threads.begin(), vec_threads.end(), [](std::thread& thr) {thr.join(); });
	map_taskInterval.clear();
}

void gdalToWMTS_unity::createScaleQueryToTile(GDALDataset* queryDataset, GDALDataset* tileDataset, wmtsInfo& wi)
{
	double geoTransformQuery[6];
	geoTransformQuery[0] = 0.0;
	geoTransformQuery[1] = tileDataset->GetRasterXSize() / queryDataset->GetRasterXSize();
	geoTransformQuery[2] = 0.0;
	geoTransformQuery[3] = 0.0;
	geoTransformQuery[4] = 0.0;
	geoTransformQuery[5] = tileDataset->GetRasterXSize() / queryDataset->GetRasterXSize();
	queryDataset->SetGeoTransform(geoTransformQuery);
	double geoTransformTile[6];
	geoTransformTile[0] = 0.0;
	geoTransformTile[1] = 1.0;
	geoTransformTile[2] = 0.0;
	geoTransformTile[3] = 0.0;
	geoTransformTile[4] = 0.0;
	geoTransformTile[5] = 1.0;
	tileDataset->SetGeoTransform(geoTransformTile);

	CPLErr eErr = GDALReprojectImage(queryDataset, NULL, tileDataset, NULL, GDALResampleAlg(wi.resampling), 0.0, 0.0, NULL, NULL, NULL);

	if (eErr != CE_None) {

	}
}

void gdalToWMTS_unity::createOverviewTiles(wmtsInfo& wi)
{
}
