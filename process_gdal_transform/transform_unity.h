#ifndef _TRANSFORM_UNITY_H_
#define _TRANSFORM_UNITY_H_


#include <mutex>

#include <json/json.h>

#include <future>

#include <io_class.h>
#include <io_constant.h>
#include <io_entity.h>
#include <io_file.h>
#include <io_json.h>
#include <io_utily.h>
#include <util_algorithm.h>

#include <gdal_priv.h>  
#include <ogrsf_frmts.h>
#include <ogr_api.h>
#include <ogr_core.h>
#include <ogr_feature.h>  
#include <ogr_geometry.h>  
#include <ogr_spatialref.h>  
#include <ogr_srs_api.h>  

class transform_unity :public io_class
{
public:
	transform_unity() {};
	~transform_unity() {};

public:
	void set(param_Transform p, callback cb);
	bool information(entity_transform et);
	bool transform(entity_transform et);	
private:
	bool information_SHP(entity_transform et);
	bool information_TIF(entity_transform et);


	bool transform_SHP(entity_transform et);
	bool transform_TIF(entity_transform et);


	void offsetGeometry(entity_transform et, OGRGeometry* ogrGeometry);
	void offset_wkbLineString(entity_transform et, OGRGeometry* ogrGeometry);
	void offset_wkbMultiPoint(entity_transform et, OGRGeometry* ogrGeometry);
	void offset_wkbMultiLineString(entity_transform et, OGRGeometry* ogrGeometry);
	void offset_wkbPoint(entity_transform et, OGRGeometry* ogrGeometry);
	void offset_wkbPolygon(entity_transform et, OGRGeometry* ogrGeometry);
	void offset_wkbPolygons(entity_transform et, OGRGeometry* ogrGeometry);

	std::string toString_FieldType(OGRFieldType fieldType);
	std::string toString_GeometryType(OGRwkbGeometryType geomType);
	std::string toString_RasterDataType(GDALDataType gdalDataType);

private:
	param_Transform m_param;
};

#endif
