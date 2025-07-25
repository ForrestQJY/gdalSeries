#ifndef _VECTORIO_BYTECOUNT_UTIL_H_
#define _VECTORIO_BYTECOUNT_UTIL_H_

#include <geo_gdal.h>

class OGRLayer;

class vectorIO_byteCount_util {
public:
    vectorIO_byteCount_util(const char* filePath);
    ~vectorIO_byteCount_util();
public:
    bool getByteCount(GUIntBig& byteCount, bool onlyReadFieldNames, int readAttrCount);
private:
    GUIntBig getByteCount_point(OGRPoint* point);
    GUIntBig getByteCount_multiPoint(OGRMultiPoint* multiPoint);
    GUIntBig getByteCount_lineString(OGRLineString* lineString);
    GUIntBig getByteCount_multiLineString(OGRMultiLineString* multiLineString);
    GUIntBig getByteCount_polygon(OGRPolygon* polygon);
    GUIntBig getByteCount_multiPolygon(OGRMultiPolygon* multiPolygon);
private:
    geo_gdal m_gdal;
    OGRLayer* m_poLayer;
};

#endif