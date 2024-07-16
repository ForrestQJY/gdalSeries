#include "gdalToTMS_unmanaged.h"

bool __stdcall tifToTMS(U_TMS* u_param)
{
    gdalToTMS_helper tms;
    tms.initialize(*u_param);
    return tms.convert();
}
