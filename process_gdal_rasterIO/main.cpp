#include "rasterIO_unmanaged.h"//main.cpp里必须引用接口头文件，否则接口不会对外开放 

int main(int argc, const char* argv[]) {
	const char* filePath = "E:\\tifTest\\FXYDEM84.tif";
	int rasterXSize = 0;
	int rasterYSize = 0;
	double geoTransform[6];
	int rasterCount = 0;
	int dataType = 0;
	char projection[4096];
	int hasNoDataValue = 0;
	double noDataValue = 0;
	getRasterInfo(filePath, rasterXSize, rasterYSize, geoTransform, rasterCount, dataType, hasNoDataValue, noDataValue, projection, 4096);
	unsigned char* data = new unsigned char[rasterXSize * rasterYSize * 4];
	readRasterData(filePath, data);
	return true;
}