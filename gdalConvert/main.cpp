#include <unmanagedClass.h>
#include "gdalConvert_unmanaged.h"


int main(int argc, const char* argv[]) {


	const char* source = "D:\\Download\\¶¤¶¤\\metadata.xml";
	const char* target = "EPSG:4326";
	double sX = 494957, sY = 2446943, sZ = 0, tX = 0, tY = 0, tZ = 0;

	getLngLatAlt_ByPROJ(source, sX, sY, sZ, tX, tY, tZ);
	//coordinateSystemConvert(source, target, sX, sY, sZ, tX, tY, tZ);

	//sX = tX, sY = tY, sZ = tZ, tX = 0, tY = 0, tZ = 0;
	//coordinateSystemConvert(target, source, sX, sY, sZ, tX, tY, tZ);

	//double* arrarX = new double[5] {520000, 520001, 520002, 520003, 520004};
	//double* arrarY = new double[5] {2500000, 2500001, 2500002, 2500003, 2500004};
	//double* arrarZ = new double[5] {0, 0, 0, 0, 0};

	//coordinateSystemConvert_Array(source, target, 5, arrarX, arrarY, arrarZ);

	//coordinateSystemConvert_Array(target, source, 5, arrarX, arrarY, arrarZ);
	return true;
}