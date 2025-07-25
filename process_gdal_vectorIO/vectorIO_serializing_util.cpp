#include "vectorIO_serializing_util.h"
#include <ogrsf_frmts.h>
#include "vectorIO_common_util.h"

vectorIO_serializing_util::vectorIO_serializing_util(const char* filePath, unsigned char* bytePtr)
{
    m_gdal.gdalRegister();
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(io_file::stringToUTF8(filePath).c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    m_poLayer = poDS->GetLayer(0);
    m_bytePtr = bytePtr;
}

vectorIO_serializing_util::~vectorIO_serializing_util()
{
    GDALClose(m_poLayer);
}

bool vectorIO_serializing_util::serializing(bool onlyReadFieldNames, int readAttrCount)
{
    //1.包围盒、坐标系、要素数量信息
    serializing_baseInfo();
    //2.属性部分
    serializing_fields(onlyReadFieldNames, readAttrCount);
    if (onlyReadFieldNames)
        return true;
    //3.几何部分
    return serializing_geometry();
}

void vectorIO_serializing_util::serializing_baseInfo()
{
    //1.包围盒、坐标系、要素数量信息
    OGREnvelope envelope;
    m_poLayer->GetExtent(&envelope);
    *reinterpret_cast<double*>(m_bytePtr) = envelope.MinX;
    m_bytePtr += sizeof(double);
    *reinterpret_cast<double*>(m_bytePtr) = envelope.MaxX;
    m_bytePtr += sizeof(double);
    *reinterpret_cast<double*>(m_bytePtr) = envelope.MinY;
    m_bytePtr += sizeof(double);
    *reinterpret_cast<double*>(m_bytePtr) = envelope.MaxY;
    m_bytePtr += sizeof(double);

    std::string wkt = m_poLayer->GetSpatialRef()->exportToWkt();
    std::memcpy(m_bytePtr, wkt.c_str(), wkt.length());
    m_bytePtr += wkt.length();
    *m_bytePtr = '\0';
    m_bytePtr++;

    GIntBig featureCount = m_poLayer->GetFeatureCount();
    *reinterpret_cast<GIntBig*>(m_bytePtr) = featureCount;
    m_bytePtr += sizeof(GIntBig);
}

void vectorIO_serializing_util::serializing_fields(bool onlyReadFieldNames, int readAttrCount)
{
    //2.属性部分
    OGRFeatureDefn* layerDefn = m_poLayer->GetLayerDefn();
    int fieldCount = layerDefn->GetFieldCount();
    *reinterpret_cast<int*>(m_bytePtr) = fieldCount;
    m_bytePtr += sizeof(int);

    std::vector<OGRFieldType> types;
    for (int i = 0; i < fieldCount; i++) {
        OGRFieldDefn* fieldDefn = layerDefn->GetFieldDefn(i);
        std::string fieldName(fieldDefn->GetAlternativeNameRef());
        if (fieldName.length() == 0)
            fieldName = fieldDefn->GetNameRef();

        std::memcpy(m_bytePtr, fieldName.c_str(), fieldName.length());
        m_bytePtr += fieldName.length();
        *m_bytePtr = '\0';
        m_bytePtr++;

        OGRFieldType type = fieldDefn->GetType();
        *m_bytePtr = (unsigned char)type;
        m_bytePtr++;

        types.emplace_back(type);
    }

    if (onlyReadFieldNames)
        return;

    GIntBig featureCount = m_poLayer->GetFeatureCount();
    if (readAttrCount > 0)
        featureCount = std::min(featureCount, (GIntBig)readAttrCount);
    //写入属性值
    OGRFeature* poFeature;
    GIntBig featureIndex = 0;
    while ((poFeature = m_poLayer->GetNextFeature()) != nullptr && featureIndex < featureCount) {
        for (int i = 0; i < fieldCount; i++) {
            switch (types[i])
            {
            case OFTInteger:
                *reinterpret_cast<int*>(m_bytePtr) = poFeature->GetFieldAsInteger(i);
                m_bytePtr += sizeof(int);
                break;
            case OFTInteger64:
                *reinterpret_cast<GIntBig*>(m_bytePtr) = poFeature->GetFieldAsInteger64(i);
                m_bytePtr += sizeof(GIntBig);
                break;
            case OFTReal:
                *reinterpret_cast<double*>(m_bytePtr) = poFeature->GetFieldAsDouble(i);
                m_bytePtr += sizeof(double);
                break;
            default:
                const char* value = poFeature->GetFieldAsString(i);
                int length = std::strlen(value);
                std::memcpy(m_bytePtr, value, length);
                m_bytePtr += length;
                *m_bytePtr = '\0';//写入终止符
                m_bytePtr++;
                break;
            }
        }
        featureIndex++;
    }
    m_poLayer->ResetReading();
}

bool vectorIO_serializing_util::serializing_geometry()
{
    OGRFeatureDefn* layerDefn = m_poLayer->GetLayerDefn();
    OGRwkbGeometryType geomType = layerDefn->GetGeomType();
    *reinterpret_cast<int*>(m_bytePtr) = geomType;
    m_bytePtr += sizeof(int);

    OGRFeature* poFeature;
    switch (geomType)
    {
    case wkbPoint:
    case wkbPointM:
    case wkbPointZM:
    case wkbPoint25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            serializing_point(poFeature->GetGeometryRef()->toPoint());
        }
        return true;
    case wkbMultiPoint:
    case wkbMultiPointM:
    case wkbMultiPointZM:
    case wkbMultiPoint25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            serializing_multiPoint(poFeature->GetGeometryRef()->toMultiPoint());
        }
        return true;
    case wkbLineString:
    case wkbLinearRing:
    case wkbLineStringM:
    case wkbLineStringZM:
    case wkbLineString25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            serializing_lineString(poFeature->GetGeometryRef()->toLineString());
        }
        return true;
    case wkbMultiLineString:
    case wkbMultiLineStringM:
    case wkbMultiLineStringZM:
    case wkbMultiLineString25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            serializing_multiLineString(poFeature->GetGeometryRef()->toMultiLineString());
        }
        return true;
    case wkbPolygon:
    case wkbPolygonM:
    case wkbPolygonZM:
    case wkbPolygon25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            serializing_polygon(poFeature->GetGeometryRef()->toPolygon());
        }
        return true;
    case wkbMultiPolygon:
    case wkbMultiPolygonM:
    case wkbMultiPolygonZM:
    case wkbMultiPolygon25D:
        while ((poFeature = m_poLayer->GetNextFeature()) != nullptr) {
            serializing_multiPolygon(poFeature->GetGeometryRef()->toMultiPolygon());
        }
        return true;
    default://暂不支持其他类型
        return false;
    }
}

void vectorIO_serializing_util::serializing_point(OGRPoint* point)
{
    *reinterpret_cast<double*>(m_bytePtr) = point->getX();
    m_bytePtr += sizeof(double);
    *reinterpret_cast<double*>(m_bytePtr) = point->getY();
    m_bytePtr += sizeof(double);
    if (point->Is3D()) {
        *reinterpret_cast<double*>(m_bytePtr) = point->getZ();
        m_bytePtr += sizeof(double);
    }
    if (point->IsMeasured()) {
        *reinterpret_cast<double*>(m_bytePtr) = point->getM();
        m_bytePtr += sizeof(double);
    }
}

void vectorIO_serializing_util::serializing_multiPoint(OGRMultiPoint* multiPoint)
{
    int num = multiPoint->getNumGeometries();
    *reinterpret_cast<int*>(m_bytePtr) = num;
    m_bytePtr += sizeof(int);
    for (int i = 0; i < num; i++) {
        OGRPoint* point = multiPoint->getGeometryRef(i);
        serializing_point(point);
    }
}

void vectorIO_serializing_util::serializing_lineString(OGRLineString* lineString)
{
    int num = lineString->getNumPoints();
    *reinterpret_cast<int*>(m_bytePtr) = num;
    m_bytePtr += sizeof(int);
    for (int i = 0; i < num; i++) {
        OGRPoint point;
        lineString->getPoint(i, &point);
        serializing_point(&point);
    }
}

void vectorIO_serializing_util::serializing_multiLineString(OGRMultiLineString* multiLineString)
{
    int num = multiLineString->getNumGeometries();
    *reinterpret_cast<int*>(m_bytePtr) = num;
    m_bytePtr += sizeof(int);
    for (int i = 0; i < num; i++) {
        OGRLineString* lineString = multiLineString->getGeometryRef(i);
        serializing_lineString(lineString);
    }
}

void vectorIO_serializing_util::serializing_polygon(OGRPolygon* polygon)
{
    serializing_lineString(polygon->getExteriorRing());
    int num = polygon->getNumInteriorRings();
    *reinterpret_cast<int*>(m_bytePtr) = num;
    m_bytePtr += sizeof(int);
    for (int i = 0; i < num; i++) {
        serializing_lineString(polygon->getInteriorRing(i));
    }
}

void vectorIO_serializing_util::serializing_multiPolygon(OGRMultiPolygon* multiPolygon)
{
    int num = multiPolygon->getNumGeometries();
    *reinterpret_cast<int*>(m_bytePtr) = num;
    m_bytePtr += sizeof(int);
    for (int i = 0; i < num; i++) {
        OGRPolygon* polygon = multiPolygon->getGeometryRef(i);
        serializing_polygon(polygon);
    }
}
