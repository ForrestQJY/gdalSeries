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


#include <io_basics.h>
#include <io_composition.h>
#include <io_file.h>
#include <io_log.h>
#include <io_thread.h>
#include <geo_plugins.h>
#include <unmanagedClass_tms.h>
#include <util_coordinate.h>

#include <GlobalGeodetic.hpp>
#include <GlobalMercator.hpp>
#include <Grid.hpp>

#include "gdalToTMS_static.h"
#include "gdalToTMS_unity.h"

using namespace gb;


class gdalToTMS_helper :public io_basics
{
public:
	gdalToTMS_helper();
	~gdalToTMS_helper() {};
public:
	void initialize(U_TMS u_TMS);
	bool convert();
	void getTifFiles();
	void buildFiles();
	void printLog();
	void getGrid(Grid& grid);
private:
	gdalToTMS_unity tmsUnity;
private:
	std::vector<tmsInfo> vec_tmsInfo;
};
#endif 