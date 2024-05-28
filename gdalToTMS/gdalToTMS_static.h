#ifndef _GDALTOTMS_STATIC_H_
#define _GDALTOTMS_STATIC_H_

#include <map>
#include <string>
#include <vector>

#include <io_constant.h>
#include <io_helper.h>
#include <io_param.h>
#include <unmanagedClass_tms.h>

#define GDALTOTMS_NAME		 ("gdalToTMS")
#define GDALTOTMS_LOG		 ((std::string)("gdalToTMS_log.txt"))
#define GDALTOTMS_ERRORLOG	 ((std::string)("gdalToTMS_errorlog.txt"))
#define GDALTOTMS_WARNINGLOG ((std::string)("gdalToTMS_warninglog.txt"))

class pStatic :public io_param
{
public:
	static void setParameters(U_TMS u_TMS);
public:
	static U_TMS u_Param;
};
#endif