#ifndef _TOTMS_UNMANAGED_H_
#define _TOTMS_UNMANAGED_H_


#include "toTMS_launch.h"

#pragma region imagery
UMANAGEAPI tifToTMS_imagery(U_TMS* u_param)
{
    toTMS_launch tms;
    tms.initialize(*u_param);
    return tms.toImagery();
}
#pragma endregion


#pragma region terrain
UMANAGEAPI tifToTMS_terrain(U_TMS* u_param)
{
    toTMS_launch tms;
    tms.initialize(*u_param);
    return tms.toTerrain();
}
#pragma endregion



#endif 


