//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "hl2_shareddefs.h"
#include "ai_memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "SpriteTrail.h"
#include "hlr/hlr_shareddefs.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#define PLASMA_MODEL "models/spitball_small.mdl"
#define PLASMA_MODEL_NPC "models/spitball_medium.mdl"
#define PLASMA_SPEED 8000
extern ConVar    sk_plr_dmg_smg1_grenade;
extern ConVar	sk_plr_dmg_smg1;
extern ConVar	 sk_npc_dmg_smg1;
ConVar sk_plasmarifle_firerate("sk_plasmarifle_firerate", "0.09");

class CPlasmaBall : public CBaseCombatCharacter
{
	DECLARE_CLASS(CPlasmaBall, CBaseCombatCharacter);
public:
	void	Spawn(void);
	void	Precache(void);
	void	ItemPostFrame(void);
	void	PlasmaTouch(CBaseEntity *pOther);
	void	KillIt(void);
	void  MoveTowardsTarget(void);
	void  SetTargetPos(const Vector &vecTargetpos, const float &fVelocity);
	void Unhide(void);

	float flVelocity;
	Vector vecTarget;
	unsigned int PhysicsSolidMaskForEntity() const;


	static CPlasmaBall *Create(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL);
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	bool	CreateTrail(void);
	bool	DrawSprite(void);
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(plasma_ball, CPlasmaBall);
BEGIN_DATADESC(CPlasmaBall)
// Function Pointers
DEFINE_FUNCTION(PlasmaTouch),
DEFINE_THINKFUNC(Unhide),
END_DATADESC()
CPlasmaBall *CPlasmaBall::Create(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner)
{
	// Create a new entity with CPlasmaBall data
	CPlasmaBall *pBall = (CPlasmaBall *)CreateEntityByName("plasma_ball");
	UTIL_SetOrigin(pBall, vecOrigin);
	pBall->SetAbsAngles(angAngles);
	pBall->Spawn();
	pBall->SetOwnerEntity(pentOwner);

	return pBall;
}
unsigned int CPlasmaBall::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CPlasmaBall::Spawn(void)
{
	Precache();
	SetMoveType(MOVETYPE_FLY);
	UTIL_SetSize(this, -Vector(1.0f, 1.0f, 1.0f), Vector(1.0f, 1.0f, 1.0f));
	SetSolid(SOLID_CUSTOM);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	//SetSolidFlags(FSOLID_TRIGGER);
	CreateTrail();
	AddEffects(EF_NODRAW);
	SetThink(&CPlasmaBall::Unhide);
	SetNextThink(gpGlobals->curtime + 0.03f);
	SetTouch(&CPlasmaBall::PlasmaTouch);
}
void CPlasmaBall::Precache(void)
{
	PrecacheModel(PLASMA_MODEL);
	PrecacheModel(PLASMA_MODEL_NPC);
	PrecacheParticleSystem("smg_plasmaball_core");
	PrecacheParticleSystem("smg_plasmaball_core_red");
	PrecacheModel("sprites/smoke.vmt");
	PrecacheModel("sprites/physcannon_bluecore2b.vmt");

}
void CPlasmaBall::Unhide(void)
{
	RemoveEffects(EF_NODRAW);
	SetThink(NULL);
}
bool CPlasmaBall::CreateTrail(void)
{
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/smoke.vmt", GetLocalOrigin(), false);
	if (m_pGlowTrail != NULL)
	{
		m_pGlowTrail->FollowEntity(this);
		m_pGlowTrail->SetStartWidth(6.0f);
		m_pGlowTrail->SetEndWidth(0.2f);
		m_pGlowTrail->SetLifeTime(0.05f);
	}
	return true;
}
bool CPlasmaBall::DrawSprite(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	m_pMainGlow = CSprite::SpriteCreate("sprites/physcannon_bluecore2b.vmt", GetLocalOrigin(), false);
	if (m_pMainGlow != NULL)
	{
		m_pMainGlow->FollowEntity(this);
		if (pPlayer->HasOverdrive())
			m_pMainGlow->SetTransparency(kRenderGlow, 255, 0, 0, 200, kRenderFxNoDissipation);
		else
			m_pMainGlow->SetTransparency(kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation);
		m_pMainGlow->SetScale(0.3f);
		m_pMainGlow->SetGlowProxySize(4.0f);
	}
	return true;
}
void CPlasmaBall::PlasmaTouch(CBaseEntity *pOther) //i touched something
{
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
			return;
	}
	if (pOther->IsPlayer())
		return;

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	if (pPlayer->HasOverdrive())
		DispatchParticleEffect("smg_plasmaball_core_red", GetAbsOrigin(), GetAbsAngles(), this); //poof effect!
	else
		DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this); //poof effect!

		if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
		{
			trace_t	tr; //initialize info
			tr = BaseClass::GetTouchTrace(); //trace touch
			Vector	vecNormalizedVel = GetAbsVelocity();

			ClearMultiDamage();
			VectorNormalize(vecNormalizedVel);
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_smg1.GetFloat(), DMG_SHOCK);
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 10.0f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
			ApplyMultiDamage();
		}
		SetMoveType(MOVETYPE_NONE);
		SetThink(&CPlasmaBall::KillIt); //schedule remove command
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime + 0.01f); //execute remove command after 0.01 seconds, this allows time to trigger the particle
	//}
}
void CPlasmaBall::SetTargetPos(const Vector &vecTargetpos, const float &fVelocity)
{
	vecTarget = vecTargetpos;
	flVelocity = fVelocity;
	MoveTowardsTarget();
}
void CPlasmaBall::MoveTowardsTarget(void)
{
	Vector vecDir = (vecTarget - GetAbsOrigin());
	VectorNormalize(vecDir);
	SetAbsVelocity(vecDir * flVelocity);
}
void CPlasmaBall::KillIt(void)
{
	SetThink(NULL);
	m_pGlowTrail == NULL;
	UTIL_Remove(this); //remove it
}
void CPlasmaBall::ItemPostFrame(void)
{
	if (GetAbsVelocity() == 0)
	{
		KillIt();
	}
}


class CWeaponPlasmaRifle : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponPlasmaRifle, CHLSelectFireMachineGun);

	CWeaponPlasmaRifle();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	AddViewKick(void);
	void	SecondaryAttack(void);
	void	PrimaryAttack(void);
	void	ItemPostFrame(void);
	void ItemHolsterFrame(void);

	void	Overheat(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo);

	int		GetMinBurst() { return 5; }
	int		GetMaxBurst() { return 5; }

	void AllowShoot();

	bool bCanShootAgain;
	//int		m_iEnergyLevel;
	CNetworkVar(bool, bActive);
	CNetworkVar(int, iShots);

	/*bool bSecondaryPressed;
	CNetworkVar(bool,bShootingSecondary);
	CNetworkVar(Vector, vecBeamEnd);
	float fSecondaryActivationTime;*/

	virtual void Equip(CBaseCombatCharacter *pOwner);
	bool	Reload(void);

	float	GetFireRate(void) { return 0.15f; }	// 13.3hz
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition(float flDot, float flDist);
	Activity	GetPrimaryAttackActivity(void);

	virtual const Vector& GetBulletSpread(void)
	{
		static const Vector cone = VECTOR_CONE_2DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

protected:

	float fDrainTime;

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	CSoundPatch* m_pAlarm;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponPlasmaRifle, DT_WeaponPlasmaRifle)
SendPropBool(SENDINFO(bActive)),
SendPropInt(SENDINFO(iShots)),
//SendPropVector(SENDINFO(vecBeamEnd),-1, SPROP_COORD),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_plasmarifle, CWeaponPlasmaRifle);
PRECACHE_WEAPON_REGISTER(weapon_plasmarifle);

BEGIN_DATADESC(CWeaponPlasmaRifle)

DEFINE_FIELD(m_vecTossVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_flNextGrenadeCheck, FIELD_TIME),
DEFINE_FIELD(bActive,FIELD_BOOLEAN),
DEFINE_FIELD(iShots,FIELD_INTEGER),
DEFINE_FIELD(fDrainTime,FIELD_FLOAT),
DEFINE_FIELD(bCanShootAgain,FIELD_BOOLEAN),

END_DATADESC()

acttable_t	CWeaponPlasmaRifle::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },


	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SMG1, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SMG1, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SMG1, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, false },
};

IMPLEMENT_ACTTABLE(CWeaponPlasmaRifle);

//=========================================================
CWeaponPlasmaRifle::CWeaponPlasmaRifle()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 9600;

	m_bAltFiresUnderwater = false;

	bCanShootAgain = true;

	//m_iClip1 = 50;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::Precache(void)
{
	//UTIL_PrecacheOther("grenade_ar2");
	UTIL_PrecacheOther("plasma_ball");
	PrecacheScriptSound("NPC_CombineBall.Explosion");
	PrecacheParticleSystem("plasma_altfire_core");
	PrecacheParticleSystem("smg_core");
	PrecacheParticleSystem("smg_core_red");
	UTIL_PrecacheOther("hlr_stunbomb");
	PrecacheParticleSystem("plasmarifle_altfire_beam");
	PrecacheParticleSystem("plasmarifle_overheat");
	PrecacheScriptSound("PlasmaRifle.Overheat");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::Equip(CBaseCombatCharacter *pOwner)
{
	if (pOwner->Classify() == CLASS_PLAYER_ALLY)
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 2500;
	}

	BaseClass::Equip(pOwner);
}

//-----------------------------------------------------------------------------
// Purpose: fires the projectile
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSound(SINGLE_NPC);//set up sound
	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());//emit with properties
	pOperator->FireBullets(0, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);//don't actually fire a bullet, but register a bullet as being fired
	QAngle angAiming;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angAiming);
	VectorAngles(vecShootDir, angAiming);
	CPlasmaBall *pBall = CPlasmaBall::Create(vecShootOrigin, angAiming, pOperator); //create plasmaball
	pBall->SetModel(PLASMA_MODEL_NPC);
	pBall->SetAbsVelocity(vecShootDir * 1200);
	pBall->m_pGlowTrail->SetTransparency(kRenderTransAdd, 255, 45, 45, 100, kRenderFxNone);
	pOperator->DoMuzzleFlash(); //emit muzzle flash
	m_iClip1 -= 1;  //remove 1 bullet from clip
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
		{
			vecShootOrigin = pOperator->Weapon_ShootPosition();
		}

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
		QAngle angAiming;
		GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angAiming);
		VectorAngles(vecShootDir, angAiming);
		CPlasmaBall *pBall = CPlasmaBall::Create(vecShootOrigin, angAiming, pOperator);
		pBall->SetModel(PLASMA_MODEL_NPC);
		pBall->SetAbsVelocity(vecShootDir * 1500);		pBall->m_pGlowTrail->SetTransparency(kRenderTransAdd, 255, 45, 45, 200, kRenderFxNone);
		pBall->SetLocalAngularVelocity(
			QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		pOperator->DoMuzzleFlash();
		//FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
	}
	break;

	/*//FIXME: Re-enable
	case EVENT_WEAPON_AR2_GRENADE:
	{
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	vecShootDir = npc->GetShootEnemyDir( vecShootOrigin );

	Vector vecThrow = m_vecTossVelocity;

	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecShootOrigin, vec3_angle, npc );
	pGrenade->SetAbsVelocity( vecThrow );
	pGrenade->SetLocalAngularVelocity( QAngle( 0, 400, 0 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY );
	pGrenade->m_hOwner			= npc;
	pGrenade->m_pMyWeaponAR2	= this;
	pGrenade->SetDamage(sk_npc_dmg_ar2_grenade.GetFloat());

	// FIXME: arrgg ,this is hard coded into the weapon???
	m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.

	m_iClip2--;
	}
	break;
	*/

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponPlasmaRifle::GetPrimaryAttackActivity(void)
{
	if (m_nShotsFired < 2)
		return ACT_VM_PRIMARYATTACK;

	if (m_nShotsFired < 3)
		return ACT_VM_RECOIL1;

	if (m_nShotsFired < 4)
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponPlasmaRifle::Reload(void)
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound(RELOAD);
	}

	return fRet;
}

bool CWeaponPlasmaRifle::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	StopParticleEffects(this);
	if (m_pAlarm)
	{
		CSoundEnvelopeController::GetController().Shutdown(m_pAlarm);
		m_pAlarm = NULL;
	}
	return BaseClass::Holster(pSwitchingTo);
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	1.0f	//Degrees
#define	SLIDE_LIMIT			2.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}


ConVar g_exp_plasma_pos("g_exp_plasma_pos", "0");
ConVar g_exp_plasma_pos_factor("g_exp_plasma_pos_factor", "0.05");
ConVar sk_plasmarifle_altfire_dmg("sk_plasmarifle_altfire_dmg", "16");
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::SecondaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
	return;

	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	if (iShots < 30.0f)
		return;

	Vector vecSrc = pPlayer->WorldSpaceCenter();
	g_pGameRules->RadiusDamage(CTakeDamageInfo(this, pPlayer, 200, DMG_DISSOLVE), vecSrc, 256, pPlayer->Classify(), false, true);
	EmitSound("NPC_CombineBall.Explosion");
	DispatchParticleEffect("plasma_altfire_core", vecSrc, GetAbsAngles(), this);
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
	pPlayer->ViewPunch(QAngle(random->RandomFloat(-30, -15), random->RandomFloat(-20, 20), 0));
	color32 white = { 255, 255, 255, 64 };
	UTIL_ScreenFade(pPlayer , white, 0.1, 0, FFADE_IN);
	iShots = 29;
	bCanShootAgain = false;
	fDrainTime = gpGlobals->curtime;

	SetThink(&CWeaponPlasmaRifle::AllowShoot);
	SetNextThink(gpGlobals->curtime + 1.0f);

}


void CWeaponPlasmaRifle::Overheat()
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;
	iShots = 29;
	SendWeaponAnim(ACT_VM_RELEASE);
	fDrainTime = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	Vector vecSrc = pPlayer->WorldSpaceCenter();
	RadiusDamage(CTakeDamageInfo(this, pPlayer, 200, DMG_DISSOLVE), vecSrc, 256, CLASS_PLAYER, NULL);
	RadiusDamage(CTakeDamageInfo(this, pPlayer, 50, DMG_PLASMA), vecSrc, 32, CLASS_NONE, NULL);
	EmitSound("NPC_CombineBall.Explosion");
	DispatchParticleEffect("plasma_altfire_core", vecSrc, GetAbsAngles(), this);
	DispatchParticleEffect("plasmarifle_overheat", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "vent");
	pPlayer->ViewPunch(QAngle(random->RandomFloat(-30, -15), random->RandomFloat(-20, 20), 0));
	bCanShootAgain = false;

}
////////////////////////////////////////////////////////
/////											   /////
/////    check it out this is the primary attack   /////
/////											   /////
////////////////////////////////////////////////////////
void CWeaponPlasmaRifle::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	pOwner->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAG_RESTART);

	Vector vecAiming;
	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors(&vForward, &vRight, &vUp);

	Vector vecSrc;
	ConVarRef classicpos("r_classic_weapon_pos");
	int right = classicpos.GetBool() ? 0 : 2;
	g_exp_plasma_pos.GetBool() ? (vecSrc = (pPlayer->EyePosition() + vForward * 12.0f + vRight * right + vUp * -3.0f) + (pPlayer->GetAbsVelocity() * g_exp_plasma_pos_factor.GetFloat())) : (vecSrc = pPlayer->EyePosition() + vForward * 12.0f + vRight * right + vUp * -3.0f);
	QAngle angAiming = pOwner->EyeAngles();
	AngleVectors(angAiming, &vecAiming);

	while (m_flNextPrimaryAttack <= gpGlobals->curtime) //while we're allowed to shoot
	{
		trace_t tr;
		Vector vecDir;
		
		// Take the Player's EyeAngles and turn it into a direction
		AngleVectors(pPlayer->EyeAngles(), &vecDir);
		Vector vecAbsStart = pPlayer->EyePosition();
		Vector vecAbsEnd = vecAbsStart + (vecDir * MAX_TRACE_LENGTH);
		UTIL_TraceLine(vecAbsStart, vecAbsEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr); //create a line from our offset initial position to where my crosshair is pointing
		Vector vecShotDir = (tr.endpos - vecSrc).Normalized();
		//debugoverlay->AddLineOverlay(tr.startpos, tr.endpos, 0, 255, 0, false, 3.0f);
		if (m_nShotsFired < 1)
			WeaponSound(SINGLE); //emit sound
		else
			WeaponSound(SPECIAL1);

		if (pPlayer->HasOverdrive())
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + (sk_plasmarifle_firerate.GetFloat() * 0.5f);
		else
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + sk_plasmarifle_firerate.GetFloat();
		//m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;//can't shoot again til after 0.1 seconds
		CPlasmaBall *pBall = CPlasmaBall::Create(vecSrc, angAiming, pOwner); //emit plasma ball object
		pBall->SetModel(PLASMA_MODEL); //set model to player model
		pBall->SetTargetPos(tr.endpos, PLASMA_SPEED);
		
		pBall->DrawSprite();
		pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);//remove 1 round from ammo count
		if (pPlayer->HasOverdrive())
		{
			pBall->m_pGlowTrail->SetTransparency(kRenderTransAdd, 255, 0, 0, 200, kRenderFxNone);//emit trail
			pBall->SetRenderColor(255, 0, 0);
		}
		else
		{
			pBall->m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 175, 255, 200, kRenderFxNone);//emit trail
			pBall->SetRenderColor(58, 215, 255);
		}
		pBall->SetLocalAngles(QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		pBall->SetLocalAngularVelocity(
			QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		
		m_nShotsFired++;

		if(!pPlayer->HasOverdrive())
			iShots++;
		if (iShots >= 30)
			fDrainTime = gpGlobals->curtime + 8.0f;
		else
			fDrainTime = gpGlobals->curtime + 2.0f;

		DoMuzzleFlash();
		int iAttachment = LookupAttachment("barrel_vm");
		if (g_thirdperson.GetBool() == false)
		{
			if (pPlayer->HasOverdrive())
				DispatchParticleEffect("smg_core_red", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), iAttachment, true);
			else
				DispatchParticleEffect("smg_core", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), iAttachment, true);
		}
		else
		{
			if (pPlayer->HasOverdrive())
				DispatchParticleEffect("smg_core_red", PATTACH_POINT_FOLLOW, this, iAttachment, true);
			else
				DispatchParticleEffect("smg_core", PATTACH_POINT_FOLLOW, this, iAttachment, true);
		}
	}
	
	pPlayer->CreateMuzzleLight(0, 200, 255, vecSrc);
	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	ConVarRef mvox("cl_hev_gender");

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0,mvox.GetBool());
	}
	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
}
#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponPlasmaRifle::WeaponRangeAttack2Condition(float flDot, float flDist)
{
	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();

	return COND_NONE;

	/*
	// --------------------------------------------------------
	// Assume things haven't changed too much since last time
	// --------------------------------------------------------
	if (gpGlobals->curtime < m_flNextGrenadeCheck )
	return m_lastGrenadeCondition;
	*/

	// -----------------------
	// If moving, don't check.
	// -----------------------
	if (npcOwner->IsMoving())
		return COND_NONE;

	CBaseEntity *pEnemy = npcOwner->GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP();
	if (!(pEnemy->GetFlags() & FL_ONGROUND) && pEnemy->GetWaterLevel() == 0 && vecEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z))
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;
	}

	// --------------------------------------
	//  Get target vector
	// --------------------------------------
	Vector vecTarget;
	if (random->RandomInt(0, 1))
	{
		// magically know where they are
		vecTarget = pEnemy->WorldSpaceCenter();
	}
	else
	{
		// toss it to where you last saw them
		vecTarget = vecEnemyLKP;
	}
	// vecTarget = m_vecEnemyLKP + (pEnemy->BodyTarget( GetLocalOrigin() ) - pEnemy->GetLocalOrigin());
	// estimate position
	// vecTarget = vecTarget + pEnemy->m_vecVelocity * 2;


	if ((vecTarget - npcOwner->GetLocalOrigin()).Length2D() <= COMBINE_MIN_GRENADE_CLEAR_DIST)
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return (COND_NONE);
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;

	while ((pTarget = gEntList.FindEntityInSphere(pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST)) != NULL)
	{
		//Check to see if the default relationship is hatred, and if so intensify that
		if (npcOwner->IRelationType(pTarget) == D_LI)
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
			return (COND_WEAPON_BLOCKED_BY_FRIEND);
		}
	}

	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------
	// FIXME: speed is based on difficulty...

	Vector vecToss = VecCheckThrow(this, npcOwner->GetLocalOrigin() + Vector(0, 0, 60), vecTarget, 600.0, 0.5);
	if (vecToss != vec3_origin)
	{
		m_vecTossVelocity = vecToss;

		// don't check again for a while.
		// JAY: HL1 keeps checking - test?
		//m_flNextGrenadeCheck = gpGlobals->curtime;
		m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.
		return COND_CAN_RANGE_ATTACK2;
	}
	else
	{
		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_WEAPON_SIGHT_OCCLUDED;
	}
}

void CWeaponPlasmaRifle::AllowShoot()
{
	bCanShootAgain = true;
}
void CWeaponPlasmaRifle::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	if (!(pPlayer->m_nButtons & IN_ATTACK) || !bCanShootAgain)
	{
		if (fDrainTime <= gpGlobals->curtime)
		{
			iShots--;
		}
	}

	if (iShots < 0)
		iShots = 0;

	bActive = (pPlayer->GetActiveWeapon() == this);


	if (iShots >= 70)
		Overheat();


	if (iShots >= 45)
	{
		CPASAttenuationFilter filter(this);

		if (!m_pAlarm)
		{
			m_pAlarm = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "PlasmaRifle.Overheat");
			CSoundEnvelopeController::GetController().Play(m_pAlarm, 1.0, 100);
		}
	}
	else
	{
		if (m_pAlarm)
		{
			CSoundEnvelopeController::GetController().Shutdown(m_pAlarm);
			m_pAlarm = NULL;
		}
	}

/*	if ((pPlayer->m_nButtons & IN_ATTACK2) && (pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
	{
		if (!bSecondaryPressed)
		{
			bSecondaryPressed = true;
			fSecondaryActivationTime = gpGlobals->curtime + 1.0f;
		}
		else if (fSecondaryActivationTime < gpGlobals->curtime)
		{
			bShootingSecondary = true;
		}
	}
	else
	{
		bSecondaryPressed = false;
		bShootingSecondary = false;
	}*/
}

void CWeaponPlasmaRifle::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();
	bActive = false;
}
//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponPlasmaRifle::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 10.0 / 3.0, 0.75 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
