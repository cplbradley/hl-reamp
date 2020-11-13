#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "convar.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "explode.h"
#include "physics_shared.h"
#include "game.h"
#include "movevars_shared.h"
#include "vstdlib/random.h"
#include "hl2_shareddefs.h"
#include "gamemovement.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"
#include "particle_parse.h"
#include "particle_system.h"

#include "tier0/memdbgon.h"

extern IGameMovement *g_pGameMovement;
#define POWERUP_MODEL "models/items/powerup.mdl"

class CHLRFreemanFury : public CBaseAnimating
{
	DECLARE_CLASS(CHLRFreemanFury, CBaseAnimating);
public:
	void	Spawn(void);
	void	Precache(void);
	void	Touch(CBaseEntity *pOther);

	CHandle< CParticleSystem >	m_hSpitEffect;


	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_freemanfury, CHLRFreemanFury); //ooh i'm being assigned a class
BEGIN_DATADESC(CHLRFreemanFury) //let's store some data
// Function Pointers
DEFINE_FUNCTION(Touch), //i can touch stuff, i need outputs
END_DATADESC() //all done!

void CHLRFreemanFury::Spawn(void)
{
	Precache();
	SetModel(POWERUP_MODEL);
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetRenderColor(200, 55, 0);
	SetMoveType(MOVETYPE_NONE);
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->KeyValue("effect_name", "powerup_ff");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}
	SetLocalAngularVelocity(QAngle(0, 10, 0));
	SetTouch(&CHLRFreemanFury::Touch);
	
}
void CHLRFreemanFury::Precache(void)
{
	PrecacheModel(POWERUP_MODEL);
	UTIL_PrecacheOther("weapon_furybar");
	PrecacheParticleSystem("powerup_ff");
	PrecacheScriptSound("Powerup.Pickup");
}
void CHLRFreemanFury::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		pPlayer->GiveNamedItem("weapon_furybar");
		RemoveDeferred();
	}
}
////////////////////////////////////////////
////////////////////////////////////////////
/////////////////////////////////////////////
class CHLRQuadJump : public CBaseAnimating
{
	DECLARE_CLASS(CHLRQuadJump, CBaseAnimating);
public:
	void	Spawn(void);
	void	Precache(void);
	void	TimerThink(void);
	void	Touch(CBaseEntity *pOther);
	void	Kill(void);

	float	m_fTimerDelay;
	CHandle< CParticleSystem >	m_hSpitEffect;


	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_quadjump, CHLRQuadJump); //ooh i'm being assigned a class
BEGIN_DATADESC(CHLRQuadJump) //let's store some data
// Function Pointers
DEFINE_FUNCTION(Touch),
DEFINE_THINKFUNC(TimerThink),
DEFINE_THINKFUNC(Kill),//i can touch stuff, i need outputs
END_DATADESC() //all done!

void CHLRQuadJump::Spawn(void)
{
	Precache();
	SetModel(POWERUP_MODEL);
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetRenderColor(230, 195, 0);
	SetMoveType(MOVETYPE_NONE);
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->KeyValue("effect_name", "powerup_qj");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}
	SetLocalAngularVelocity(QAngle(0, 10, 0));
	SetTouch(&CHLRQuadJump::Touch);

}
void CHLRQuadJump::Precache(void)
{
	PrecacheModel(POWERUP_MODEL);
	PrecacheParticleSystem("powerup_qj");
	PrecacheScriptSound("Powerup.Pickup");
}
void CHLRQuadJump::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		
		CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
		color32 gold = { 230, 190, 0, 175 };
		SetTouch(NULL);
		SetSolid(SOLID_NONE);
		RemoveSolidFlags(FSOLID_TRIGGER);
		UTIL_ScreenFade(pOther, gold, 0.5, 0, FFADE_IN);
		gm->m_iMaxJumps = 4;
		m_hSpitEffect->StopParticleSystem();
		m_fTimerDelay = gpGlobals->curtime + 30.0f;
		SetThink(&CHLRQuadJump::TimerThink);
		SetNextThink(gpGlobals->curtime);
	}
}
void CHLRQuadJump::TimerThink(void)
{
	CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
	if (gpGlobals->curtime >= m_fTimerDelay)
	{
		RemoveDeferred();
		gm->m_iMaxJumps = 2;
		SetThink(&CHLRQuadJump::Kill);
	}
	else
	{
		SetNextThink(gpGlobals->curtime + 0.1f);
	}
}
void CHLRQuadJump::Kill(void)
{
	UTIL_Remove(this);
}
//////////////////////////////////
//////////////////////////////////
//////////////////////////////////
class CHLRTripleDamage : public CBaseAnimating
{
	DECLARE_CLASS(CHLRTripleDamage, CBaseAnimating);
public:
	void	Spawn(void);
	void	Precache(void);
	void	TimerThink(void);
	void	Touch(CBaseEntity *pOther);
	void	Kill(void);

	float	m_fTimerDelay;
	CHandle< CParticleSystem >	m_hSpitEffect;


	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_tripledamage, CHLRTripleDamage); //ooh i'm being assigned a class
BEGIN_DATADESC(CHLRTripleDamage) //let's store some data
// Function Pointers
DEFINE_FUNCTION(Touch),
DEFINE_THINKFUNC(TimerThink),
DEFINE_THINKFUNC(Kill),//i can touch stuff, i need outputs
END_DATADESC() //all done!

void CHLRTripleDamage::Spawn(void)
{
	Precache();
	SetModel(POWERUP_MODEL);
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetRenderColor(150, 0, 255);
	SetMoveType(MOVETYPE_NONE);
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->KeyValue("effect_name", "powerup_td");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}
	SetLocalAngularVelocity(QAngle(0, 10, 0));
	SetTouch(&CHLRQuadJump::Touch);

}
void CHLRTripleDamage::Precache(void)
{
	PrecacheModel(POWERUP_MODEL);
	PrecacheParticleSystem("powerup_td");
	PrecacheScriptSound("Powerup.Pickup");
}
void CHLRTripleDamage::Touch(CBaseEntity *pOther)
{
	
	if (pOther->IsPlayer())
	{
		ConVar *sk_quaddmg_scale = cvar->FindVar("sk_quaddmg_scale");
		sk_quaddmg_scale->SetValue(3);
		SetSolid(SOLID_NONE);
		RemoveSolidFlags(FSOLID_TRIGGER);
		m_hSpitEffect->StopParticleSystem();
		color32 purple = { 150, 0, 255, 175 };
		UTIL_ScreenFade(pOther, purple, 0.5, 0, FFADE_IN);
		AddFlag(EF_NODRAW);
		m_fTimerDelay = gpGlobals->curtime + 30.0f;
		SetThink(&CHLRQuadJump::TimerThink);
		SetNextThink(gpGlobals->curtime);

	}
}
void CHLRTripleDamage::TimerThink(void)
{
	if (gpGlobals->curtime >= m_fTimerDelay)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		ConVar *sk_quaddmg_scale = cvar->FindVar("sk_quaddmg_scale");
		sk_quaddmg_scale->SetValue(1);
		SetThink(&CHLRQuadJump::Kill);
		SetNextThink(gpGlobals->curtime + 0.1f);
		color32 purple = { 150, 0, 255, 175 };
		UTIL_ScreenFade(pPlayer, purple, 0.5, 0, FFADE_IN);
	}
	else
	{
		SetNextThink(gpGlobals->curtime + 0.1f);
	}
}
void CHLRTripleDamage::Kill(void)
{
	UTIL_Remove(this);
}

class CHLRFreqShifter : public CBaseAnimating
{
	DECLARE_CLASS(CHLRFreqShifter, CBaseAnimating);
public:
	void Precache(void);
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
	void Kill(void);
	float m_fTimerDelay;

	void ActiveThink(void);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_freqshifter, CHLRFreqShifter);


BEGIN_DATADESC(CHLRFreqShifter)
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(Kill),
DEFINE_THINKFUNC(ActiveThink),
END_DATADESC()

void CHLRFreqShifter::Precache(void)
{
	PrecacheModel(POWERUP_MODEL);
	PrecacheScriptSound("FreqShifter.Voice");
}
void CHLRFreqShifter::Spawn(void)
{
	Precache();
	SetModel(POWERUP_MODEL);
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetRenderColor(0, 255, 150);
	SetMoveType(MOVETYPE_CUSTOM);
	//m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	/*if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->KeyValue("effect_name", "powerup_td");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}*/
	SetLocalAngularVelocity(QAngle(0, 10, 0));
	SetTouch(&CHLRFreqShifter::Touch);
}

void CHLRFreqShifter::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		SetSolid(SOLID_NONE);
		RemoveSolidFlags(FSOLID_TRIGGER);
		//m_hSpitEffect->StopParticleSystem();
		color32 purple = { 150, 0, 255, 175 };
		UTIL_ScreenFade(pOther, purple, 0.5, 0, FFADE_IN);
		SetModelName(NULL_STRING);
		m_fTimerDelay = gpGlobals->curtime + 9.0f;
		SetThink(&CHLRFreqShifter::ActiveThink);
		SetNextThink(gpGlobals->curtime);
		EmitSound("FreqShifter.Voice");

	}
}
void CHLRFreqShifter::ActiveThink(void)
{
	ConVar *host_timescale = cvar->FindVar("host_timescale");
	ConVar *sv_cheats = cvar->FindVar("sv_cheats");
	if (gpGlobals->curtime < m_fTimerDelay)
	{
		if (host_timescale->GetFloat() != 0.3f)
		{
			sv_cheats->SetValue(1);
			host_timescale->SetValue(0.3f);
		}
		SetNextThink(gpGlobals->curtime + 0.01f);
	}
	else
	{
		sv_cheats->SetValue(0);
		SetThink(NULL);
		Kill();
		return;
	}
}

void CHLRFreqShifter::Kill(void)
{
	RemoveDeferred();
}


class CHLRGravShifter : public CBaseAnimating
{
	DECLARE_CLASS(CHLRGravShifter, CBaseAnimating);
public:
	void Precache(void);
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
	void Kill(void);
	float m_fTimerDelay;

	void ResetGrav(void);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_gravshifter, CHLRGravShifter);


BEGIN_DATADESC(CHLRGravShifter)
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(Kill),
DEFINE_THINKFUNC(ResetGrav),
END_DATADESC()

void CHLRGravShifter::Precache(void)
{
	PrecacheModel(POWERUP_MODEL);
	PrecacheScriptSound("FreqShifter.Voice");
}
void CHLRGravShifter::Spawn(void)
{
	Precache();
	SetModel(POWERUP_MODEL);
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetRenderColor(0, 255, 150);
	SetMoveType(MOVETYPE_CUSTOM);
	//m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	/*if (m_hSpitEffect != NULL)
	{
	// Setup our basic parameters
	m_hSpitEffect->KeyValue("start_active", "1");
	m_hSpitEffect->KeyValue("effect_name", "powerup_td");
	m_hSpitEffect->SetParent(this);
	m_hSpitEffect->SetLocalOrigin(vec3_origin);
	DispatchSpawn(m_hSpitEffect);
	if (gpGlobals->curtime > 0.5f)
	m_hSpitEffect->Activate();
	}*/
	SetLocalAngularVelocity(QAngle(0, 10, 0));
	SetTouch(&CHLRFreqShifter::Touch);
}

void CHLRGravShifter::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		SetSolid(SOLID_NONE);
		RemoveSolidFlags(FSOLID_TRIGGER);
		//m_hSpitEffect->StopParticleSystem();
		SetModelName(NULL_STRING);
		IPhysicsEnvironment	*physenv = physics->GetActiveEnvironmentByIndex(0);
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		//IPhysicsObject *pVPhysics = pPlayer->VPhysicsGetObject();
		QAngle angView = pPlayer->EyeAngles();
		angView[ROLL] += 90.f;
		pPlayer->SnapEyeAngles(angView);

		assert(physenv);
		physenv->SetGravity(Vector(0, 0, GetCurrentGravity()));
		SetThink(&CHLRGravShifter::ResetGrav);
		SetNextThink(gpGlobals->curtime + 30.0f);
	}
}
void CHLRGravShifter::ResetGrav(void)
{
	IPhysicsEnvironment	*physenv = physics->GetActiveEnvironmentByIndex(0);
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	pPlayer->SetAbsAngles(-GetAbsAngles());
	QAngle angView = pPlayer->EyeAngles();
	angView[ROLL] += 720.f;

	pPlayer->SnapEyeAngles(angView);
	Kill();
}

void CHLRGravShifter::Kill(void)
{
	RemoveDeferred();
}