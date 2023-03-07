#include "cbase.h"
#include "hud_poweruptime.h"



class C_HLRFreemanFury : public C_BaseAnimating
{
	
	DECLARE_CLASS(C_HLRFreemanFury,C_BaseAnimating)
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();
	
public:
	C_HLRFreemanFury() {};
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void CheckHudElement();
	virtual void InitHud();
	virtual void ClientThink();
	virtual int DrawModel(int flags);
	bool m_bDrawHud;
	int m_iTimeLeft;

	CHudProgressFF *hud;
private:
	C_HLRFreemanFury(const C_HLRFreemanFury &);
};


BEGIN_DATADESC(C_HLRFreemanFury)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_HLRFreemanFury,DT_FreemanFury,CHLRFreemanFury)
RecvPropBool(RECVINFO(m_bDrawHud)),
RecvPropInt(RECVINFO(m_iTimeLeft)),
END_RECV_TABLE()

void C_HLRFreemanFury::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
		InitHud();
	if (type == DATA_UPDATE_DATATABLE_CHANGED)
	{
		CheckHudElement();
		if (m_bDrawHud)
		{
			DevMsg("clientside bool is true\n");
			DevMsg("clientside timeleft = %i", m_iTimeLeft);
		}
	}
}
void C_HLRFreemanFury::InitHud()
{
	hud = (CHudProgressFF *)GET_HUDELEMENT(CHudProgressFF);
	m_iTimeLeft = 30;
	CheckHudElement();
}
void C_HLRFreemanFury::CheckHudElement()
{
	hud->bShowIcon = m_bDrawHud;
	hud->m_iTimeLeft = m_iTimeLeft - 1;
}
void C_HLRFreemanFury::ClientThink()
{
	BaseClass::ClientThink();
	
}
int C_HLRFreemanFury::DrawModel(int flags)
{
	if (m_bDrawHud)
		return 0;
	return BaseClass::DrawModel(flags);
}

//-------------------------------------------------------------------------------------------------//
//  ______   ____    ____  _______ .______       _______  .______       __  ____    ____  _______  //
// /  __  \  \   \  /   / |   ____||   _  \     |       \ |   _  \     |  | \   \  /   / |   ____| //
//|  |  |  |  \   \/   /  |  |__   |  |_)  |    |  .--.  ||  |_)  |    |  |  \   \/   /  |  |__    //
//|  |  |  |   \      /   |   __|  |      /     |  |  |  ||      /     |  |   \      /   |   __|   //
//|  `--'  |    \    /    |  |____ |  |\  \----.|  '--'  ||  |\  \----.|  |    \    /    |  |____  //
// \______/      \__/     |_______|| _| `._____||_______/ | _| `._____||__|     \__/     |_______| //
//-------------------------------------------------------------------------------------------------//

class C_HLROverdrive : public C_BaseAnimating
{

	DECLARE_CLASS(C_HLROverdrive, C_BaseAnimating)
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

public:
	C_HLROverdrive() {};
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void CheckHudElement();
	virtual void InitHud();
	virtual void ClientThink();
	virtual int DrawModel(int flags);
	bool m_bDrawHud;
	int m_iTimeLeft;

	CHudProgressOD *hud;
private:
	C_HLROverdrive(const C_HLROverdrive &);
};


BEGIN_DATADESC(C_HLROverdrive)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_HLROverdrive, DT_Overdrive, CHLROverdrive)
RecvPropBool(RECVINFO(m_bDrawHud)),
RecvPropInt(RECVINFO(m_iTimeLeft)),
END_RECV_TABLE()

void C_HLROverdrive::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
		InitHud();
	if (type == DATA_UPDATE_DATATABLE_CHANGED)
	{
		CheckHudElement();
		if (m_bDrawHud)
		{
			DevMsg("clientside bool is true\n");
			DevMsg("clientside timeleft = %i", m_iTimeLeft);
		}
	}
}
void C_HLROverdrive::InitHud()
{
	hud = (CHudProgressOD *)GET_HUDELEMENT(CHudProgressOD);
	m_iTimeLeft = 30;
	CheckHudElement();
}
void C_HLROverdrive::CheckHudElement()
{
	hud->bShowIcon = m_bDrawHud;
	hud->m_iTimeLeft = m_iTimeLeft - 1;
}
void C_HLROverdrive::ClientThink()
{
	BaseClass::ClientThink();

}
int C_HLROverdrive::DrawModel(int flags)
{
	if (m_bDrawHud)
		return 0;
	return BaseClass::DrawModel(flags);
}


class C_HLRQuadJump : public C_BaseAnimating
{

	DECLARE_CLASS(C_HLRQuadJump, C_BaseAnimating)
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

public:
	C_HLRQuadJump() {};
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void CheckHudElement();
	virtual void InitHud();
	virtual void ClientThink();
	virtual int DrawModel(int flags);
	bool m_bDrawHud;
	int m_iTimeLeft;

	CHudProgressQJ* hud;
private:
	C_HLRQuadJump(const C_HLRQuadJump&);
};

BEGIN_DATADESC(C_HLRQuadJump)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_HLRQuadJump, DT_QuadJump, CHLRQuadJump)
RecvPropBool(RECVINFO(m_bDrawHud)),
RecvPropInt(RECVINFO(m_iTimeLeft)),
END_RECV_TABLE()

void C_HLRQuadJump::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
		InitHud();
	if (type == DATA_UPDATE_DATATABLE_CHANGED)
	{
		CheckHudElement();
		if (m_bDrawHud)
		{
			DevMsg("clientside bool is true\n");
			DevMsg("clientside timeleft = %i", m_iTimeLeft);
		}
	}
}

void C_HLRQuadJump::InitHud()
{
	hud = (CHudProgressQJ*)GET_HUDELEMENT(CHudProgressQJ);
	m_iTimeLeft = 30;
	CheckHudElement();
}
void C_HLRQuadJump::CheckHudElement()
{
	hud->bShowIcon = m_bDrawHud;
	hud->m_iTimeLeft = m_iTimeLeft - 1;
}
void C_HLRQuadJump::ClientThink()
{
	BaseClass::ClientThink();

}
int C_HLRQuadJump::DrawModel(int flags)
{
	if (m_bDrawHud)
		return 0;
	return BaseClass::DrawModel(flags);
}