#include "gdalToWMTS_static.h"

void pStatic::setParameters(U_WMTS u_WMTS)
{
	u_Param.f_Basic.Input = u_WMTS.f_Basic.Input;
	u_Param.f_Basic.OverlayFile = u_WMTS.f_Basic.OverlayFile == 1 ? 1 : 0;
	u_Param.f_Basic.Output = u_WMTS.f_Basic.Output;
	u_Param.f_Basic.OutputFormat = u_WMTS.f_Basic.OutputFormat;
	u_Param.f_Basic.RunnableThread = u_WMTS.f_Basic.RunnableThread < 1 ? 1 : u_WMTS.f_Basic.RunnableThread;
	u_Param.f_Basic.UId = u_WMTS.f_Basic.UId == NULL ? DEFAULT_UID : u_WMTS.f_Basic.UId;

	u_Param.f_WMTSConfig = u_WMTS.f_WMTSConfig;

	u_Param.f_Delegation = u_WMTS.f_Delegation;

	callbackOriginator co(GDALTOWMTS_NAME, u_Param.f_Basic.UId);
	co.setCallback("Delegate_Message", u_Param.f_Delegation.Delegate_Message);
	co.setCallback("Delegate_Progress", u_Param.f_Delegation.Delegate_Progress);
	co.setCallback("Delegate_Value", u_Param.f_Delegation.Delegate_Value);

	callback_Originator = co;
}

U_WMTS pStatic::u_Param;