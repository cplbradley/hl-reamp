#include "cbase.h"
#include "hlr/hud/hud_redkey.h"
#include "hlr/hud/hud_bluekey.h"
#include "hlr/hud/hud_purplekey.h"

class CHudRedKey;
class CHudBlueKey;
class CHudPurpleKey;

enum {
	KEYCOLOR_RED,
	KEYCOLOR_BLUE,
	KEYCOLOR_PURPLE,
};

class C_HLRKeycard : public C_BaseAnimating
{
public:
	C_HLRKeycard() {};

	DECLARE_CLASS(C_HLRKeycard, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void CheckHudElement();
	bool		m_bDrawHud;
	int			m_nColor;
	virtual int DrawModel(int flags);

	CHudRedKey *m_pRedKey;
	CHudBlueKey *m_pBlueKey;
	CHudPurpleKey *m_pPurpleKey;

private:
	C_HLRKeycard(const C_HLRKeycard &);
};

BEGIN_DATADESC(C_HLRKeycard)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_HLRKeycard,DT_Keycard,CHLRKeycard)
RecvPropBool(RECVINFO(m_bDrawHud)),
RecvPropInt(RECVINFO(m_nColor)),
END_RECV_TABLE()


void C_HLRKeycard::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		m_pRedKey = (CHudRedKey *)GET_HUDELEMENT(CHudRedKey);
		Assert(m_pRedKey);
		m_pBlueKey = (CHudBlueKey *)GET_HUDELEMENT(CHudBlueKey);
		Assert(m_pBlueKey);
		m_pPurpleKey = (CHudPurpleKey *)GET_HUDELEMENT(CHudPurpleKey);
		Assert(m_pPurpleKey);
		CheckHudElement();
	}
	if (type == DATA_UPDATE_DATATABLE_CHANGED)
	{
		CheckHudElement();
	}
}

void C_HLRKeycard::CheckHudElement()
{
	switch (m_nColor)
	{
	case KEYCOLOR_RED:
	{
		m_pRedKey->bShowKey = m_bDrawHud;
		if (m_bDrawHud)
			DevMsg("clientside key bool true");
		break;
	}
	case KEYCOLOR_BLUE:
	{
		m_pBlueKey->bShowKey = m_bDrawHud;
		break;
	}
	case KEYCOLOR_PURPLE:
	{
		m_pPurpleKey->bShowKey = m_bDrawHud;
		break;
	}
	default:
		break;
	}
}
int C_HLRKeycard::DrawModel(int flags)
{
	if (m_bDrawHud)
		return 0;
	return BaseClass::DrawModel(flags);
}