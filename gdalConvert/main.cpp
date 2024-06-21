#include <unmanagedClass.h>
#include "gdalConvert_unmanaged.h"


int main(int argc, const char* argv[]) {
	//const char* source = "D:\\Download\\¶¤¶¤\\metadata.xml";
	//const char* target = "EPSG:4326";
	double sX = 494957, sY = 2446943, sZ = 0, tX = 0, tY = 0, tZ = 0;


	const char* source = "E:\\data\\pointCloud\\las\\metadata.xml";
	coordSystemConvert(source, "EPSG:4326", 0, 0, 0, tX, tY, tZ);

	double* arrarX = new double[5] {520000, 520001, 520002, 520003, 520004};
	double* arrarY = new double[5] {2500000, 2500001, 2500002, 2500003, 2500004};
	double* arrarZ = new double[5] {0, 0, 0, 0, 0};

	//coordSystemConvert_Array(source, target, 5, arrarX, arrarY, arrarZ);

	//coordSystemConvert_Array(target, source, 5, arrarX, arrarY, arrarZ);
	return true;
}