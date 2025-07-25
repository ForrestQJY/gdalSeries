#include "rasterIO_launch.h"
#include <gdal_priv.h>

rasterIO_launch::rasterIO_launch()
{
    carryStart();
    m_gdal.gdalRegister();
}

rasterIO_launch::~rasterIO_launch()
{
	carryEnd();
}

bool rasterIO_launch::getRaterInfo(const char* filePath, int& rasterXSize, int& rasterYSize, double* geoTransform, int& bandCount, int& dataType, int& hasNoDataVal, double& noDataValue, char* projection, int projectionMaxLength)
{
	std::string filePathUTF8 = io_file::stringToUTF8(filePath);
	GDALDataset* dataset = (GDALDataset*)GDALOpen(filePathUTF8.c_str(), GA_ReadOnly);
	if (dataset == NULL) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "打开文件失败");
		return false;
	}

	rasterXSize = dataset->GetRasterXSize();
	rasterYSize = dataset->GetRasterYSize();
	bandCount = dataset->GetRasterCount();
	dataset->GetGeoTransform(geoTransform);

	if (bandCount < 1) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "通道数为0，读取失败");
		GDALClose(dataset);
		return false;
	}

	dataType = dataset->GetRasterBand(1)->GetRasterDataType();
	noDataValue = dataset->GetRasterBand(1)->GetNoDataValue(&hasNoDataVal);

	std::string projectionStr = dataset->GetProjectionRef();
	strcpy(projection, projectionStr.c_str());
	if (projectionStr.size() > projectionMaxLength) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "投影信息字符串过长，请调整接收字符串的最大长度");
		GDALClose(dataset);
		return false;
	}

	return true;
}

bool rasterIO_launch::read(const char* filePath, unsigned char* byteArr)
{
	std::string filePathUTF8 = io_file::stringToUTF8(filePath);
	GDALDataset* dataset = (GDALDataset*)GDALOpen(filePathUTF8.c_str(), GA_ReadOnly);
	if (dataset == NULL) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "打开文件失败");
		return false;
	}
	const OGRSpatialReference* srs_input = dataset->GetSpatialRef();
	if (srs_input == NULL) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "获取空间参考失败");
		GDALClose(dataset);
		return false;
	}

	int nXSize = dataset->GetRasterXSize();
	int nYSize = dataset->GetRasterYSize();
	int nBandCount = dataset->GetRasterCount();

	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (poDriver == NULL) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "找不到GTiff驱动");
		GDALClose(dataset);
		return false;
	}
	
	int unitLength = 4;//TODO：根据数据类型获取
	dataset->RasterIO(GDALRWFlag::GF_Read, 0, 0, nXSize, nYSize, byteArr, nXSize, nYSize, GDT_Float32, nBandCount, NULL, 0, 0, 0);

	GDALClose(dataset);
    return true;
}

bool rasterIO_launch::write(const char* filePath, int rasterXSize, int rasterYSize, double* geoTransform, int rasterCount, int dataType, int hasNoDataVal, double noDataValue, const char* projection, unsigned char* byteArr)
{
	std::string filePathUTF8 = io_file::stringToUTF8(filePath);
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (poDriver == NULL) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "找不到GTiff驱动");
		return false;
	}
	GDALDataset* dataset = poDriver->Create(filePathUTF8.c_str(), rasterXSize, rasterYSize, rasterCount, (GDALDataType)dataType, NULL);
	if (dataset == NULL) {
		m_callback.sendError(io_utily::appendBracket(io_file::getFileNameWithoutExtension(filePathUTF8)) + "创建数据集失败");
		return false;
	}

	dataset->SetGeoTransform(geoTransform);
	dataset->SetProjection(projection);
	if (hasNoDataVal > 0) {
		for (int i = 1; i <= rasterCount; ++i) {
			dataset->GetRasterBand(i)->SetNoDataValue(noDataValue);
		}
	}

	// 写入数据
	dataset->RasterIO(GF_Write, 0, 0, rasterXSize, rasterYSize, byteArr, rasterXSize, rasterYSize, (GDALDataType)dataType, rasterCount, NULL, 0, 0, 0);

	GDALClose(dataset);
	return true;
}