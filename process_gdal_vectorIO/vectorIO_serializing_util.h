#ifndef _VECTORIO_SERIALIZING_UTIL_H_
#define _VECTORIO_SERIALIZING_UTIL_H_

#include <geo_gdal.h>

class OGRLayer;

class vectorIO_serializing_util {
public:
    vectorIO_serializing_util(const char* filePath, unsigned char* bytePtr);
    ~vectorIO_serializing_util();
public:
    bool serializing(bool onlyReadFieldNames, int readAttrCount);
private:
    void serializing_baseInfo();
    void serializing_fields(bool onlyReadFieldNames, int readAttrCount);
    bool serializing_geometry();
    void serializing_point(OGRPoint* point);
    void serializing_multiPoint(OGRMultiPoint* multiPoint);
    void serializing_lineString(OGRLineString* lineString);
    void serializing_multiLineString(OGRMultiLineString* multiLineString);
    void serializing_polygon(OGRPolygon* polygon);
    void serializing_multiPolygon(OGRMultiPolygon* multiPolygon);
private:
    geo_gdal m_gdal;
    OGRLayer* m_poLayer;
    unsigned char* m_bytePtr;
};

#endif
