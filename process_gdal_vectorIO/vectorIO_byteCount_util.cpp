#include "vectorIO_bytecount_util.h"
#include <ogrsf_frmts.h>
#include "vectorIO_common_util.h"

vectorIO_byteCount_util::vectorIO_byteCount_util(const char* filePath)
{
    m_gdal.gdalRegister();
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(io_file::stringToUTF8(filePath).c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    m_poLayer = poDS->GetLayer(0);
}

vectorIO_byteCount_util::~vectorIO_byteCount_util()
{
    GDALClose(m_poLayer);
}

bool vectorIO_byteCount_util::getByteCount(GUIntBig& byteCount, bool onlyReadFieldNames, int readAttrCount)
{
    //先遍历，计算所需数组的大小
    byteCount = 0;
    //1.包围盒、坐标系信息
    byteCount += 4 * sizeof(double);//MinX、MaxX、MinY、MaxY
    std::string wkt = m_poLayer->GetSpatialRef()->exportToWkt();
    byteCount += wkt.length() + 1;

    byteCount += sizeof(GIntBig);//要素数量GIntBig

    //2.属性部分
    //标记头
    OGRFeatureDefn* layerDefn = m_poLayer->GetLayerDefn();
    int fieldCount = layerDefn->GetFieldCount();
    byteCount += fieldCount;

    GIntBig featureCount = m_poLayer->GetFeatureCount();
    if (readAttrCount > 0)
        featureCount = std::min(featureCount, (GIntBig)readAttrCount);

    //字段名
    std::vector<int> stringIndices;
    for (int i = 0; i < fieldCount; i++) {
        OGRFieldDefn* fieldDefn = layerDefn->GetFieldDefn(i);
        std::string fieldName(fieldDefn->GetAlternativeNameRef());
        if (fieldName.length() == 0)
            fieldName = fieldDefn->GetNameRef();
        byteCount += fieldName.length() + 1;

        //实际字段内容
        OGRFieldType type = fieldDefn->GetType();
        byteCount += 1;//unsigned char类型，记录type

        if (onlyReadFieldNames)
            continue;

        switch (type)//只处理三种情况，OFTInteger、OFTInteger64、OFTReal，其他均转为string
        {
        case OFTInteger:
            byteCount += sizeof(int) * featureCount;
            break;
        case OFTInteger64:
            byteCount += sizeof(GIntBig) * featureCount;
            break;
        case OFTReal:
            byteCount += sizeof(double) * featureCount;
            break;
        default://转为string的情况，放到下面遍历要素的时候一起计算
            stringIndices.emplace_back(i);
            break;
        }
    }

    if (onlyReadFieldNames)
        return true;

    //补上前面没计算的string类型的属性长度
    OGRFeature* poFeature;
    GIntBig featureIndex = 0;
    while ((poFeature = m_poLayer->GetNextFeature()) != nullptr && featureIndex < featureCount) {
        for (int index : stringIndices) {
            //末尾的+1用于存储终止符\0
            byteCount += std::strlen(poFeature->GetFieldAsString(index)) + 1;
        }
        featureIndex++;
    }
    m_poLayer->ResetReading();

    //3.几何部分
    //标记头
    byteCount += sizeof(int);//几何枚举类型，转为int

    //为了代码结构清晰，这里再遍历一遍。如有性能问题，可与上面属性计算合并，或另寻他法
    OGRwkbGeometryType geomType = layerDefn->GetGeomType();
    switch (geomType)
    {
    case wkbPoint:
    case wkbPointM:
    case wkbPointZM:
    case wkbPoint25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            byteCount += getByteCount_point(poFeature->GetGeometryRef()->toPoint());
        }
        return true;
    case wkbMultiPoint:
    case wkbMultiPointM:
    case wkbMultiPointZM:
    case wkbMultiPoint25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            byteCount += getByteCount_multiPoint(poFeature->GetGeometryRef()->toMultiPoint());
        }
        return true;
    case wkbLineString:
    case wkbLinearRing:
    case wkbLineStringM:
    case wkbLineStringZM:
    case wkbLineString25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            byteCount += getByteCount_lineString(poFeature->GetGeometryRef()->toLineString());
        }
        return true;
    case wkbMultiLineString:
    case wkbMultiLineStringM:
    case wkbMultiLineStringZM:
    case wkbMultiLineString25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            byteCount += getByteCount_multiLineString(poFeature->GetGeometryRef()->toMultiLineString());
        }
        return true;
    case wkbPolygon:
    case wkbPolygonM:
    case wkbPolygonZM:
    case wkbPolygon25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            byteCount += getByteCount_polygon(poFeature->GetGeometryRef()->toPolygon());
        }
        return true;
    case wkbMultiPolygon:
    case wkbMultiPolygonM:
    case wkbMultiPolygonZM:
    case wkbMultiPolygon25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            byteCount += getByteCount_multiPolygon(poFeature->GetGeometryRef()->toMultiPolygon());
        }
        return true;
    default://暂不支持其他类型
        return false;
    }
}

GUIntBig vectorIO_byteCount_util::getByteCount_point(OGRPoint* point)
{
    return sizeof(double) * point->getCoordinateDimension();
}

GUIntBig vectorIO_byteCount_util::getByteCount_multiPoint(OGRMultiPoint* multiPoint)
{
    GUIntBig byteCount = 0;
    int num = multiPoint->getNumGeometries();
    byteCount += sizeof(int);
    for (int i = 0; i < num; i++) {
        byteCount += getByteCount_point(multiPoint->getGeometryRef(i));
    }
    return byteCount;
}

GUIntBig vectorIO_byteCount_util::getByteCount_lineString(OGRLineString* lineString)
{
    GUIntBig byteCount = 0;
    int num = lineString->getNumPoints();
    byteCount += sizeof(int);
    for (int i = 0; i < num; i++) {
        OGRPoint point;
        lineString->getPoint(i, &point);
        byteCount += getByteCount_point(&point);
    }
    return byteCount;
}

GUIntBig vectorIO_byteCount_util::getByteCount_multiLineString(OGRMultiLineString* multiLineString)
{
    GUIntBig byteCount = 0;
    int num = multiLineString->getNumGeometries();
    byteCount += sizeof(int);
    for (int i = 0; i < num; i++) {
        byteCount += getByteCount_lineString(multiLineString->getGeometryRef(i));
    }
    return byteCount;
}

GUIntBig vectorIO_byteCount_util::getByteCount_polygon(OGRPolygon* polygon)
{
    GUIntBig byteCount = 0;
    byteCount += getByteCount_lineString(polygon->getExteriorRing());
    int num = polygon->getNumInteriorRings();
    byteCount += sizeof(int);
    for (int i = 0; i < num; i++) {
        byteCount += getByteCount_lineString(polygon->getInteriorRing(i));
    }
    return byteCount;
}

GUIntBig vectorIO_byteCount_util::getByteCount_multiPolygon(OGRMultiPolygon* multiPolygon)
{
    GUIntBig byteCount = 0;
    int num = multiPolygon->getNumGeometries();
    byteCount += sizeof(int);
    for (int i = 0; i < num; i++) {
        byteCount += getByteCount_polygon(multiPolygon->getGeometryRef(i));
    }
    return byteCount;
}

