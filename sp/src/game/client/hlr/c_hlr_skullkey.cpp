#include "cbase.h"
#include "hud_skullkey.h"

class CHudRedSkullKey;
class CHudBlueSkullKey;
class CHudPurpleSkullKey;

enum {
	KEYCOLOR_RED,
	KEYCOLOR_BLUE,
	KEYCOLOR_PURPLE,
};

class C_HLRSkullKey : public C_BaseAnimating
{
public:
	C_HLRSkullKey() {};

	DECLARE_CLASS(C_HLRSkullKey, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void CheckHudElement();
	bool		m_bDrawHud;
	int			m_nColor;
	virtual int DrawModel(int flags);

	CHudRedSkullKey *m_pRedKey;
	CHudBlueSkullKey *m_pBlueKey;
	CHudPurpleSkullKey *m_pPurpleKey;

private:
	C_HLRSkullKey(const C_HLRSkullKey &);
};

BEGIN_DATADESC(C_HLRSkullKey)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_HLRSkullKey, DT_SkullKey, CHLRSkullKey)
RecvPropBool(RECVINFO(m_bDrawHud)),
RecvPropInt(RECVINFO(m_nColor)),
END_RECV_TABLE()


void C_HLRSkullKey::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		m_pRedKey = (CHudRedSkullKey *)GET_HUDELEMENT(CHudRedSkullKey);
		Assert(m_pRedKey);
		m_pBlueKey = (CHudBlueSkullKey *)GET_HUDELEMENT(CHudBlueSkullKey);
		Assert(m_pBlueKey);
		m_pPurpleKey = (CHudPurpleSkullKey *)GET_HUDELEMENT(CHudPurpleSkullKey);
		Assert(m_pPurpleKey);
		CheckHudElement();
	}
	if (type == DATA_UPDATE_DATATABLE_CHANGED)
	{
		CheckHudElement();
	}
}

void C_HLRSkullKey::CheckHudElement()
{
	switch (m_nColor)
	{
	case KEYCOLOR_RED:
	{
		m_pRedKey->bShowKey = m_bDrawHud;
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
int C_HLRSkullKey::DrawModel(int flags)
{
	if (m_bDrawHud)
		return 0;
	return BaseClass::DrawModel(flags);
}