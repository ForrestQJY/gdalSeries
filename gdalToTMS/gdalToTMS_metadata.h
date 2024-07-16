#ifndef _GDALTOTMS_METADATA_H_
#define _GDALTOTMS_METADATA_H_

#include <vector>

#include <GDALTiler.hpp>
#include <TileCoordinate.hpp>
#include <types.hpp>

#include <io_class.h>
#include <io_file.h>
#include <io_utily.h>
#include <io_json.h>
#include <util_algorithm.h>
#include <util_entity.h>

using namespace gb;

class levelInfo {
public:
	levelInfo() {
		startX = startY = io_utily::getNumberMax<int>();
		finalX = finalY = io_utily::getNumberMin<int>();
	}
	inline void add(const TileCoordinate* coordinate) {
		startX = std::min(startX, (int)coordinate->x);
		startY = std::min(startY, (int)coordinate->y);
		finalX = std::max(finalX, (int)coordinate->x);
		finalY = std::max(finalY, (int)coordinate->y);
	}
	inline void add(const levelInfo& level) {
		startX = std::min(startX, level.startX);
		startY = std::min(startY, level.startY);
		finalX = std::max(finalX, level.finalX);
		finalY = std::max(finalY, level.finalY);
	}
public:
	int startX;
	int startY;
	int finalX; 
	int finalY;
};
class gdalToTMS_metadata :public io_class
{
public:
	gdalToTMS_metadata() {	}
	void set(param_TMS p, callback cb);
	void add(const GDALTiler& tiler, const TileCoordinate* coordinate);
	void add(const gdalToTMS_metadata& otherMetadata);
	void writeJsonFile(entity_tms ti);
	void writeXmlFile(entity_tms ti);
public:
	std::vector<levelInfo> vec_levelInfo;
	CRSBounds bounds;
	CRSBounds validBounds;
	i_zoom maxZoom;
	i_zoom minZoom;
private:
	param_TMS m_param;
};


#endif
