#include <io_constant.h>
#include "gdalToTMS_unmanaged.h"

#include <string>
int main(int argc, const char* argv[]) {
	U_TMS u_param;
	u_param.f_Basic.AdaptiveFrontEnd = 1;
	u_param.f_Basic.Input = "E:\\data\\tif\\µØÐÎ\\dem\\dem.tif";
	u_param.f_Basic.Output = "E:\\out\\tif";
	u_param.f_Basic.RunnableThread = 1;
	u_param.f_Basic.OverlayFile = 0;


	u_param.f_TMS.TMSFormat = "mesh"; /*"terrain mesh png jpg tiff*/
	u_param.f_TMS.Profile = "geodetic";/*mercator geodetic*/
	u_param.f_TMS.Gzip = 0;
	u_param.f_TMS.WriteVertexNormals = 0;
	u_param.f_TMS.MaxZoom = -1;
	u_param.f_TMS.MinZoom = -1;

	tifToTMS(&u_param);

	return true;
}