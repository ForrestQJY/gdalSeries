#include "gdalToWMTS_unmanaged.h"

bool __stdcall tifToWMTS(U_WMTS* u_WMTS)
{
    gdalToWMTS_helper wmts;
    wmts.initialize(*u_WMTS);
    return wmts.convert();
}
