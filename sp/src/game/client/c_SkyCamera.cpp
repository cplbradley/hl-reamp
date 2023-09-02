#include "cbase.h"
#include "c_playerlocaldata.h"

#include "tier0/memdbgon.h"

class C_SkyCamera : public C_BaseEntity
{
	DECLARE_CLASS(C_SkyCamera, C_BaseEntity);
	DECLARE_CLIENTCLASS();

public:
	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t type);
private:
	bool bParented;
	bool bUseAngles;
	bool bAmActiveSkyCamera;
};

IMPLEMENT_CLIENTCLASS_DT(C_SkyCamera,DT_SkyCamera,CSkyCamera)
RecvPropBool(RECVINFO(bParented)),
RecvPropBool(RECVINFO(bUseAngles)),
RecvPropBool(RECVINFO(bAmActiveSkyCamera)),
END_RECV_TABLE()

void C_SkyCamera::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		ConColorMsg(Color(0, 128, 255), "SkyCamera spawned on the client, parented = %i, useangles = %i\n",bParented,bUseAngles);
		if(bParented)
			SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

void C_SkyCamera::ClientThink()
{
	if (bParented)
	{
		CBasePlayer* player = UTIL_PlayerByIndex(1);
		if (!player)
			return;
		if (!bAmActiveSkyCamera)
			return;
		player->m_Local.m_skybox3d.origin = GetAbsOrigin();
		if (bUseAngles)
			player->m_Local.m_skybox3d.angles = GetAbsAngles();
	}
}