#include <io_constant.h>
#include "gdalToTMS_unmanaged.h"

#include <string>
int main(int argc, const char* argv[]) {
	U_TMS u_param;
	u_param.f_Basic.AdaptiveFrontEnd = 1;
	u_param.f_Basic.Input = "E:\\data\\tif\\Ó°Ïñ\\ÑÕÉ«´íÂÒ\\shejizhidu12231_jz4490.tif";
	u_param.f_Basic.Output = "E:\\out\\3";
	u_param.f_Basic.RunnableThread = 16;
	u_param.f_Basic.OverlayFile = 0;

	u_param.f_TMS.TMSFormat = "png"; /*"terrain mesh png jpg tiff*/
	u_param.f_TMS.GeographicProjectionFormat = "mercator";/*mercator geodetic*/
	u_param.f_TMS.Gzip = 0;
	u_param.f_TMS.MaxZoom = 0;
	u_param.f_TMS.MinZoom = 0;
	u_param.f_TMS_Quality.CesiumFriendly = 1;
	u_param.f_TMS_Quality.CesiumMetadata = 0;
	u_param.f_TMS_Quality.MeshQualityFactor = 1.0;	
	u_param.f_TMS_Quality.WriteVertexNormals = 0;

	u_param.f_Info.ProvideError = 1;
	u_param.f_Info.ProvideMessage = 1;
	u_param.f_Info.ProvideWarning = 1;
	tifToTMS_terrain(&u_param);

	return true;
}