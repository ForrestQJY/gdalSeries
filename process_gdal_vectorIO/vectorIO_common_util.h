#ifndef _VECTORIO_COMMON_UTIL_H_
#define _VECTORIO_COMMON_UTIL_H_

#include <ogr_core.h>

class vectorIO_common_util {
public:
    vectorIO_common_util() {}
    ~vectorIO_common_util() {}
public:
    void hasZM(OGRwkbGeometryType geomType, bool& hasZ, bool& hasM);
};

#endif