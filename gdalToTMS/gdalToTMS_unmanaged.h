#ifndef _GDALTOTMS_UNMANAGED_H_
#define _GDALTOTMS_UNMANAGED_H_


#include "gdalToTMS_helper.h"


#pragma region terrainServer
extern "C" UMANAGEAPI bool __stdcall tifToTMS(U_TMS* u_param);

#pragma endregion



#endif //ctb_unmanaged


