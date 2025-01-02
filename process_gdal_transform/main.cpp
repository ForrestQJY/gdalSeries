#include "transform_unmanaged.h"


int main(int argc, const char* argv[]) {
	const char* source = "PROJCS[\"NJ08_118_50_CM\",GEOGCS[\"GCS_GRS_1980\",DATUM[\"D_GRS_1980\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Gauss_Kruger\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",118.8333333333333],PARAMETER[\"Scale_Factor\",1.0],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]],VERTCS[\"EGM96_Geoid\",VDATUM[\"EGM96_Geoid\"],PARAMETER[\"Vertical_Shift\",0.0],PARAMETER[\"Direction\",1.0],UNIT[\"Meter\",1.0],AUTHORITY[\"EPSG\",5773]]";
	const char* target = "EPSG:4326";
	double sX = 497144.56765650876, sY = 3540698.2920209286, sZ = 11.580780273028754, tX = 0, tY = 0, tZ = 0;

	transform_coordSystem(source, target, sX, sY, sZ, tX, tY, tZ);

	double* arrarX = new double[5] 
		{
			459141.000000,
			458791.090000,
			474814.57,
			473229.8563
		};
	double* arrarY = new double[5] 
		{
			0, 
			3846636.680000, 
			3849566.63,
			3843016.885			
		};
	double* arrarZ = new double[5] {114.000000, 172.000000, 100, 107};

	transform_coordSystem_Array(source, target, 4, arrarX, arrarY, arrarZ);

	transform_coordSystem_Array(target, source, 4, arrarX, arrarY, arrarZ);
	return true;
}