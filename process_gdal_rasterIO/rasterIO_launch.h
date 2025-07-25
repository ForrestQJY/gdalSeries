#ifndef _RASTERIO_LAUNCH_H_
#define _RASTERIO_LAUNCH_H_

#include <io_class.h>
#include <geo_gdal.h>

class rasterIO_launch :public io_class
{
public:
	rasterIO_launch();
	~rasterIO_launch();
public:
	bool getRaterInfo(const char* filePath, int& rasterXSize, int& rasterYSize, double* geoTransform, int& bandCount, int& dataType, int& hasNoDataVal, double& noDataValue, char* projection, int projectionMaxLength);
	bool read(const char* filePath, unsigned char* byteArr);
	bool write(const char* filePath, int rasterXSize, int rasterYSize, double* geoTransform, int rasterCount, int dataType, int hasNoDataVal, double noDataValue, const char* projection, unsigned char* byteArr);
private:
	geo_gdal m_gdal;
};

#endif