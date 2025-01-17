#include "transform_unity.h"
#include <geo_gdal.h>
#include <util_spatial.h>

void transform_unity::set(param_Transform p, callback cb)
{
	m_param = p;
	m_callback = cb;
}

bool transform_unity::information(entity_transform et)
{
	if (et.i.file_extension == format_shp) {
		return information_SHP(et);
	}
	else if (et.i.file_extension == format_tif) {
		return information_TIF(et);
	}
	return false;
}

bool transform_unity::transform(entity_transform et)
{
	if (et.i.file_extension == format_shp) {
		return transform_SHP(et);
	}
	else if (et.i.file_extension == format_tif) {
		return transform_TIF(et);
	}
	return false;
}

bool transform_unity::information_SHP(entity_transform et)
{
	GDALDataset* dataset = (GDALDataset*)GDALOpenEx(et.i.path_file_utf8.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (dataset == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "打开文件失败");
		return 1;
	}


	Json::Value json;
	json["file"]["file_extension"] = et.i.file_extension;
	json["file"]["file_name_within_extension"] = et.i.file_name_within_extension;
	json["file"]["file_name_without_extension"] = et.i.file_name_without_extension;
	json["file"]["folder_name"] = et.i.folder_name;
	json["file"]["path_file"] = et.i.path_file;
	json["file"]["path_folder"] = et.i.path_folder;
	json["gdal"]["driver"] = dataset->GetDriver()->GetDescription();


	int layerCount = dataset->GetLayerCount();
	json["layerCount"] = layerCount;


	for (int i = 0; i < layerCount; ++i) {
		Json::Value json_layer;
		OGRLayer* layer = dataset->GetLayer(i);
		json_layer["name"] = io_file::utf8ToString(layer->GetName());


		OGRwkbGeometryType layerGeomType = layer->GetGeomType();
		json_layer["geometryType"] = toString_GeometryType(layerGeomType);


		OGRSpatialReference* spatialRef = layer->GetSpatialRef();
		if (spatialRef != NULL) {
			char* srsWkt = NULL;
			spatialRef->exportToWkt(&srsWkt);
			json_layer["spatialReference"] = srsWkt;
			CPLFree(srsWkt);
		}


		int featureCount = layer->GetFeatureCount();
		json_layer["featureCount"] = featureCount;


		OGREnvelope envelope;
		if (layer->GetExtent(&envelope) == OGRERR_NONE) {
			json_layer["extent"]["minX"] = envelope.MinX;
			json_layer["extent"]["minY"] = envelope.MinY;
			json_layer["extent"]["maxX"] = envelope.MaxX;
			json_layer["extent"]["maxY"] = envelope.MaxY;
		}


		OGRFeatureDefn* featureDefn = layer->GetLayerDefn();
		int fieldCount = featureDefn->GetFieldCount();
		Json::Value json_fields;
		for (int j = 0; j < fieldCount; ++j) {
			Json::Value json_feature;
			OGRFieldDefn* fieldDefn = featureDefn->GetFieldDefn(j);
			json_feature["featureName"] = fieldDefn->GetNameRef();
			json_feature["featureType"] = toString_FieldType(fieldDefn->GetType());
			json_fields.append(json_feature);
		}
		json_layer["fieldCount"] = fieldCount;
		json_layer["fields"] = json_fields;


		Json::Value json_features;
		OGRFeature* feature;
		for (int k = 0; k < featureCount; ++k) {
			feature = layer->GetNextFeature();
			if (feature == NULL) {
				break;
			}
			Json::Value json_feature;
			OGRGeometry* geometry = feature->GetGeometryRef();
			if (geometry != NULL) {
				std::string geometryName = geometry->getGeometryName();
				json_feature["geometry"]["name"] = geometryName;
				OGRwkbGeometryType featureGeomType = geometry->getGeometryType();
				json_feature["geometry"]["geometryType"] = toString_GeometryType(featureGeomType);
				if (wkbFlatten(featureGeomType) == wkbPoint) {
					OGRPoint* point = (OGRPoint*)geometry;
					json_feature["geometry"]["coordinates"]["x"] = point->getX();
					json_feature["geometry"]["coordinates"]["y"] = point->getY();
				}
				else if (wkbFlatten(featureGeomType) == wkbMultiPoint) {
					OGRMultiPoint* multiPoint = (OGRMultiPoint*)geometry;
					for (int p = 0; p < multiPoint->getNumGeometries(); ++p) {
						OGRGeometry* subGeometry = multiPoint->getGeometryRef(p);
						if (wkbFlatten(subGeometry->getGeometryType()) == wkbPoint) {
							OGRPoint* subPoint = (OGRPoint*)subGeometry;
							Json::Value coord;
							coord["x"] = subPoint->getX();
							coord["y"] = subPoint->getY();
							json_feature["geometry"]["coordinates"].append(coord);
						}
					}
				}
				else if (wkbFlatten(featureGeomType) == wkbLineString) {
					OGRLineString* lineString = (OGRLineString*)geometry;
					for (int p = 0; p < lineString->getNumPoints(); ++p) {
						Json::Value coord;
						coord["x"] = lineString->getX(p);
						coord["y"] = lineString->getY(p);
						json_feature["geometry"]["coordinates"].append(coord);
					}
				}
				else if (wkbFlatten(featureGeomType) == wkbMultiLineString) {
					OGRMultiLineString* multiLineString = (OGRMultiLineString*)geometry;
					for (int p = 0; p < multiLineString->getNumGeometries(); ++p) {
						OGRGeometry* subGeometry = multiLineString->getGeometryRef(p);
						if (wkbFlatten(subGeometry->getGeometryType()) == wkbLineString) {
							OGRLineString* subLineString = (OGRLineString*)subGeometry;
							for (int q = 0; q < subLineString->getNumPoints(); ++q) {
								Json::Value coord;
								coord["x"] = subLineString->getX(q);
								coord["y"] = subLineString->getY(q);
								json_feature["geometry"]["coordinates"].append(coord);
							}
						}
					}
				}
				else if (wkbFlatten(featureGeomType) == wkbPolygon) {
					OGRPolygon* polygon = (OGRPolygon*)geometry;
					OGRLinearRing* ring = polygon->getExteriorRing();
					Json::Value exteriorRing;
					for (int p = 0; p < ring->getNumPoints(); ++p) {
						Json::Value coord;
						coord["x"] = ring->getX(p);
						coord["y"] = ring->getY(p);
						json_feature["geometry"]["coordinates"].append(coord);
					}


					for (int r = 0; r < polygon->getNumInteriorRings(); ++r) {
						OGRLinearRing* interiorRing = polygon->getInteriorRing(r);
						Json::Value interior;
						for (int p = 0; p < interiorRing->getNumPoints(); ++p) {
							Json::Value coord;
							coord["x"] = interiorRing->getX(p);
							coord["y"] = interiorRing->getY(p);
							json_feature["geometry"]["coordinates"].append(coord);
						}
					}
				}
				else if (wkbFlatten(featureGeomType) == wkbMultiPolygon) {
					OGRMultiPolygon* multiPolygon = (OGRMultiPolygon*)geometry;
					for (int p = 0; p < multiPolygon->getNumGeometries(); ++p) {
						OGRGeometry* subGeometry = multiPolygon->getGeometryRef(p);
						if (wkbFlatten(subGeometry->getGeometryType()) == wkbPolygon) {
							OGRPolygon* subPolygon = (OGRPolygon*)subGeometry;
							OGRLinearRing* ring = subPolygon->getExteriorRing();
							Json::Value exteriorRing;
							for (int q = 0; q < ring->getNumPoints(); ++q) {
								Json::Value coord;
								coord["x"] = ring->getX(q);
								coord["y"] = ring->getY(q);

								json_feature["geometry"]["coordinates"].append(coord);
							}


							for (int r = 0; r < subPolygon->getNumInteriorRings(); ++r) {
								OGRLinearRing* interiorRing = subPolygon->getInteriorRing(r);
								Json::Value interior;
								for (int q = 0; q < interiorRing->getNumPoints(); ++q) {
									Json::Value coord;
									coord["x"] = interiorRing->getX(q);
									coord["y"] = interiorRing->getY(q);
									json_feature["geometry"]["coordinates"].append(coord);
								}
							}
						}
					}
				}
			}


			for (int l = 0; l < fieldCount; ++l) {
				OGRFieldDefn* fieldDefn = featureDefn->GetFieldDefn(l);
				std::string fieldValue = feature->GetFieldAsString(l);
				json_feature["attributes"][fieldDefn->GetNameRef()] = io_file::utf8ToString(fieldValue);
			}
			json_features.append(json_feature);
			OGRFeature::DestroyFeature(feature);
		}
		json_layer["features"] = json_features;
		json["layer"].append(json_layer);
	}


	GDALClose(dataset);
	io_json::writeJson(et.o.path_folder + symbol_dir + et.i.file_name_without_extension + symbol_ext + format_json, json);
	return true;
}

bool transform_unity::information_TIF(entity_transform et)
{
	GDALDataset* dataset = (GDALDataset*)GDALOpen(et.i.path_file_utf8.c_str(), GA_ReadOnly);
	if (dataset == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "打开文件失败");
		return false;
	}


	Json::Value json;


	json["file"]["file_extension"] = et.i.file_extension;
	json["file"]["file_name_within_extension"] = et.i.file_name_within_extension;
	json["file"]["file_name_without_extension"] = et.i.file_name_without_extension;
	json["file"]["folder_name"] = et.i.folder_name;
	json["file"]["path_file"] = et.i.path_file;
	json["file"]["path_folder"] = et.i.path_folder;
	json["gdal"]["driver"] = dataset->GetDriver()->GetDescription();
	json["raster"]["width"] = dataset->GetRasterXSize();
	json["raster"]["height"] = dataset->GetRasterYSize();
	json["raster"]["bands"] = dataset->GetRasterCount();


	GDALDataType dataType = dataset->GetRasterBand(1)->GetRasterDataType();
	json["raster"]["data_type"] = toString_RasterDataType(dataType);


	const OGRSpatialReference* spatialRef = dataset->GetSpatialRef();
	if (spatialRef != NULL) {
		char* srsWkt = NULL;
		spatialRef->exportToWkt(&srsWkt);
		json["raster"]["spatial_reference"] = srsWkt;
		CPLFree(srsWkt);
	}


	double transform[6];
	if (dataset->GetGeoTransform(transform) == CE_None) {
		json["raster"]["geotransform"][0] = transform[0];
		json["raster"]["geotransform"][1] = transform[1];
		json["raster"]["geotransform"][2] = transform[2];
		json["raster"]["geotransform"][3] = transform[3];
		json["raster"]["geotransform"][4] = transform[4];
		json["raster"]["geotransform"][5] = transform[5];
	}


	//int nXSize = dataset->GetRasterXSize();
	//int nYSize = dataset->GetRasterYSize();
	//int nBandCount = dataset->GetRasterCount();
	//float* pafScanline = (float*)CPLMalloc(sizeof(float) * nXSize * nBandCount);
	//Json::Value json_data;
	//for (int i = 1; i <= nBandCount; ++i) {
	//	GDALRasterBand* band = dataset->GetRasterBand(i);
	//	for (int y = 0; y < nYSize; ++y) {
	//		band->RasterIO(GF_Read, 0, y, nXSize, 1, pafScanline + (i - 1) * nXSize, nXSize, 1, GDT_Float32, 0, 0);
	//		for (int x = 0; x < nXSize; ++x) {
	//			json_data[y][x][i - 1] = pafScanline[x + (i - 1) * nXSize];
	//		}
	//	}
	//}
	//json["raster"]["data"] = json_data;


	//CPLFree(pafScanline);
	GDALClose(dataset);
	io_json::writeJson(et.o.path_folder + symbol_dir + et.i.file_name_without_extension + symbol_ext + format_json, json);
	return true;
}


bool transform_unity::transform_SHP(entity_transform et)
{
	util_spatial spatial("", et.targetSpatial);

	GDALDataset* dataset_input = (GDALDataset*)GDALOpenEx(et.i.path_file_utf8.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (dataset_input == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "打开文件失败");
		return false;
	}

	OGRLayer* layer_input = dataset_input->GetLayer(0);
	if (layer_input == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "获取图层Layer失败");
		GDALClose(dataset_input);
		return false;
	}

	OGRSpatialReference* srs_input = layer_input->GetSpatialRef();
	if (srs_input == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "获取空间参考失败");
		GDALClose(dataset_input);
		return false;
	}

	OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(srs_input, &spatial.tOGRSR);
	if (transform == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "转换空间参考失败");
		GDALClose(dataset_input);
		return false;
	}

	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	if (poDriver == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "找不到ESRI Shapefile驱动");
		GDALClose(dataset_input);
		return false;
	}
	GDALDataset* dataset_output = poDriver->Create(et.o.path_file_utf8.c_str(), 0, 0, 0, GDT_Unknown, NULL);
	if (dataset_output == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.o.file_name_within_extension) + "创建输出文件失败");
		GDALClose(dataset_input);
		return false;
	}

	OGRLayer* layer_output = dataset_output->CreateLayer(et.o.file_name_without_extension.c_str(), &spatial.tOGRSR, layer_input->GetGeomType(), NULL);
	if (layer_output == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.o.file_name_within_extension) + "创建图层Layer失败");
		GDALClose(dataset_input);
		GDALClose(dataset_output);
		return false;
	}


	OGRFeatureDefn* featureDefn_input = layer_input->GetLayerDefn();
	for (int i = 0; i < featureDefn_input->GetFieldCount(); ++i) {
		layer_output->CreateField(featureDefn_input->GetFieldDefn(i));
	}

	OGRFeatureDefn* featureDefn_output = layer_output->GetLayerDefn();
	OGRFeature* feature_input;
	while ((feature_input = layer_input->GetNextFeature()) != NULL) {
		OGRFeature* feature_output = new OGRFeature(featureDefn_output);
		OGRGeometry* geometry_input = feature_input->GetGeometryRef();
		if (geometry_input != NULL) {
			OGRGeometry* geometry_output = geometry_input->clone();
			offsetGeometry(et, geometry_output);
			if (geometry_output->transform(transform) == OGRERR_NONE) {
				feature_output->SetGeometry(geometry_output);
				for (int i = 0; i < featureDefn_output->GetFieldCount(); ++i) {
					feature_output->SetField(featureDefn_output->GetFieldDefn(i)->GetNameRef(), feature_input->GetFieldAsString(i));
				}
				if (layer_output->CreateFeature(feature_output) != OGRERR_NONE) {
					m_callback.sendError(io_utily::appendBracket(et.o.file_name_within_extension) + "创建图层特征失败");
				}
			}
			else {
				m_callback.sendError(io_utily::appendBracket(et.o.file_name_within_extension) + "几何转换坐标失败");
			}
		}
		else {
			m_callback.sendError(io_utily::appendBracket(et.o.file_name_within_extension) + "几何为空");
		}


		OGRFeature::DestroyFeature(feature_input);
		OGRFeature::DestroyFeature(feature_output);
	}


	GDALClose(dataset_input);
	GDALClose(dataset_output);
	OGRCoordinateTransformation::DestroyCT(transform);
	return true;
}
bool transform_unity::transform_TIF(entity_transform et)
{
	util_spatial spatial("", et.targetSpatial);


	GDALDataset* dataset_input = (GDALDataset*)GDALOpen(et.i.path_file_utf8.c_str(), GA_ReadOnly);
	if (dataset_input == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "打开文件失败");
		return false;
	}
	const OGRSpatialReference* srs_input = dataset_input->GetSpatialRef();
	if (srs_input == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "获取空间参考失败");
		GDALClose(dataset_input);
		return false;
	}
	OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(srs_input, &spatial.tOGRSR);
	if (transform == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "转换空间参考失败");
		GDALClose(dataset_input);
		return false;
	}

	int nXSize = dataset_input->GetRasterXSize();
	int nYSize = dataset_input->GetRasterYSize();
	int nBandCount = dataset_input->GetRasterCount();


	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (poDriver == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.i.file_name_within_extension) + "找不到GTiff驱动");
		GDALClose(dataset_input);
		return false;
	}

	GDALDataset* dataset_output = poDriver->Create(et.o.path_file_utf8.c_str(), nXSize, nYSize, nBandCount, GDT_Float32, NULL);
	if (dataset_output == NULL) {
		m_callback.sendError(io_utily::appendBracket(et.o.file_name_within_extension) + "创建输出文件失败");
		GDALClose(dataset_input);
		return false;
	}


	dataset_output->SetSpatialRef(&spatial.tOGRSR);

	for (int i = 1; i <= nBandCount; ++i) {
		GDALRasterBand* band_input = dataset_input->GetRasterBand(i);
		GDALRasterBand* band_output = dataset_output->GetRasterBand(i);
		float* pafScanline = (float*)CPLMalloc(sizeof(float) * nXSize);

		for (int y = 0; y < nYSize; ++y) {
			band_input->RasterIO(GF_Read, 0, y, nXSize, 1, pafScanline, nXSize, 1, GDT_Float32, 0, 0);
			band_output->RasterIO(GF_Write, 0, y, nXSize, 1, pafScanline, nXSize, 1, GDT_Float32, 0, 0);
		}

		CPLFree(pafScanline);
	}

	GDALClose(dataset_input);
	GDALClose(dataset_output);
	OGRCoordinateTransformation::DestroyCT(transform);

	return false;
}

void transform_unity::offsetGeometry(entity_transform et, OGRGeometry* ogrGeometry)
{
	if (et.offsetX == 0 && et.offsetY == 0 && et.offsetZ == 0)return;
	OGRwkbGeometryType geometryType = ogrGeometry->getGeometryType();
	switch (geometryType)
	{
	case OGRwkbGeometryType::wkbLineString:
		offset_wkbLineString(et, ogrGeometry);
		break;
	case OGRwkbGeometryType::wkbMultiLineString:
		offset_wkbMultiLineString(et, ogrGeometry);
		break;
	case OGRwkbGeometryType::wkbMultiPoint:
		offset_wkbMultiPoint(et, ogrGeometry);
		break;
	case OGRwkbGeometryType::wkbPoint:
		offset_wkbPoint(et, ogrGeometry);
		break;
	case OGRwkbGeometryType::wkbPolygon:
		offset_wkbPolygon(et, ogrGeometry);
		break;
	case OGRwkbGeometryType::wkbMultiPolygon:
		offset_wkbPolygons(et, ogrGeometry);
		break;
	default:
		break;
	}
}
void transform_unity::offset_wkbLineString(entity_transform et, OGRGeometry* ogrGeometry)
{
	OGRLineString* lineStringGeom = (OGRLineString*)ogrGeometry;
	for (int i = 0; i < lineStringGeom->getNumPoints(); ++i) {
		lineStringGeom->setPoint(i, lineStringGeom->getX(i) + et.offsetX, lineStringGeom->getY(i) + et.offsetY, lineStringGeom->getZ(i) + et.offsetZ);
	}
}
void transform_unity::offset_wkbMultiLineString(entity_transform et, OGRGeometry* ogrGeometry)
{
	OGRMultiLineString* multiLineStringGeom = (OGRMultiLineString*)ogrGeometry;
	for (int i = 0; i < multiLineStringGeom->getNumGeometries(); ++i) {
		OGRGeometry* subGeom = multiLineStringGeom->getGeometryRef(i);
		if (wkbFlatten(subGeom->getGeometryType()) == wkbLineString) {
			OGRLineString* subLineString = (OGRLineString*)subGeom;
			for (int j = 0; j < subLineString->getNumPoints(); ++j) {
				subLineString->setPoint(j, subLineString->getX(j) + et.offsetX, subLineString->getY(j) + et.offsetY, subLineString->getZ(j) + et.offsetZ);
			}
		}
	}
}
void transform_unity::offset_wkbMultiPoint(entity_transform et, OGRGeometry* ogrGeometry)
{
	OGRMultiPoint* multiPointGeom = (OGRMultiPoint*)ogrGeometry;
	for (int i = 0; i < multiPointGeom->getNumGeometries(); ++i) {
		OGRGeometry* subGeom = multiPointGeom->getGeometryRef(i);
		if (wkbFlatten(subGeom->getGeometryType()) == wkbPoint) {
			OGRPoint* subPoint = (OGRPoint*)subGeom;
			subPoint->setX(subPoint->getX() + et.offsetX);
			subPoint->setY(subPoint->getY() + et.offsetX);
			subPoint->setZ(subPoint->getZ() + et.offsetX);
		}
	}
}
void transform_unity::offset_wkbPoint(entity_transform et, OGRGeometry* ogrGeometry)
{
	OGRPoint* pointGeom = (OGRPoint*)ogrGeometry;
	pointGeom->setX(pointGeom->getX() + et.offsetX);
	pointGeom->setY(pointGeom->getY() + et.offsetX);
	pointGeom->setZ(pointGeom->getZ() + et.offsetX);
}
void transform_unity::offset_wkbPolygon(entity_transform et, OGRGeometry* ogrGeometry)
{
	OGRPolygon* polygonGeom = (OGRPolygon*)ogrGeometry;
	OGRLinearRing* ring = polygonGeom->getExteriorRing();
	for (int i = 0; i < ring->getNumPoints(); ++i) {
		ring->setPoint(i, ring->getX(i) + et.offsetX, ring->getY(i) + et.offsetY, ring->getZ(i) + et.offsetZ);
	}
	for (int i = 0; i < polygonGeom->getNumInteriorRings(); ++i) {
		OGRLinearRing* interiorRing = polygonGeom->getInteriorRing(i);
		for (int j = 0; j < interiorRing->getNumPoints(); ++j) {
			interiorRing->setPoint(j, interiorRing->getX(j) + et.offsetX, interiorRing->getY(j) + et.offsetY, interiorRing->getZ(j) + et.offsetZ);
		}
	}
}
void transform_unity::offset_wkbPolygons(entity_transform et, OGRGeometry* ogrGeometry)
{
	OGRMultiPolygon* multiPolygonGeom = (OGRMultiPolygon*)ogrGeometry;
	for (int i = 0; i < multiPolygonGeom->getNumGeometries(); ++i) {
		OGRGeometry* subGeom = multiPolygonGeom->getGeometryRef(i);
		if (wkbFlatten(subGeom->getGeometryType()) == wkbPolygon) {
			OGRPolygon* subPolygon = (OGRPolygon*)subGeom;
			OGRLinearRing* ring = subPolygon->getExteriorRing();
			for (int j = 0; j < ring->getNumPoints(); ++j) {
				ring->setPoint(j, ring->getX(j) + et.offsetX, ring->getY(j) + et.offsetY, ring->getZ(j) + et.offsetZ);
			}
			for (int k = 0; k < subPolygon->getNumInteriorRings(); ++k) {
				OGRLinearRing* interiorRing = subPolygon->getInteriorRing(k);
				for (int l = 0; l < interiorRing->getNumPoints(); ++l) {
					interiorRing->setPoint(k, interiorRing->getX(k) + et.offsetX, interiorRing->getY(k) + et.offsetY, interiorRing->getZ(k) + et.offsetZ);
				}
			}
		}
	}
}

std::string transform_unity::toString_FieldType(OGRFieldType fieldType)
{
	switch (fieldType) {
	case OFTInteger: return "Integer";
	case OFTReal: return "Real";
	case OFTString: return "String";
	case OFTWideString: return "WideString";
	case OFTIntegerList: return "IntegerList";
	case OFTRealList: return "RealList";
	case OFTStringList: return "StringList";
	case OFTWideStringList: return "WideStringList";
	case OFTBinary: return "Binary";
	case OFTDate: return "Date";
	case OFTTime: return "Time";
	case OFTDateTime: return "DateTime";
	case OFTInteger64: return "Integer64";
	case OFTInteger64List: return "Integer64List";
	default: return "Unknown";
	}
}
std::string transform_unity::toString_GeometryType(OGRwkbGeometryType geomType)
{
	switch (wkbFlatten(geomType)) {
	case wkbPoint: return "Point";
	case wkbLineString: return "LineString";
	case wkbPolygon: return "Polygon";
	case wkbMultiPoint: return "MultiPoint";
	case wkbMultiLineString: return "MultiLineString";
	case wkbMultiPolygon: return "MultiPolygon";
	case wkbGeometryCollection: return "GeometryCollection";
	case wkbNone: return "None";
	case wkbLinearRing: return "LinearRing";
	case wkbCircularString: return "CircularString";
	case wkbCompoundCurve: return "CompoundCurve";
	case wkbCurvePolygon: return "CurvePolygon";
	case wkbMultiCurve: return "MultiCurve";
	case wkbMultiSurface: return "MultiSurface";
	case wkbSurface: return "Surface";
	case wkbPolyhedralSurface: return "PolyhedralSurface";
	case wkbTIN: return "TIN";
	case wkbTriangle: return "Triangle";
	default: return "Unknown";
	}
}
std::string transform_unity::toString_RasterDataType(GDALDataType gdalDataType)
{
	switch (gdalDataType) {
	case GDT_Byte: return "Byte";
	case GDT_UInt16: return "UInt16";
	case GDT_Int16: return "Int16";
	case GDT_UInt32: return "UInt32";
	case GDT_Int32: return "Int32";
	case GDT_Float32: return "Float32";
	case GDT_Float64: return "Float64";
	default: return "Unknown";
	}
}

