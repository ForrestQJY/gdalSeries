#ifndef _VECTORIO_UNMANAGED_H_
#define _VECTORIO_UNMANAGED_H_

#include <io_constant.h>
#include <geo_gdal.h>
#include <ogrsf_frmts.h>
#include "vectorIO_bytecount_util.h"
#include "vectorIO_serializing_util.h"

UMANAGEAPI getVectorByteArrSize(const char* filePath, GUIntBig& byteCount, bool onlyReadFieldNames, int readAttrCount) {
    vectorIO_byteCount_util util(filePath);
    return util.getByteCount(byteCount, onlyReadFieldNames, readAttrCount);
}

UMANAGEAPI getVectorByteArr(const char* filePath, unsigned char* byteArr, bool onlyReadFieldNames, int readAttrCount) {
    vectorIO_serializing_util util(filePath, byteArr);
    return util.serializing(onlyReadFieldNames, readAttrCount);
}

#endif