
#ifndef _TRANSFORM_LAUNCH_H_
#define _TRANSFORM_LAUNCH_H_

#include <gdal_priv.h>


#include <io_class.h>
#include <io_composition.h>
#include <io_entity.h>
#include <io_file.h>
#include <io_thread.h>
#include <io_transfer.h>
#include <io_utily.h>
#include <geo_gdal.h>
#include <util_spatial.h>
#include <util_coordinate.h>
#include <util_spatial.h>

class transform_launch :public io_class
{
public:
	transform_launch();
	~transform_launch() {};
public:
	void initialize(U_Transform u_param);
	bool toInformation();
	bool toTransform();
	void preFiles();
	void preInformation();
	void preTransform();
private:
	std::vector<entity_transform> vec_entityTransform;
private:
	geo_gdal m_gdal;
private:
	param_Transform m_param;
};

#endif 