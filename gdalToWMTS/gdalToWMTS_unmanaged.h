#ifndef _GDALTOWMTS_UNMANAGED_H_
#define _GDALTOWMTS_UNMANAGED_H_


#include <unmanagedClass_wmts.h>

#include "gdalToWMTS_helper.h"


#pragma region terrainServer
extern "C" UMANAGEAPI bool __stdcall tifToWMTS(U_WMTS * u_WMTS);

#pragma endregion



#endif //ctb_unmanaged


