#ifndef _GDALTOTMS_UNMANAGED_H_
#define _GDALTOTMS_UNMANAGED_H_


#include <unmanagedClass_tms.h>
#include "gdalToTMS_helper.h"


#pragma region terrainServer
extern "C" UMANAGEAPI bool __stdcall tifToTMS(U_TMS * u_TMS);

#pragma endregion



#endif //ctb_unmanaged


