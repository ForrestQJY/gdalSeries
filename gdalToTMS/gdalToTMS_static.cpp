#include "gdalToTMS_static.h"

void pStatic::setParameters(U_TMS u_TMS)
{
	u_Param.f_Basic.Input = u_TMS.f_Basic.Input;
	u_Param.f_Basic.OverlayFile = u_TMS.f_Basic.OverlayFile == 1 ? 1 : 0;
	u_Param.f_Basic.Output = u_TMS.f_Basic.Output;
	u_Param.f_Basic.OutputFormat = u_TMS.f_Basic.OutputFormat;
	u_Param.f_Basic.RunnableThread = u_TMS.f_Basic.RunnableThread < 1 ? 1 : u_TMS.f_Basic.RunnableThread;
	u_Param.f_Basic.UId = u_TMS.f_Basic.UId == NULL ? DEFAULT_UID : u_TMS.f_Basic.UId;

	u_Param.f_TMSConfig = u_TMS.f_TMSConfig;

	u_Param.f_Delegation = u_TMS.f_Delegation;

	callbackOriginator co(GDALTOTMS_NAME, u_Param.f_Basic.UId);
	co.setCallback("Delegate_Message", u_Param.f_Delegation.Delegate_Message);
	co.setCallback("Delegate_Progress", u_Param.f_Delegation.Delegate_Progress);
	co.setCallback("Delegate_Value", u_Param.f_Delegation.Delegate_Value);

	callback_Originator = co;
}

U_TMS pStatic::u_Param;