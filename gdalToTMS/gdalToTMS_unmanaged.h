#ifndef _GDALTOTMS_UNMANAGED_H_
#define _GDALTOTMS_UNMANAGED_H_


#include "gdalToTMS_helper.h"


#pragma region terrainServer
UMANAGEAPI tifToTMS(U_TMS* u_param)
{
    gdalToTMS_helper tms;
    tms.initialize(*u_param);
    return tms.convert();
}

#pragma endregion



#endif //ctb_unmanaged


