#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "soundent.h"
#include "hl2_player.h"
#include "hl2_shareddefs.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"	

#include "particle_system.h"

#include "tier0/memdbgon.h"

#define MEGAMELON_MODEL "models/props_junk/watermelon01.mdl"

class CHLRMegaMelon : public CBaseAnimating
{
	DECLARE_CLASS(CHLRMegaMelon,CBaseAnimating)
public:
	void Spawn(void);
	void Precache(void);
	void Touch(CBaseEntity *pOther);
	void Destroy(void);
	CHandle< CParticleSystem >	m_hSpitEffect;
	DECLARE_DATADESC()
};

LINK_ENTITY_TO_CLASS(hlr_megamelon, CHLRMegaMelon);

BEGIN_DATADESC(CHLRMegaMelon)
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(Destroy),
END_DATADESC();

void CHLRMegaMelon::Spawn(void)
{
	Precache();
	SetModel(MEGAMELON_MODEL);
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_NONE);
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetModelScale(1.5f);
	SetTouch(&CHLRMegaMelon::Touch);
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->KeyValue("effect_name", "hlr_megamelon_core");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}
}

void CHLRMegaMelon::Precache(void)
{
	PrecacheModel(MEGAMELON_MODEL);
	PrecacheParticleSystem("hlr_megamelon_core");
	PrecacheScriptSound("Flesh_Bloody.ImpactHard");
}
void CHLRMegaMelon::Touch(CBaseEntity *pOther)
{

	if (pOther->IsPlayer())
	{
		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(pOther);
		color32 teal = { 0, 200, 120, 175 };
		SetTouch(NULL);
		SetSolid(SOLID_NONE);
		RemoveSolidFlags(FSOLID_TRIGGER);
		pPlayer->SetArmorValue(200);
		pPlayer->SetHealth(200);
		UTIL_ScreenFade(pPlayer, teal, 0.5, 0, FFADE_IN);
		EmitSound("Flesh_Bloody.ImpactHard");
		Destroy();
		
	}
	SetNextThink(gpGlobals->curtime + 0.5f);
}
void CHLRMegaMelon::Destroy(void)
{
	RemoveDeferred();
}