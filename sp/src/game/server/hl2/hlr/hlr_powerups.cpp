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
#define FF_MODEL "models/items/powerup_fury.mdl"
#define POWERUP_TIMER_SOUND "Powerup.Timer"
#define POWERUP_IDLE_SOUND "Powerup.Idle"

class CHLRFreemanFury : public CBaseAnimating
{
	DECLARE_CLASS(CHLRFreemanFury, CBaseAnimating);
	DECLARE_SERVERCLASS();
public:
	void	Spawn(void);
	void	Precache(void);
	void	Touch(CBaseEntity *pOther);
	void	UpdateTimer(void);
	void	Kill(void);
	void	SetAnimation(void);

	CNetworkVar(bool, m_bDrawHud);
	CNetworkVar(int, m_iTimeLeft);

	CHandle< CParticleSystem >	m_hSpitEffect;


	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_freemanfury, CHLRFreemanFury); //ooh i'm being assigned a class
BEGIN_DATADESC(CHLRFreemanFury) //let's store some data
// Function Pointers
DEFINE_FUNCTION(Touch), //i can touch stuff, i need outputs
DEFINE_FIELD(m_bDrawHud,FIELD_BOOLEAN),
DEFINE_FIELD(m_iTimeLeft,FIELD_INTEGER),
END_DATADESC() //all done!
IMPLEMENT_SERVERCLASS_ST(CHLRFreemanFury,DT_FreemanFury)
SendPropBool(SENDINFO(m_bDrawHud)),
SendPropInt(SENDINFO(m_iTimeLeft)),
END_SEND_TABLE()


void CHLRFreemanFury::Spawn(void)
{
	Precache();
	SetModel(FF_MODEL);
	SetModelScale(1.25f);
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	//SetRenderColor(200, 55, 0);
	SetMoveType(MOVETYPE_NONE);
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	RegisterThinkContext("timercontext");
	m_iTimeLeft = 30;
	if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->KeyValue("effect_name", "powerup_ff_new_core");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}
	SetAnimation();
	SetTouch(&CHLRFreemanFury::Touch);
	
}
void CHLRFreemanFury::SetAnimation(void)
{
//	int iSequence = LookupSequence("loop");
	EmitSound(POWERUP_IDLE_SOUND);
	//SetPlaybackRate(1.0f);
}
void CHLRFreemanFury::Precache(void)
{
	PrecacheModel(FF_MODEL);
	UTIL_PrecacheOther("weapon_furybar");
	PrecacheParticleSystem("powerup_ff_new_core");
	PrecacheScriptSound("Powerup.Pickup");
	PrecacheScriptSound(POWERUP_TIMER_SOUND);
	PrecacheScriptSound(POWERUP_IDLE_SOUND);
}
void CHLRFreemanFury::UpdateTimer(void)
{
	m_iTimeLeft--;
	SetNextThink(gpGlobals->curtime + 1, "timercontext");
	DevMsg("serverside timeleft = %i", m_iTimeLeft);
	if (m_iTimeLeft <= 3)
		EmitSound(POWERUP_TIMER_SOUND);
	if (m_iTimeLeft <= 0)
	{
		SetThink(NULL);
		SetContextThink(NULL, gpGlobals->curtime, "timercontext");
		Kill();
	}
}
void CHLRFreemanFury::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		StopSound(POWERUP_IDLE_SOUND);
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		pPlayer->GiveNamedItem("weapon_furybar");
		AddEFlags(EF_NODRAW);
		SetThink(&CHLRFreemanFury::Kill);
		SetTouch(NULL);
		SetSolid(SOLID_NONE);
		m_iTimeLeft = 30;
		SetNextThink(gpGlobals->curtime + 30.0f);
		m_bDrawHud = true;
		if (m_hSpitEffect)
			m_hSpitEffect->StopParticleSystem();
		DevMsg("Serverside bool is true\n");
		
		SetContextThink(&CHLRFreemanFury::UpdateTimer, gpGlobals->curtime + 1, "timercontext");
	}
}
void CHLRFreemanFury::Kill(void)
{
	m_bDrawHud = false;
	RemoveDeferred();
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
	void	ActivateEffects(void);
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

	ActivateEffects();
	
	SetTouch(&CHLRQuadJump::Touch);

}
void CHLRQuadJump::ActivateEffects(void)
{
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
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
//		CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
		color32 gold = { 230, 190, 0, 175 };
		SetTouch(NULL);
		SetSolid(SOLID_NONE);
		RemoveSolidFlags(FSOLID_TRIGGER);
		UTIL_ScreenFade(pOther, gold, 0.5, 0, FFADE_IN);
		//gm->m_iMaxJumps = 4;
		if (m_hSpitEffect)
			m_hSpitEffect->StopParticleSystem();
		pPlayer->m_bQuadJump = true;
		m_fTimerDelay = gpGlobals->curtime + 30.0f;
		SetThink(&CHLRQuadJump::TimerThink);
		SetNextThink(gpGlobals->curtime);
	}
}
void CHLRQuadJump::TimerThink(void)
{
	//CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
	if (gpGlobals->curtime >= m_fTimerDelay)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		pPlayer->m_bQuadJump = false;
		RemoveDeferred();
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
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		pPlayer->m_bTripleDamage = true;
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
		pPlayer->m_bTripleDamage = false;
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
	//CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
	ConVar *host_timescale = cvar->FindVar("host_timescale");
	ConVar *sv_cheats = cvar->FindVar("sv_cheats");
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (gpGlobals->curtime < m_fTimerDelay)
	{
		if (host_timescale->GetFloat() != 0.3f)
		{
			sv_cheats->SetValue(1);
			host_timescale->SetValue(0.3f);
			pPlayer->SetMaxSpeed(1500.0f);
		}
		SetNextThink(gpGlobals->curtime + 0.01f);
	}
	else
	{
		host_timescale->SetValue(1.0f);
		pPlayer->SetMaxSpeed(450.0f);
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
		angView[ROLL] += 180.f;
		pPlayer->SnapEyeAngles(angView);
		pPlayer->SetAbsAngles(-GetAbsAngles());
		Vector vOffset = pPlayer->GetViewOffset();
		pPlayer->SetViewOffset(-vOffset);
		assert(physenv);
		physenv->SetGravity(Vector(0, 0, GetCurrentGravity()));
		pPlayer->SetGravity(-1);
		pPlayer->SetGroundEntity(NULL);
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
	angView[ROLL] += 180.0f;

	pPlayer->SnapEyeAngles(angView);
	Kill();
}

void CHLRGravShifter::Kill(void)
{
	RemoveDeferred();
}




class CHLROverdrive : public CBaseAnimating
{
	DECLARE_CLASS(CHLROverdrive, CBaseAnimating);
	DECLARE_SERVERCLASS();
public:
	void Precache(void);
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
	void Kill(void);
	void UpdateTimer(void);
	float m_fTimerDelay;
	CHandle< CParticleSystem >	m_hSpitEffect;
	void Deactivate(void);

	CNetworkVar(bool, m_bDrawHud);
	CNetworkVar(int, m_iTimeLeft);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_overdrive, CHLROverdrive);


BEGIN_DATADESC(CHLROverdrive)
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(Kill),
DEFINE_FUNCTION(Deactivate),
DEFINE_FIELD(m_hSpitEffect, FIELD_EHANDLE),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CHLROverdrive, DT_Overdrive)
SendPropBool(SENDINFO(m_bDrawHud)),
SendPropInt(SENDINFO(m_iTimeLeft)),
END_SEND_TABLE()


void CHLROverdrive::Precache(void)
{
	PrecacheModel(POWERUP_MODEL);
}
void CHLROverdrive::Spawn(void)
{
	Precache();
	SetModel(POWERUP_MODEL);
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetRenderColor(255, 120, 0);
	SetMoveType(MOVETYPE_CUSTOM);
	RegisterThinkContext("timercontext");
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect != NULL)
	{
	// Setup our basic parameters
	m_hSpitEffect->KeyValue("start_active", "1");
	m_hSpitEffect->KeyValue("effect_name", "powerup_od");
	m_hSpitEffect->SetParent(this);
	m_hSpitEffect->SetLocalOrigin(vec3_origin);
	DispatchSpawn(m_hSpitEffect);
	if (gpGlobals->curtime > 0.5f)
	m_hSpitEffect->Activate();
	}
	SetLocalAngularVelocity(QAngle(0, 10, 0));
	SetTouch(&CHLROverdrive::Touch);
	m_bDrawHud = false;
}

void CHLROverdrive::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		SetSolid(SOLID_NONE);
		RemoveSolidFlags(FSOLID_TRIGGER);
		SetTouch(NULL);
		if (m_hSpitEffect)
			m_hSpitEffect->StopParticleSystem();
		SetModelName(NULL_STRING);
		color32 orange = { 255, 120, 0, 175 };
		UTIL_ScreenFade(pOther, orange, 0.5, 0, FFADE_IN);
		pPlayer->m_bOverdrive = true;
		m_iTimeLeft = 30;
		m_bDrawHud = true;
		DevMsg("Serverside bool is true\n");

		SetContextThink(&CHLROverdrive::UpdateTimer, gpGlobals->curtime + 1, "timercontext");
	}
}
void CHLROverdrive::Deactivate(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	pPlayer->m_bOverdrive = false;
	m_bDrawHud = false;
	color32 orange = { 255, 120, 0, 175 };
	UTIL_ScreenFade(pPlayer, orange, 0.5, 0, FFADE_IN);
	Kill();
	SetThink(NULL);
}

void CHLROverdrive::Kill(void)
{
	RemoveDeferred();
}
void CHLROverdrive::UpdateTimer(void)
{
	m_iTimeLeft--;
	SetNextThink(gpGlobals->curtime + 1, "timercontext");
	DevMsg("serverside timeleft = %i", m_iTimeLeft);
	if (m_iTimeLeft <= 0)
	{
		SetThink(NULL);
		SetContextThink(NULL, gpGlobals->curtime, "timercontext");
		Deactivate();
	}
}