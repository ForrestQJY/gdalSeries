#include "gdalToTMS_unmanaged.h"

bool __stdcall tifToTMS(U_TMS* u_TMS)
{
    gdalToTMS_helper tms;
    tms.initialize(*u_TMS);
    return tms.convert();
}
