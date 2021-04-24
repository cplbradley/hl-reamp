#include "cbase.h"

#include <baseviewport.h>
#include "tier0/memdbgon.h"


extern ConVar cl_drawhud;
class C_Util_HudBlocker : public C_BaseEntity
{

public:
	C_Util_HudBlocker() {};
	DECLARE_CLASS(C_Util_HudBlocker, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();
	
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink();

private:
	bool bEnabled;

protected:
	CBaseViewport			*m_pViewport;
};

BEGIN_DATADESC(C_Util_HudBlocker)
END_DATADESC()

IMPLEMENT_CLIENTCLASS_DT(C_Util_HudBlocker,DT_HudBlocker,CUtilHudBlocker)
RecvPropBool(RECVINFO(bEnabled)),
END_RECV_TABLE()


void C_Util_HudBlocker::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}
void C_Util_HudBlocker::ClientThink(void)
{
	if (bEnabled)
	{
		if (cl_drawhud.GetBool() == true)
			cl_drawhud.SetValue(0);
		DevMsg("enabling hudblocker\n");
	}
	else
	{
		if (cl_drawhud.GetBool() == false)
			cl_drawhud.SetValue(1);
		DevMsg("disabling hudblocker\n");
	}
}