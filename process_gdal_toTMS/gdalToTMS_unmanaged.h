#ifndef _GDALTOTMS_UNMANAGED_H_
#define _GDALTOTMS_UNMANAGED_H_


#include "gdalToTMS_launch.h"


#pragma region terrain
UMANAGEAPI tifToTMS_terrain(U_TMS* u_param)
{
    gdalToTMS_launch tms;
    tms.initialize(*u_param);
    return tms.toTerrain();
}
#pragma endregion


#pragma region imagery
UMANAGEAPI tifToTMS_imagery(U_TMS* u_param)
{
    gdalToTMS_launch tms;
    tms.initialize(*u_param);
    return tms.toImagery();
}
#pragma endregion


#endif 


