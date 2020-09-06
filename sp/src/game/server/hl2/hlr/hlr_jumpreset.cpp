#include "cbase.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "gamemovement.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "particle_parse.h"
#include "sprite.h"
#include "hl2_gamerules.h"
#include "movevars_shared.h"
#include "gamestats.h"

#include "tier0/memdbgon.h"


class CJumpReset : public CBaseAnimating
{
	DECLARE_CLASS(CJumpReset, CBaseAnimating);
public:
	void Spawn(void);
	void Precache(void);
	void Reenable(void);
	void TouchThink(CBaseEntity *pOther);
	unsigned int PhysicsSolidMaskForEntity() const;
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(hlr_jumpreset, CJumpReset);
BEGIN_DATADESC(CJumpReset)
// Function Pointers
DEFINE_THINKFUNC(Reenable),
DEFINE_FUNCTION(TouchThink),
END_DATADESC()

#define ORB_MODEL "models/spitball_large.mdl"
unsigned int CJumpReset::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CJumpReset::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetModel(ORB_MODEL);
	SetRenderColor(255, 215, 0);
	SetModelScale(2.0f);
	SetTouch(&CJumpReset::TouchThink);
	DispatchParticleEffect("jumpreset_core", GetAbsOrigin(), GetAbsAngles(), this);
}
void CJumpReset::Precache(void)
{
	PrecacheModel(ORB_MODEL);
	PrecacheScriptSound("Jump_Reset.Single");
	PrecacheParticleSystem("jumpreset_core");
}
void CJumpReset::TouchThink(CBaseEntity *pOther) //something touched me
{
	if (pOther->IsPlayer())
	{
		extern IGameMovement *g_pGameMovement;
		CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
		gm->m_iJumpCount = 0;
		SetTouch(NULL);
		EmitSound("Jump_Reset.Single");
		AddEffects(EF_NODRAW);
		SetThink(&CJumpReset::Reenable);
		SetNextThink(gpGlobals->curtime + 15.0f);
		StopParticleEffects(this);
	}
}
void CJumpReset::Reenable(void)
{
	SetTouch(&CJumpReset::TouchThink);
	RemoveEffects(EF_NODRAW);
	DispatchParticleEffect("jumpreset_core", GetAbsOrigin(), GetAbsAngles(), this);
}