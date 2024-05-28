
#include <string>
#include "gdalToWMTS_unmanaged.h"
int main(int argc, const char* argv[]) {

	U_WMTS u_WMTS;
	//u_WMTS.f_Basic.Input = "E:\\data\\tif\\地形\\dem\\dem.tif";
	u_WMTS.f_Basic.Input = "E:\\data\\tif\\影像\\DOM影像\\dom.tif";
	std::string output = "E:\\out\\tif\\影像" /*+ io_constant::getGuid8()*/;
	u_WMTS.f_Basic.Output = output.c_str();
	u_WMTS.f_Basic.RunnableThread = 8;//开启8线程
	u_WMTS.f_Basic.OverlayFile = 0;

	u_WMTS.f_WMTSConfig.MaxZoom = 14;
	u_WMTS.f_WMTSConfig.MinZoom = 10;
	u_WMTS.f_WMTSConfig.ResampleAlg = 3;
	u_WMTS.f_WMTSConfig.TileSize = 256;
	tifToWMTS(&u_WMTS);

	return true;
}