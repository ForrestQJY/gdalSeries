#ifndef _GDALCONVERT_UNMANAGED_H_
#define _GDALCONVERT_UNMANAGED_H_



#include <io_constant.h>
#include <geo_gdal.h>
#include <util_spatial.h>

#include <gdal_priv.h>  
#include <ogrsf_frmts.h>
#include <ogr_api.h>
#include <ogr_core.h>
#include <ogr_feature.h>  
#include <ogr_geometry.h>  
#include <ogr_spatialref.h>  
#include <ogr_srs_api.h>  

#pragma region coordinateConvert

UMANAGEAPI coordSystemConvert(const char* sourceCoord, const char* targetCoord, double sX, double sY, double sZ, double& tX, double& tY, double& tZ)
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
	return true;
}

UMANAGEAPI coordSystemConvert_Array(const char* sourceCoord, const char* targetCoord, int size, double* x, double* y, double* z)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial(sourceCoord, targetCoord);
	spatial.toLngLatAlt(size, x, y, z);
	spatial.uninstall();
	return true;
}


#pragma endregion



#endif


