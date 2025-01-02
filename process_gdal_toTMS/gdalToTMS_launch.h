#ifndef _GDALTOTMS_LAUNCH_H_
#define _GDALTOTMS_LAUNCH_H_

#ifdef _HAS_STD_BYTE
#undef _HAS_STD_BYTE
#endif
#define _HAS_STD_BYTE 0

#include <io.h>
#include <stdio.h>
#include <array>
#include <thread>
#include <vector>
#include <iostream>
#include <functional>
#include <future>
#include <string>
#include <fstream>

#include <gdal_priv.h>


#include <io_class.h>
#include <io_composition.h>
#include <io_entity.h>
#include <io_file.h>
#include <io_thread.h>
#include <io_transfer.h>
#include <io_utily.h>
#include <geo_gdal.h>
#include <util_spatial.h>
#include <util_coordinate.h>
#include <util_spatial.h>


#include <GlobalGeodetic.hpp>
#include <GlobalMercator.hpp>
#include <Grid.hpp>

#include "gdalToTMS_unity.h"

using namespace gb;


class gdalToTMS_launch :public io_class
{
public:
	gdalToTMS_launch();
	~gdalToTMS_launch() {};
public:
	void initialize(U_TMS u_param);
	bool toImagery();
	bool toTerrain();
	void getTifFiles();
	void buildFiles();
	void getGrid(Grid& grid);
private:
	std::vector<entity_tms> vec_entityTMS;
private:
	geo_gdal m_gdal;
	util_spatial m_spatial;
private:
	param_TMS m_param;
};
#endif 