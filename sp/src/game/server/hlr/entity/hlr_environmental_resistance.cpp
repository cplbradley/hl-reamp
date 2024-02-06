#include "cbase.h"
#include "particle_system.h"
#include "particle_parse.h"
#include "player.h"
#include "game.h"
#include "shareddefs.h"


#include "tier0/memdbgon.h"

#define POWERUP_MODEL "models/items/envresist.mdl"
#define TOXIC_DAMAGE 0
#define FIRE_DAMAGE 1
#define ELECTRIC_DAMAGE 2


class CHLREnvironmentResistance : public CBaseAnimating
{
	DECLARE_CLASS(CHLREnvironmentResistance, CBaseAnimating);
	DECLARE_SERVERCLASS();

public:
	int UpdateTransmitState()	// always send to all clients
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}
	void Precache(void);
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
	void EnableBlocker(void);
	void Think(void);
	void Kill(void);
	void SetEnvColor(void);

	COutputEvent m_OnPickup;
	

private:
	CNetworkVar(bool, m_bDrawHud);
	CNetworkVar(int, m_iDamageType);
	int m_iDamageLeft;
	int m_iPoints;

	CNetworkVar(int, m_iMaxToxicPoints);
	CNetworkVar(int, m_iMaxFirePoints);
	/*bool m_bDrawHud;
	int m_iDamageType;
	int m_iDamageLeft;*/




	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_environmental_resistance, CHLREnvironmentResistance);

BEGIN_DATADESC(CHLREnvironmentResistance)
DEFINE_FIELD(m_bDrawHud, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_iDamageType, FIELD_INTEGER, "DamageType"),
DEFINE_KEYFIELD(m_iPoints, FIELD_INTEGER, "BlockPoints"),
DEFINE_FIELD(m_iMaxToxicPoints,FIELD_INTEGER),
DEFINE_FIELD(m_iMaxFirePoints,FIELD_INTEGER),
DEFINE_THINKFUNC(Think),
DEFINE_FUNCTION(Touch),
DEFINE_OUTPUT(m_OnPickup, "OnPickup"),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CHLREnvironmentResistance,DT_EnvironmentalResistance)
SendPropBool(SENDINFO(m_bDrawHud)),
SendPropInt(SENDINFO(m_iDamageType)),
END_SEND_TABLE()


void CHLREnvironmentResistance::Precache(void)
{
	PrecacheModel(POWERUP_MODEL);
}
void CHLREnvironmentResistance::Spawn(void)
{
	Precache();
	SetModel(POWERUP_MODEL);
	SetSolid(SOLID_BBOX);
	SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	int bodygroup = FindBodygroupByName("envresist_BG1");
	SetBodygroup(bodygroup, m_iDamageType);
	SetEnvColor();
	SetTouch(&CHLREnvironmentResistance::Touch);
}
void CHLREnvironmentResistance::SetEnvColor(void)
{
	switch (m_iDamageType)
	{
	case 0:
	{
		SetRenderColor(128, 180, 0);
		break;
	}
	case 1:
	{
		SetRenderColor(255, 100, 0);
		break;
	}
	case 2:
	{
		SetRenderColor(0, 128, 255);
		break;
	}
	default:
		break;
	}
}
void CHLREnvironmentResistance::Touch(CBaseEntity *pOther)
{
	if (!pOther)
		return;
	if (pOther->IsPlayer())
	{
		m_OnPickup.FireOutput(pOther, this);
		EnableBlocker();
		SetThink(&CHLREnvironmentResistance::Think);
		SetNextThink(gpGlobals->curtime + 0.5f);
		SetSolid(SOLID_NONE);
		SetTouch(NULL);
	}
}
void CHLREnvironmentResistance::EnableBlocker(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	m_iDamageLeft = m_iPoints;
	m_bDrawHud = true;
	switch (m_iDamageType)
	{
	case 0:
	{
		DevMsg("setting toxic blocker points to %i\n", m_iPoints);
		pPlayer->SetBlockerDamageLeft(m_iDamageLeft, TOXIC_DAMAGE);
		m_iMaxToxicPoints = m_iDamageLeft;
		break;
	}
	case 1:
	{
		DevMsg("setting fire blocker points to %i\n", m_iPoints);
		pPlayer->SetBlockerDamageLeft(m_iDamageLeft, FIRE_DAMAGE);
		m_iMaxFirePoints = m_iDamageLeft;
		break;
	}
	case 2:
	{
		DevMsg("setting electric blocker points to %i\n", m_iPoints);
		pPlayer->SetBlockerDamageLeft(m_iDamageLeft, ELECTRIC_DAMAGE);
		break;
	}
	default:
		break;
	}
	if (!pPlayer->HasBlocker())
	{
		DevMsg("damage blocker not enabled, enabling\n");
		pPlayer->ToggleDamageBlocker();
	}
}
void CHLREnvironmentResistance::Think(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	switch (m_iDamageType)
	{
	case 0:
	{
		
		m_iDamageLeft = pPlayer->GetBlockerDamageLeft(TOXIC_DAMAGE);
		DevMsg("retrieving %i toxic points\n", m_iDamageLeft);
		break;
	}
	case 1:
	{
		m_iDamageLeft = pPlayer->GetBlockerDamageLeft(FIRE_DAMAGE);
		DevMsg("retrieving %i fire points\n", m_iDamageLeft);
		break;
	}
	case 2:
	{
		m_iDamageLeft = pPlayer->GetBlockerDamageLeft(ELECTRIC_DAMAGE);
		DevMsg("retrieving %i electric points\n", m_iDamageLeft);
		break;
	}
	default:
		break;
	}
	if (m_iDamageLeft < 0)
	{
		SetThink(NULL);
		m_bDrawHud = false;
		Kill();
		return;
	}
	SetNextThink(gpGlobals->curtime + 0.5f);
}
void CHLREnvironmentResistance::Kill(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	pPlayer->ToggleDamageBlocker();
	RemoveDeferred();
}