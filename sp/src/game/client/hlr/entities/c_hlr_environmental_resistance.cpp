#include "cbase.h"
#include "hlr/hud/hud_environmental_resistance.h"
#include "c_baseplayer.h"
#include "dlight.h"
#include "iefx.h"


class C_HLREnvironmentalResistance : public C_BaseAnimating
{

	DECLARE_CLASS(C_HLREnvironmentalResistance, C_BaseAnimating)
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

public:
	C_HLREnvironmentalResistance() {};
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void CheckHudElement();
	virtual void InitHud();
	virtual void ClientThink();
	virtual void InitDLight();
	virtual int DrawModel(int flags);
	bool m_bDrawHud;
	int m_iToxicDamageLeft;
	int m_iFireDamageLeft;
	int m_iElectricDamageLeft;
	int m_iDamageType;

	CHudEnvironmentResistToxic *toxic;
	CHudEnvironmentResistFire *fire;
	CHudEnvironmentResistElectric *shock;

	dlight_t *dl;
private:
	C_HLREnvironmentalResistance(const C_HLREnvironmentalResistance &);
};


BEGIN_DATADESC(C_HLREnvironmentalResistance)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_HLREnvironmentalResistance, DT_EnvironmentalResistance, CHLREnvironmentResistance)
RecvPropBool(RECVINFO(m_bDrawHud)),
RecvPropBool(RECVINFO(m_iDamageType)),
END_RECV_TABLE()

void C_HLREnvironmentalResistance::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		InitHud();
		InitDLight();
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
		
	if (type == DATA_UPDATE_DATATABLE_CHANGED)
	{
		CheckHudElement();
		if (m_bDrawHud)
		{
			DevMsg("clientside bool is true\n");
			DevMsg("clientside fire damageleft = %i\n", m_iFireDamageLeft);
			DevMsg("clientside toxic damageleft = %i\n", m_iToxicDamageLeft);
			DevMsg("clientside electric damageleft = %i\n", m_iElectricDamageLeft);
		}
	}
	DevMsg("idk lol");
}
void C_HLREnvironmentalResistance::InitHud()
{
	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
	toxic = (CHudEnvironmentResistToxic *)GET_HUDELEMENT(CHudEnvironmentResistToxic);
	fire = (CHudEnvironmentResistFire *)GET_HUDELEMENT(CHudEnvironmentResistFire);
	shock = (CHudEnvironmentResistElectric *)GET_HUDELEMENT(CHudEnvironmentResistElectric);
	m_iToxicDamageLeft = pPlayer->m_iToxicDamageLeft;
	m_iFireDamageLeft = pPlayer->m_iFireDamageLeft;
	m_iElectricDamageLeft = pPlayer->m_iElectricDamageLeft;
	CheckHudElement();
}
void C_HLREnvironmentalResistance::CheckHudElement()
{
	
}
void C_HLREnvironmentalResistance::InitDLight()
{
	dl = effects->CL_AllocDlight(index);
	dl->origin = GetAbsOrigin();
	dl->die = FLT_MAX;
	dl->radius = random->RandomFloat(100.0f, 128.0f);
	switch (m_iDamageType)
	{
	case 0:
	{
		dl->color.r = 128;
		dl->color.g = 180;
		dl->color.b = 0;
		break;
	}
	case 1:
	{
		dl->color.r = 255;
		dl->color.g = 100;
		dl->color.b = 0;
		break;
	}
	case 2:
	{
		dl->color.r = 0;
		dl->color.g = 128;
		dl->color.b = 255;
		break;
	}
	default:
		break;
	}

}
void C_HLREnvironmentalResistance::ClientThink()
{
	CheckHudElement();
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
	if (!pPlayer->IsSuitEquipped())
		return;
	if (m_bDrawHud && dl != NULL)
	{
		dl->die = gpGlobals->curtime;
		dl = NULL;
	}


	m_iToxicDamageLeft = pPlayer->m_iToxicDamageLeft;
	m_iFireDamageLeft = pPlayer->m_iFireDamageLeft;
	m_iElectricDamageLeft = pPlayer->m_iElectricDamageLeft;
	if (m_iToxicDamageLeft <= 0)
		toxic->bShowIcon = false;
	else
		toxic->bShowIcon = true;

	if (m_iFireDamageLeft <= 0)
		fire->bShowIcon = false;
	else
		fire->bShowIcon = true;
	if (m_iElectricDamageLeft <= 0)
		shock->bShowIcon = false;
	else
		shock->bShowIcon = true;

	BaseClass::ClientThink();

}
int C_HLREnvironmentalResistance::DrawModel(int flags)
{
	if (m_bDrawHud)
		return 0;
	return BaseClass::DrawModel(flags);
}