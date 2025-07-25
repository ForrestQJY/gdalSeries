#include "vectorIO_unmanaged.h"//main.cpp里必须引用接口头文件，否则接口不会对外开放 

int main(int argc, const char* argv[]) {
	geo_gdal gdal;
	gdal.gdalRegister();

	//const char* filePath = "E:\\数据\\测试数据\\gdb\\pointZ.gdb";
	const char* filePath = "E:\\数据\\测试数据\\gdb\\越秀区建筑矢量.gdb";
	GUIntBig byteCount = 0;
	getVectorByteArrSize(filePath, byteCount, false, 0);
	unsigned char* byteArr = new unsigned char[byteCount];
	getVectorByteArr(filePath, byteArr, false, 0);
    // 从byteArr中读取4个double值
    double values[4];
    memcpy(values, byteArr, sizeof(double) * 4);

    // 打印读取的值
    for(int i = 0; i < 4; i++) {
        std::cout << "value[" << i << "]: " << values[i] << std::endl;
    }

	delete[] byteArr;
	byteArr = nullptr;
	return true;
}