#include "gdalConvert_unmanaged.h"


void __stdcall coordSystemConvert(const char* sourceCoord, const char* targetCoord, double sX, double sY, double sZ, double& tX, double& tY, double& tZ)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial(sourceCoord, targetCoord);

	Eigen::Vector3d source;
	Eigen::Vector3d target;
	if (sX == 0 && sY == 0 && sZ == 0) {
		target = spatial.sCS.cLngLatAlt.xyz;
	}
	else {
		source = Eigen::Vector3d(sX, sY, sZ);
		spatial.toLngLatAlt(source, target);
	}
	tX = target.x();
	tY = target.y();
	tZ = target.z();
	spatial.uninstall();
}

void __stdcall coordSystemConvert_Array(const char* sourceCoord, const char* targetCoord, int size, double* x, double* y, double* z)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial(sourceCoord, targetCoord);
	spatial.toLngLatAlt(size, x, y, z);
	spatial.uninstall();
}

