#ifndef _RASTERIO_UNMANAGED_H_
#define _RASTERIO_UNMANAGED_H_

#include "rasterIO_launch.h"

UMANAGEAPI getRasterInfo(const char* filePath, int& rasterXSize, int& rasterYSize, double* geoTransform, int& rasterCount, int& dataType, int& hasNoDataVal, double& noDataValue, char* projection, int projectionMaxLength)
{
    rasterIO_launch launch;
    return launch.getRaterInfo(filePath, rasterXSize, rasterYSize, geoTransform, rasterCount, dataType, hasNoDataVal, noDataValue, projection, projectionMaxLength);
}

UMANAGEAPI readRasterData(const char* filePath, unsigned char* byteArr)
{
    rasterIO_launch launch;
    return launch.read(filePath, byteArr);
}

UMANAGEAPI writeRaster(const char* filePath, int rasterXSize, int rasterYSize, double* geoTransform, int rasterCount, int dataType, int hasNoDataVal, double noDataValue, const char* projection, unsigned char* byteArr) {
    rasterIO_launch launch;
    return launch.write(filePath, rasterXSize, rasterYSize, geoTransform, rasterCount, dataType, hasNoDataVal, noDataValue, projection, byteArr);
}

#endif