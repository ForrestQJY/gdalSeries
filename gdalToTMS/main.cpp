#include <unmanagedClass_tms.h>
#include <io_constant.h>
#include "gdalToTMS_unmanaged.h"

#include <string>
int main(int argc, const char* argv[]) {
	U_TMS u_TMS;
	//u_TMS.f_Basic.Input = "E:\\data\\tif\\地形\\dem\\dem.tif";
	u_TMS.f_Basic.Input = "E:\\data\\tif\\影像\\DOM影像\\dom.tif";
	std::string output = "E:\\out\\tif\\影像" /*+ io_constant::getGuid8()*/;
	u_TMS.f_Basic.Output = output.c_str();
	u_TMS.f_Basic.RunnableThread = 8;//开启8线程
	u_TMS.f_Basic.OverlayFile = 0;

	u_TMS.f_TMSConfig.TMSFormat = "png"; /*"terrain mesh png jpg tiff*/
	u_TMS.f_TMSConfig.Profile = "mercator";/*mercator geodetic*/
	u_TMS.f_TMSConfig.Gzip = 0;
	u_TMS.f_TMSConfig.WriteVertexNormals = 0;
	u_TMS.f_TMSConfig.SpecifiedHeight = 0;
	u_TMS.f_TMSConfig.HeightValue = 0;
	u_TMS.f_TMSConfig.StartZoom = -1;
	u_TMS.f_TMSConfig.EndZoom = -1;
	tifToTMS(&u_TMS);

	return true;
}