#ifndef _TOTMS_UNITY_H_
#define _TOTMS_UNITY_H_


#include <mutex>

#include <json/json.h>

#include <future>


#include <GBFileTileSerializer.hpp>
#include <GDALDatasetReader.hpp>
#include <GDALSerializer.hpp>
#include <GlobalGeodetic.hpp>
#include <GlobalMercator.hpp>
#include <Grid.hpp>
#include <MeshIterator.hpp>
#include <MeshSerializer.hpp>
#include <MeshTiler.hpp>
#include <RasterIterator.hpp>
#include <TerrainTiler.hpp>
#include <TerrainIterator.hpp>
#include <TerrainSerializer.hpp>

#include <io_class.h>
#include <io_constant.h>
#include <io_entity.h>
#include <io_file.h>
#include <io_json.h>
#include <io_utily.h>
#include <util_algorithm.h>

#include "toTMS_metadata.h"


using namespace gb;


class toTMS_unity :public io_class
{
public:
	toTMS_unity() {};
	~toTMS_unity() {};

public:
	void set(param_TMS p, callback cb);
	void setTif(entity_tms& et, Grid& grid);
private:
	void buildServer(entity_tms et, Grid grid, toTMS_metadata* metadata);
	void buildGDAL(entity_tms et, GDALSerializer& serializer, const RasterTiler& tiler, toTMS_metadata* metadata);
	void buildMesh(entity_tms et, MeshSerializer& serializer, const MeshTiler& tiler, toTMS_metadata* metadata);
	void buildMetadata(entity_tms et, const RasterTiler& tiler, toTMS_metadata* metadata);
	void buildTerrain(entity_tms et, TerrainSerializer& serializer, const TerrainTiler& tiler, toTMS_metadata* metadata);
	void buildJson(entity_tms et, Grid grid, toTMS_metadata* metadata);

	std::string createEmptyRootElevationFile(std::string& fileName, const Grid& grid, const TileCoordinate& coord);
	void reportProgress(entity_tms et, int completedCount);
private:
	std::mutex mtx;
private:
	int globalIteratorIndex = 0;
private:
	param_TMS m_param;
};

#endif
