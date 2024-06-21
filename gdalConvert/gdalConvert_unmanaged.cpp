#include "gdalConvert_unmanaged.h"

void __stdcall getLngLatAlt_ByXML(const char* input, double& lng, double& lat, double& alt)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial(input);
	Eigen::Vector3d target;
	spatial.toLngLatAlt(spatial.sCS.cEPSG.xyz, target);
	lng = target.x();
	lat = target.y();
	alt = target.z();
	spatial.uninstall();
}
void __stdcall getLngLatAlt_ByEPSG(int epsg, double x, double y, double z, double& lng, double& lat, double& alt)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial("EPSG:" + io_utily::toString(epsg));
	Eigen::Vector3d source(x, y, z);
	Eigen::Vector3d target;
	spatial.toLngLatAlt(source, target);
	lng = target.x();
	lat = target.y();
	alt = target.z();
	spatial.uninstall();
}

void __stdcall getLngLatAlt_ByPROJ(const char* proj, double x, double y, double z, double& lng, double& lat, double& alt)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial(proj);
	Eigen::Vector3d source(x, y, z);
	Eigen::Vector3d target;
	spatial.toLngLatAlt(source, target);
	lng = target.x();
	lat = target.y();
	alt = target.z();
	spatial.uninstall();
}

void __stdcall getLngLatAltArray_ByEPSG(int epsg, int size, double* x, double* y, double* z)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial("EPSG:" + io_utily::toString(epsg));
	spatial.toLngLatAlt(size, x, y, z);
	spatial.uninstall();
}

void __stdcall getLngLatAltArray_ByPROJ(const char* proj, int size, double* x, double* y, double* z)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial(proj);
	spatial.toLngLatAlt(size, x, y, z);
	spatial.uninstall();
}

void __stdcall coordSystemConvert(const char* sourceCoord, const char* targetCoord, double sX, double sY, double sZ, double& tX, double& tY, double& tZ)
{
	geo_gdal gdal;
	gdal.gdalRegister();
	util_spatial spatial(sourceCoord, targetCoord);

	Eigen::Vector3d source;
	Eigen::Vector3d target;
	if (sX == 0 || sY == 0 || sZ == 0) {
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
//
//void __stdcall fileCoordinateSystemConvert(const char* inputFile, const char* outputFile, const char* targetCoordinate)
//{
//	geo_plugins::gdalRegister();
//	GDALDataset* poDataset = (GDALDataset*)GDALOpen(inputFile, GA_ReadOnly);
//	//poDataset->GetDriver()->GetDescription();驱动名称
//	//poDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME); 驱动名称
//	//poDataset->GetRasterXSize(); poDataset->GetRasterYSize();poDataset->GetRasterCount();文件的宽高
//	//poDataset->GetProjectionRef();proj信息
//	//double adfGeoTransform[6];
//	//poDataset->GetGeoTransform(adfGeoTransform);包围盒
//	//adfGeoTransform[0]; adfGeoTransform[3];经纬度左上角
//	//adfGeoTransform[1]; adfGeoTransform[5];像素点尺寸
//	//GDALRasterBand* poBand = poDataset->GetRasterBand(1);
//	//int nBlockXSize;
//	//int nBlockYSize;
//	//poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);
//
//	if (poDataset == NULL) {
//		return;
//	}
//	const OGRSpatialReference* sOGRSpatialReference = poDataset->GetSpatialRef();
//	int epsgCode = sOGRSpatialReference->GetEPSGGeogCS();
//	util_spatial spatial(io_utily::toString(epsgCode), targetCoordinate);
//
//	OGRLayer* poLayer = poDataset->GetLayer(0); // 获取第一个图层  
//	if (poLayer == NULL) {
//		GDALClose(poDataset);
//		return;
//	}
//	poLayer->ResetReading();
//	OGRFeature* poFeature;
//	while ((poFeature = poLayer->GetNextFeature()) != NULL) {
//		OGRGeometry* poGeometry = poFeature->GetGeometryRef();
//		poGeometry->transformTo(&spatial.tOGRSpatialReference);
//		OGRFeature::DestroyFeature(poFeature); // 释放特征  
//	}
//	spatial.uninstall();
//	GDALClose(poDataset);
//
//	return;
//}

