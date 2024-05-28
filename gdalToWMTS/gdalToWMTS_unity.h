#ifndef _GDALTOWMTS_UNITY_H_
#define _GDALTOWMTS_UNITY_H_


#include <mutex>
#include <future>

#include <gdal_priv.h>
#include <gdalwarper.h>

#include <GBException.hpp>

#include <json/json.h>
#include <io_constant.h>
#include <io_log.h>
#include <io_file.h>
#include <json_helper.h>
#include <util_algorithm.h>

#include "gdalToWMTS_static.h"



class gdalToWMTS_unity
{
public:
	gdalToWMTS_unity() {};
	~gdalToWMTS_unity() {};

public:
	void setTif(wmtsInfo& wi);
private:
	void buildServer(wmtsInfo& wi);
	void buildTileNumbersFromCoords(std::vector<int>& tileNumbers, double xMin, double yMin, double xMax, double yMax, double tileSize, int zoom);
	void buildBaseTiles(wmtsInfo& wi);
	void buildTileBounds(std::vector<double>& TileBounds, int tileX, int tileY, int tileSize, int zoom);
	void buildGeoQuery(std::vector<std::vector<int>>& geoQuerys, const wmtsInfo wi, double upperLeftX, double upperLeftY, double lowerRightX, double lowerRightY);
	void createBaseTile(wmtsInfo& wi);
	void createScaleQueryToTile(GDALDataset* queryDataset, GDALDataset* tileDataset, wmtsInfo& wi);
	void createOverviewTiles(wmtsInfo& wi);
};

#endif
