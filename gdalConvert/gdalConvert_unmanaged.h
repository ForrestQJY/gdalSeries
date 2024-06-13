#ifndef _GDALCONVERT_UNMANAGED_H_
#define _GDALCONVERT_UNMANAGED_H_



#include <io_constant.h>
#include <geo_plugins.h>
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

extern "C" UMANAGEAPI void __stdcall getLngLatAlt_ByXML(const char* input, double& lng, double& lat, double& alt);
extern "C" UMANAGEAPI void __stdcall getLngLatAlt_ByEPSG(int epsg, double x, double y, double z, double& lng, double& lat, double& alt);
extern "C" UMANAGEAPI void __stdcall getLngLatAlt_ByPROJ(const char* proj, double x, double y, double z, double& lng, double& lat, double& alt);

extern "C" UMANAGEAPI void __stdcall getLngLatAltArray_ByEPSG(int epsg, int size, double* x, double* y, double* z);
extern "C" UMANAGEAPI void __stdcall getLngLatAltArray_ByPROJ(const char* proj, int size, double* x, double* y, double* z);

extern "C" UMANAGEAPI void __stdcall coordSystemConvert(const char* sourceCoord, const char* targetCoord, double sX, double sY, double sZ, double& tX, double& tY, double& tZ);
extern "C" UMANAGEAPI void __stdcall coordSystemConvert_Array(const char* sourceCoord, const char* targetCoord, int size, double* x, double* y, double* z);

//extern "C" UMANAGEAPI void __stdcall fileCoordinateSystemConvert(const char* inputFile, const char* outputFile, const char* targetCoordinate);

#pragma endregion



#endif


