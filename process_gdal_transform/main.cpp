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
	const char* source = "D:\\downloads\\metadata.xml";
	const char* target = "EPSG:4326";

	num n1(698000, 2540000, 0);


	std::vector<num> vec_num{ n1 };

	for (num& n : vec_num)
	{
		transform_coordSystem(source, target, n.sx, n.sy, n.sz, n.tx, n.ty, n.tz);
		transform_coordSystem(target, source, n.tx, n.ty, n.tz, n.ex, n.ey, n.ez);
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