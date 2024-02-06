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
#include "hl2_gamerules.h"
#include "hl2_shareddefs.h"
#include "ai_memory.h"
#include "soundent.h"
#include "gamerules.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "SpriteTrail.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#define PLASMA_MODEL "models/spitball_small.mdl"
#define PLASMA_MODEL_NPC "models/spitball_medium.mdl"
#define PLASMA_SPEED 1500
extern ConVar    sk_plr_dmg_smg1_grenade;
extern ConVar	sk_plr_dmg_smg1;
extern ConVar	 sk_npc_dmg_smg1;

class CNPCPlasmaBall : public CBaseCombatCharacter
{
	DECLARE_CLASS(CNPCPlasmaBall, CBaseCombatCharacter);
public:
	void	Spawn(void);
	void	Precache(void);
	void	ItemPostFrame(void);
	void	PlasmaTouch(CBaseEntity *pOther);
	void	KillIt(void);
	bool	CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;


	static CNPCPlasmaBall *Create(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL);
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	bool	CreateTrail(void);
	bool	DrawSprite(void);
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(npc_plasma_ball, CNPCPlasmaBall);
BEGIN_DATADESC(CNPCPlasmaBall)
// Function Pointers
DEFINE_FUNCTION(PlasmaTouch),
END_DATADESC()
CNPCPlasmaBall *CNPCPlasmaBall::Create(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner)
{
	// Create a new entity with CNPCPlasmaBall data
	CNPCPlasmaBall *pBall = (CNPCPlasmaBall *)CreateEntityByName("npc_plasma_ball");
	UTIL_SetOrigin(pBall, vecOrigin);
	pBall->SetAbsAngles(angAngles);
	pBall->Spawn();
	pBall->SetOwnerEntity(pentOwner);

	return pBall;
}
bool CNPCPlasmaBall::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_CUSTOM, FSOLID_NOT_SOLID, false);

	return true;
}
unsigned int CNPCPlasmaBall::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CNPCPlasmaBall::Spawn(void)
{
	Precache();
	SetMoveType(MOVETYPE_FLY);
	UTIL_SetSize(this, -Vector(1.0f, 1.0f, 1.0f), Vector(1.0f, 1.0f, 1.0f));
	SetSolid(SOLID_BBOX);
	SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	//SetSolidFlags(FSOLID_TRIGGER);
	SetModel(PLASMA_MODEL_NPC);
	SetRenderColor(255, 0, 0);
	CreateTrail();
	SetTouch(&CNPCPlasmaBall::PlasmaTouch);

}
void CNPCPlasmaBall::Precache(void)
{
	PrecacheModel(PLASMA_MODEL);
	PrecacheModel(PLASMA_MODEL_NPC);
	PrecacheParticleSystem("smg_plasmaball_core");
	PrecacheModel("sprites/smoke.vmt");
	PrecacheModel("sprites/physcannon_bluecore2b.vmt");
}
bool CNPCPlasmaBall::CreateTrail(void)
{
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/smoke.vmt", GetLocalOrigin(), false);
	if (m_pGlowTrail != NULL)
	{
		m_pGlowTrail->FollowEntity(this);
		m_pGlowTrail->SetStartWidth(6.0f);
		m_pGlowTrail->SetEndWidth(0.2f);
		m_pGlowTrail->SetLifeTime(0.05f);
		m_pGlowTrail->SetTransparency(kRenderWorldGlow, 255, 0, 0, 200, kRenderFxNone);
	}
	return true;
}
bool CNPCPlasmaBall::DrawSprite(void)
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
void CNPCPlasmaBall::PlasmaTouch(CBaseEntity *pOther) //i touched something
{
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
			return;
	}

		DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this); //poof effect!

		if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
		{
			trace_t	tr, tr2; //initialize info
			tr = BaseClass::GetTouchTrace(); //trace touch
			Vector	vecNormalizedVel = GetAbsVelocity();

			ClearMultiDamage();
			VectorNormalize(vecNormalizedVel);
			if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC()) //am i owned by the player and is what i touched an NPC? if so, i should do damage.
			{
				CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_smg1.GetFloat(), DMG_SHOCK);
				dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 1.0f);
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
				//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this);
			}
			if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && !pOther->IsNPC())
			{
				CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_smg1.GetFloat(), DMG_BULLET | DMG_NEVERGIB);
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 1.0f);
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
			}

			if (GetOwnerEntity() && !GetOwnerEntity()->IsPlayer() && pOther->IsPlayer()) //am i owned by something other than the player? did i touch the player? if so, i should do damage
			{
				CTakeDamageInfo dmgInfo(this, GetOwnerEntity(), sk_npc_dmg_smg1.GetFloat(), DMG_SHOCK);
				dmgInfo.AdjustPlayerDamageTakenForSkillLevel();
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);//scale damage
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
			}
			ApplyMultiDamage();
		}
		string_t str = pOther->m_iClassname;
		DevMsg("Hit Object %s\n", str);
		Vector vecStall = Vector(0, 0, 0);
		SetAbsVelocity(vecStall); //freeze in place
		SetThink(&CNPCPlasmaBall::KillIt); //schedule remove command
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime + 0.01f); //execute remove command after 0.01 seconds
}
void CNPCPlasmaBall::KillIt(void)
{
	SetThink(NULL);
	m_pGlowTrail == NULL;
	UTIL_Remove(this); //remove it
}
void CNPCPlasmaBall::ItemPostFrame(void)
{
	if (GetAbsVelocity() == 0)
	{
		KillIt();
	}
}


class CWeaponSMG1 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponSMG1, CHLSelectFireMachineGun );

	CWeaponSMG1();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );
	void	AddViewKick( void );
	void	SecondaryAttack( void );
	void	PrimaryAttack(void);

	int		GetMinBurst();
	int		GetMaxBurst() { return GetMinBurst(); }

	virtual void Equip( CBaseCombatCharacter *pOwner );
	bool	Reload( void );

	float	GetFireRate( void ) { return 0.15f; }	// 13.3hz
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1 | bits_CAP_WEAPON_RANGE_ATTACK2; }
	int		WeaponRangeAttack2Condition();
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_2DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponSMG1, DT_WeaponSMG1)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_smg1, CWeaponSMG1 );
PRECACHE_WEAPON_REGISTER(weapon_smg1);

BEGIN_DATADESC( CWeaponSMG1 )

	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),

END_DATADESC()

acttable_t	CWeaponSMG1::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },
	
// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE(CWeaponSMG1);

//=========================================================
CWeaponSMG1::CWeaponSMG1( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1024;

	m_bAltFiresUnderwater = false;
	//m_iClip1 = 50;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Precache( void )
{
	UTIL_PrecacheOther("grenade_ar2");
	UTIL_PrecacheOther("npc_plasma_ball");
	SetSkin(1);

	BaseClass::Precache();
}

int CWeaponSMG1::GetMinBurst()
{
	switch (g_iSkillLevel)
	{
	case SKILL_EASY:
		return 3;

	case SKILL_MEDIUM:
		return 5;

	case SKILL_HARD:
		return 8;

	default:
		return 0.0f;
	}
}
//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponSMG1::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSound( SINGLE_NPC );	//call weapon sound
	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() ); //emit with properties

	CNPCPlasmaBall* pBall = CNPCPlasmaBall::Create(vecShootOrigin, RandomAngle(0, 100), pOperator);

	float fSpeed = g_pGameRules->SkillAdjustValue(1500.0f);

	pBall->SetAbsVelocity(vecShootDir * fSpeed);
	
	pOperator->DoMuzzleFlash();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_SMG1:
		{
			Vector vecShootOrigin, vecShootDir;
			QAngle angDiscard;

			GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angDiscard);

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT(npc != NULL);
			Vector vecTarget = vec3_origin;
			if (npc->GetEnemy())
				vecTarget = npc->GetEnemy()->WorldSpaceCenter();
			vecShootDir = npc->GetSkillAdjustedShootTrajectory(vecShootOrigin, vecTarget, g_pGameRules->SkillAdjustValue(1500.0f));


			FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
		}
		break;

	case EVENT_WEAPON_AR2_ALTFIRE:
	{
		CAI_BaseNPC *npc = pOperator->MyNPCPointer();

		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		//vecShootDir = npc->GetShootEnemyDir( vecShootOrigin );

		//Checks if it can fire the grenade
		WeaponRangeAttack2Condition();

		Vector vecThrow = m_vecTossVelocity;

		//If on the rare case the vector is 0 0 0, cancel for avoid launching the grenade without speed
		//This should be on WeaponRangeAttack2Condition(), but for some unknown reason return CASE_NONE
		//doesn't stop the launch
		if (vecThrow == Vector(0, 0, 0)){
			break;
		}

		CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecShootOrigin, vec3_angle, npc);
		pGrenade->SetAbsVelocity(vecThrow);
		pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400)); //tumble in air
		pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

		pGrenade->SetThrower(GetOwner());

		pGrenade->SetGravity(0.5); // lower gravity since grenade is aerodynamic and engine doesn't know it.

		pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());

		if (g_pGameRules->IsSkillLevel(SKILL_HARD))
		{
			m_flNextGrenadeCheck = gpGlobals->curtime + RandomFloat(2, 3);
		}
		else{
			m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}

		m_iClip2--;
	}
	break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponSMG1::GetPrimaryAttackActivity( void )
{ 
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponSMG1::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	1.0f	//Degrees
	#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	/*CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
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
/////    check it out this is the primary attack   /////
////////////////////////////////////////////////////////
void CWeaponSMG1::PrimaryAttack(void)
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

	Vector vecSrc = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 2.0f + vUp * -3.0f;
	QAngle angAiming = pOwner->EyeAngles();
	AngleVectors(angAiming, &vecAiming);
	while (m_flNextPrimaryAttack <= gpGlobals->curtime) //while we're allowed to shoot
	{
		WeaponSound(SINGLE, m_flNextPrimaryAttack); //emit sound
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + 0.1f; //can't shoot again til after 0.1 seconds
		CNPCPlasmaBall *pBall = CNPCPlasmaBall::Create(vecSrc, angAiming, pOwner); //emit plasma ball object
		pBall->SetModel(PLASMA_MODEL); //set model to player model
		float fProjSpeed = PLASMA_SPEED;
		g_pGameRules->SkillAdjustValue(fProjSpeed);
		pBall->SetAbsVelocity(vecAiming * fProjSpeed); //set speed and vector
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
int CWeaponSMG1::WeaponRangeAttack2Condition()
{
	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();

	//return COND_NONE;

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
	if ( npcOwner->IsMoving())
		return COND_NONE;

	CBaseEntity *pEnemy = npcOwner->GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP();
	if ( !( pEnemy->GetFlags() & FL_ONGROUND ) && pEnemy->GetWaterLevel() == 0 && vecEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z) )
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
	if (random->RandomInt(0,1))
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


	if ( ( vecTarget - npcOwner->GetLocalOrigin() ).Length2D() <= COMBINE_MIN_GRENADE_CLEAR_DIST )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return (COND_NONE);
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;

	while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST ) ) != NULL )
	{
		//Check to see if the default relationship is hatred, and if so intensify that
		if ( npcOwner->IRelationType( pTarget ) == D_LI )
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

	Vector vecToss = VecCheckThrow( this, npcOwner->GetLocalOrigin() + Vector(0,0,60), vecTarget, 600.0, 0.5 );
	if ( vecToss != vec3_origin )
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

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponSMG1::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
