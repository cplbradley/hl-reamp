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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#define PLASMA_MODEL "models/spitball_small.mdl"
#define PLASMA_MODEL_NPC "models/spitball_medium.mdl"
#define PLASMA_SPEED 3400
extern ConVar    sk_plr_dmg_smg1_grenade;
extern ConVar	sk_plr_dmg_smg1;
extern ConVar	 sk_npc_dmg_smg1;

class CPlasmaBall : public CBaseCombatCharacter
{
	DECLARE_CLASS(CPlasmaBall, CBaseCombatCharacter);
public:
	void	Spawn(void);
	void	Precache(void);
	void	ItemPostFrame(void);
	void	PlasmaTouch(CBaseEntity *pOther);
	void	KillIt(void);
	bool	CreateVPhysics(void);
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
bool CPlasmaBall::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_SOLID, false);

	return true;
}
unsigned int CPlasmaBall::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CPlasmaBall::Spawn(void)
{
	Precache();
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(1.0f, 1.0f, 1.0f), Vector(1.0f, 1.0f, 1.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID || FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	//SetSolidFlags(FSOLID_TRIGGER);
	CreateTrail();
	SetTouch(&CPlasmaBall::PlasmaTouch);
}
void CPlasmaBall::Precache(void)
{
	PrecacheModel(PLASMA_MODEL);
	PrecacheModel(PLASMA_MODEL_NPC);
	PrecacheParticleSystem("smg_plasmaball_core");
	PrecacheModel("sprites/smoke.vmt");
	PrecacheModel("sprites/physcannon_bluecore2b.vmt");

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
	m_pMainGlow = CSprite::SpriteCreate("sprites/physcannon_bluecore2b.vmt", GetLocalOrigin(), false);
	if (m_pMainGlow != NULL)
	{
		m_pMainGlow->FollowEntity(this);
		m_pMainGlow->SetTransparency(kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation);
		m_pMainGlow->SetScale(0.3f);
		m_pMainGlow->SetGlowProxySize(4.0f);
	}
	return true;
}
void CPlasmaBall::PlasmaTouch(CBaseEntity *pOther) //i touched something
{
	if (pOther->IsSolid()) //is what i touched solid?
	{
		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER)) //is it a trigger?
		{
			return; //ignore it, keep going
		}
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE) //is it another projectile?
		{
			return; //ignore it, keep going
		}
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_WEAPON)
		{
			return;
		}
		if (pOther->IsPlayer())
		{
			return;
		}
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
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 1.0f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
			ApplyMultiDamage();
		}
		SetMoveType(MOVETYPE_NONE);
		SetThink(&CPlasmaBall::KillIt); //schedule remove command
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime + 0.01f); //execute remove command after 0.01 seconds, this allows time to trigger the particle
	}
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

	int		GetMinBurst() { return 5; }
	int		GetMaxBurst() { return 5; }

	int		m_iEnergyLevel;

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

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponPlasmaRifle, DT_WeaponPlasmaRifle)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_plasmarifle, CWeaponPlasmaRifle);
PRECACHE_WEAPON_REGISTER(weapon_plasmarifle);

BEGIN_DATADESC(CWeaponPlasmaRifle)

DEFINE_FIELD(m_vecTossVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_flNextGrenadeCheck, FIELD_TIME),

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
};

IMPLEMENT_ACTTABLE(CWeaponPlasmaRifle);

//=========================================================
CWeaponPlasmaRifle::CWeaponPlasmaRifle()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 9600;

	m_bAltFiresUnderwater = false;

	m_iEnergyLevel = 0;
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPlasmaRifle::SecondaryAttack(void)
{
	// Only the player fires this way so we can cast


	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
	return;
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	if (m_iEnergyLevel < 30)
		return;

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	Vector vecSrc = pPlayer->WorldSpaceCenter();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	RadiusDamage(CTakeDamageInfo (this, pPlayer, 100, DMG_DISSOLVE),GetAbsOrigin(),256,CLASS_PLAYER,NULL);
	EmitSound("NPC_CombineBall.Explosion");
	DispatchParticleEffect("plasma_altfire_core", vecSrc, GetAbsAngles(), this);
	//m_flNextSecondaryAttack = gpGlobals->curtime + 3.5f;
	pPlayer->ViewPunch(QAngle(random->RandomFloat(-30, -15), random->RandomFloat(-20, 20), 0));
	color32 white = { 255, 255, 255, 64 };
	UTIL_ScreenFade(pPlayer , white, 0.1, 0, FFADE_IN);
	m_iEnergyLevel = 0;

	/*pPlayer->FireBullets(1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0, -1, -1, 1);
	trace_t tr; // Create our trace_t class to hold the end result
	// Do the TraceLine, and write our results to our trace_t class, tr.
	Vector vecAbsStart = pPlayer->EyePosition();
	pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	Vector vecAbsEnd = vecAbsStart + (vecAiming * MAX_TRACE_LENGTH);
	UTIL_TraceLine(vecAbsStart, vecAbsEnd, MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr);
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
	Vector vecPartStart;
	Vector vecEndPos = tr.endpos;
	DispatchParticleEffect("plasma_beam", vecSrc, tr.endpos, GetAbsAngles(), this);/**/
	/*
	if ( pPlayer == NULL )
	return;

	//Must have ammo
	if ( ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 ) || ( pPlayer->GetWaterLevel() == 3 ) )
	{
	SendWeaponAnim( ACT_VM_DRYFIRE );
	BaseClass::WeaponSound( EMPTY );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	return;
	}

	if( m_bInReload )
	m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound( WPN_DOUBLE );

	pPlayer->RumbleEffect( RUMBLE_357, 0, RUMBLE_FLAGS_NONE );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors( pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow );
	VectorScale( vecThrow, 2000.0f, vecThrow );

	//Create the grenade
	QAngle angles;
	VectorAngles( vecThrow, angles );
	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecSrc, angles, pPlayer );
	pGrenade->SetAbsVelocity( vecThrow );

	pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	pGrenade->SetThrower( GetOwner() );
	pGrenade->SetDamage( sk_plr_dmg_smg1_grenade.GetFloat() );

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Decrease ammo
	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, false, GetClassname() );*/
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

	Vector vecSrc = pPlayer->EyePosition() + vForward * 12.0f + vRight * 2.0f + vUp * -3.0f;
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
		UTIL_TraceLine(vecAbsStart, vecAbsEnd, MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr);
		Vector vecShotDir = (tr.endpos - vecSrc).Normalized();
		//debugoverlay->AddLineOverlay(tr.startpos, tr.endpos, 0, 255, 0, false, 3.0f);
		WeaponSound(SINGLE, m_flNextPrimaryAttack); //emit sound
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.1f;
		//m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;//can't shoot again til after 0.1 seconds
		CPlasmaBall *pBall = CPlasmaBall::Create(vecSrc, angAiming, pOwner); //emit plasma ball object
		pBall->SetModel(PLASMA_MODEL); //set model to player model
		pBall->SetAbsVelocity(vecShotDir * PLASMA_SPEED); //set speed and vector
		pBall->m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 175, 255, 200, kRenderFxNone);//emit trail
		pBall->DrawSprite();
		pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);//remove 1 round from ammo count
		pBall->SetRenderColor(58, 215, 255);
		pBall->SetLocalAngles(QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		pBall->SetLocalAngularVelocity(
			QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		m_iEnergyLevel++;
		int iAttachment = LookupAttachment("barrel");
		DispatchParticleEffect("smg_core", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), iAttachment, true);
	}
	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
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
void CWeaponPlasmaRifle::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
	if (m_iEnergyLevel >= 30)
	{
		m_iEnergyLevel = 30;
	}

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
