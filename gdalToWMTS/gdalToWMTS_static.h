#ifndef _GDALTOWMTS_STATIC_H_
#define _GDALTOWMTS_STATIC_H_

#include <map>
#include <string>
#include <vector>

#include <io_constant.h>
#include <io_helper.h>
#include <io_param.h>
#include <unmanagedClass_wmts.h>

#define GDALTOWMTS_NAME		  ("gdalToWMTS")
#define GDALTOWMTS_LOG		  ((std::string)("gdalToWMTS_log.txt"))
#define GDALTOWMTS_ERRORLOG	  ((std::string)("gdalToWMTS_errorlog.txt"))
#define GDALTOWMTS_WARNINGLOG ((std::string)("gdalToWMTS_warninglog.txt"))

class pStatic :public io_param
{
public:
	static void setParameters(U_WMTS u_WMTS);
public:
	static U_WMTS u_Param;
};
#endif