#ifndef _GDALTOWMTS_HELPER_H_
#define _GDALTOWMTS_HELPER_H_

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
#include <geo_plugins.h>
#include <io_log.h>
#include <io_thread.h>
#include <unmanagedClass_wmts.h>
#include <util_coordinate.h>


#include "gdalToWMTS_static.h"
#include "gdalToWMTS_unity.h"


class gdalToWMTS_helper :public io_basics
{
public:
	gdalToWMTS_helper();
	~gdalToWMTS_helper() {};
public:
	void initialize(U_WMTS u_WMTS);
	bool convert();
	void getTifFiles();
	void buildFiles();
	void printLog();
private:
	gdalToWMTS_unity wmtsUnity;
private:
	std::vector<wmtsInfo> vec_wmtsInfo;
};
#endif 