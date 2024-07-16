#include <unmanagedClass.h>
#include "gdalConvert_unmanaged.h"


int main(int argc, const char* argv[]) {
	//const char* source = "D:\\Download\\¶¤¶¤\\metadata.xml";
	//const char* target = "EPSG:4326";
	double sX = 38448528, sY = 2447027, sZ = 0, tX = 0, tY = 0, tZ = 0;


	const char* source = "EPSG:4526";
	coordSystemConvert(source, "EPSG:4326", sX, sY, sZ, tX, tY, tZ);

	double* arrarX = new double[5] {520000, 520001, 520002, 520003, 520004};
	double* arrarY = new double[5] {2500000, 2500001, 2500002, 2500003, 2500004};
	double* arrarZ = new double[5] {0, 0, 0, 0, 0};

	//coordSystemConvert_Array(source, target, 5, arrarX, arrarY, arrarZ);

	//coordSystemConvert_Array(target, source, 5, arrarX, arrarY, arrarZ);
	return true;
}