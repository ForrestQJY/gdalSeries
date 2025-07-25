#include "vectorIO_common_util.h"

void vectorIO_common_util::hasZM(OGRwkbGeometryType geomType, bool& hasZ, bool& hasM)
{
    switch (geomType)
    {
    case wkbPoint25D:
    case wkbLineString25D:
    case wkbPolygon25D:
    case wkbMultiPoint25D:
    case wkbMultiLineString25D:
    case wkbMultiPolygon25D:
    case wkbGeometryCollection25D:
        hasZ = true;
        hasM = false;
        break;

    case wkbPointM:
    case wkbLineStringM:
    case wkbPolygonM:
    case wkbMultiPointM:
    case wkbMultiLineStringM:
    case wkbMultiPolygonM:
    case wkbGeometryCollectionM:
    case wkbCircularStringM:
    case wkbCompoundCurveM:
    case wkbCurvePolygonM:
    case wkbMultiCurveM:
    case wkbMultiSurfaceM:
    case wkbCurveM:
    case wkbSurfaceM:
    case wkbPolyhedralSurfaceM:
    case wkbTINM:
    case wkbTriangleM:
        hasZ = false;
        hasM = true;
        break;

    case wkbPointZM:
    case wkbLineStringZM:
    case wkbPolygonZM:
    case wkbMultiPointZM:
    case wkbMultiLineStringZM:
    case wkbMultiPolygonZM:
    case wkbGeometryCollectionZM:
    case wkbCircularStringZM:
    case wkbCompoundCurveZM:
    case wkbCurvePolygonZM:
    case wkbMultiCurveZM:
    case wkbMultiSurfaceZM:
    case wkbCurveZM:
    case wkbSurfaceZM:
    case wkbPolyhedralSurfaceZM:
    case wkbTINZM:
    case wkbTriangleZM:
        hasZ = true;
        hasM = true;
        break;

    default:
        hasZ = false;
        hasM = false;
        break;
    }
}
