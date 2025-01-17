#include "transform_unmanaged.h"

class num {
public:
	num() {}
	num(double x, double y, double z) :sx(x), sy(y), sz(z) {}
public:
	double sx = 0;
	double sy = 0;
	double sz = 0;
	double tx = 0;
	double ty = 0;
	double tz = 0;
	double ex = 0;
	double ey = 0;
	double ez = 0;
};

int main(int argc, const char* argv[]) {
	const char* source = "EPSG:4978";
	const char* target = "EPSG:4326";

	//num n1(-2348712.20201754, 5404849.321179945, 2431891.432501744);
	//num n2(-2295257.86964383, 5407288.682376011, 2476785.363652067);
	num n3(113.48766697316626, 22.56031199279221, 169.32368773035705);
	num n4(112.9999999598919, 22.999995766160485, 169.17392709944397);


	std::vector<num> vec_num{ n3 ,n4 };

	for (num& n : vec_num)
	{
		transform_coordSystem(target, source, n.sx, n.sy, n.sz, n.tx, n.ty, n.tz);
		transform_coordSystem(source, target, n.tx, n.ty, n.tz, n.ex, n.ey, n.ez);
		int a = 0;
	}

	//double* arrarX = new double[2]
	//	{
	//		-2348712.20201754,
	//		-2295257.8696438335,
	//	};
	//double* arrarY = new double[2]
	//	{
	//		5404849.321179945,
	//		5407288.682376011
	//	};
	//double* arrarZ = new double[2] 
	//	{
	//		2431891.432501744,
	//		2476785.363652067
	//	};

	//transform_coordSystem_Array(source, target, 2, arrarX, arrarY, arrarZ);
	//											
	//transform_coordSystem_Array(target, source, 2, arrarX, arrarY, arrarZ);
	//U_Transform u_param;
	//u_param.f_Basic.Input = "E:\\t";
	//u_param.f_Basic.Output = "E:\\t\\o";
	//u_param.f_Transform.TargetSpatial = "EPSG:4547";
	//u_param.f_Info.ProvideError = 1;
	//u_param.f_Info.ProvideMessage = 1;
	//u_param.f_Info.ProvideWarning = 1;

	//transform_fileInformation(&u_param);
	return true;
}