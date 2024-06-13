#ifndef _GDALTOTMS_HELPER_H_
#define _GDALTOTMS_HELPER_H_

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
#include <io_file.h>
#include <io_utily.h>
#include <io_thread.h>
#include <geo_plugins.h>
#include <unmanagedClass_tms.h>
#include <util_coordinate.h>
#include <util_entity.h>

#include <GlobalGeodetic.hpp>
#include <GlobalMercator.hpp>
#include <Grid.hpp>

#include "gdalToTMS_unity.h"

using namespace gb;


class gdalToTMS_helper :public io_class
{
public:
	gdalToTMS_helper();
	~gdalToTMS_helper() {};
public:
	void initialize(U_TMS u_param);
	bool convert();
	void getTifFiles();
	void buildFiles();
	void getGrid(Grid& grid);
private:
	std::vector<entity_tms> vec_entityTMS;
private:
	U_TMS u_Param;
};
#endif 