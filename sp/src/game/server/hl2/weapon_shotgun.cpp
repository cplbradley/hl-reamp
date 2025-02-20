//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A shotgun.
//
//			Primary attack: single barrel shot.
//			Secondary attack: double barrel shot.
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"		// For g_pGameRules
#include "in_buttons.h"
#include "npc_hunter.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "hl2_shareddefs.h"
#include "gamestats.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "te_effect_dispatch.h"
#include "weapon_grapple.h"
#include "particle_parse.h"
#include "hlr/weapons/actual_bullet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar mat_classic_render;

ConVar newshotgun("newshotgun", "1");
class CShotgunPellet : public CBaseCombatCharacter
{
	DECLARE_CLASS(CShotgunPellet, CBaseCombatCharacter);
public:
	void	Spawn(void);
	void	Precache(void);
	void	ItemPostFrame(void);
	void	PelletTouch(CBaseEntity *pOther);
	void	KillIt(void);
	//void	EnableTouch(void);
	bool	CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;


	static CShotgunPellet *Create(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL);
	CHandle<CSpriteTrail>	m_pGlowTrail;
	bool	CreateTrail(void);
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(shotgun_pellet, CShotgunPellet);
BEGIN_DATADESC(CShotgunPellet)
// Function Pointers
DEFINE_FUNCTION(PelletTouch),
END_DATADESC()
CShotgunPellet *CShotgunPellet::Create(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner)
{
	// Create a new entity with CShotgunPellet data
	CShotgunPellet *pPellet = (CShotgunPellet *)CreateEntityByName("shotgun_pellet");
	UTIL_SetOrigin(pPellet, vecOrigin);
	pPellet->SetAbsAngles(angAngles);
	pPellet->Spawn();
	pPellet->SetOwnerEntity(pentOwner);
	pPellet->SetModel("models/spitball_small.mdl");
	return pPellet;
}
bool CShotgunPellet::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_SOLID, false);

	return true;
}
unsigned int CShotgunPellet::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CShotgunPellet::Spawn(void)
{
	Precache();
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(0.1f, 0.1f, 0.1f), Vector(0.1f, 0.1f, 0.1f));
	SetSolid(SOLID_BBOX);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	CreateTrail();
	SetTouch(&CShotgunPellet::PelletTouch);
}
void CShotgunPellet::Precache(void)
{
	PrecacheModel("models/spitball_small.mdl");
	//PrecacheModel(PLASMA_MODEL_NPC);
	//PrecacheParticleSystem("smg_plasmaball_core");
}
bool CShotgunPellet::CreateTrail(void)
{
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/smoke.vmt", GetLocalOrigin(), false);
	if (m_pGlowTrail != NULL)
	{
		m_pGlowTrail->FollowEntity(this);

		m_pGlowTrail->SetStartWidth(6.0f);
		m_pGlowTrail->SetEndWidth(0.2f);
		m_pGlowTrail->SetLifeTime(0.15f);
	}
	return true;
}
void CShotgunPellet::PelletTouch(CBaseEntity *pOther) //i touched something
{
	if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
		return;

	if (pOther->IsSolid()) //is what i touched solid?
	{
		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER)) //is it a trigger?
		{
			return; //carry on like nothing happened
		}
		//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this); //poof effect!
	
		if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
		{
			trace_t	tr, tr2; //initialize info
			tr = BaseClass::GetTouchTrace(); //trace touch
			Vector	vecNormalizedVel = GetAbsVelocity();

			ClearMultiDamage();
			VectorNormalize(vecNormalizedVel);
			if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC()) //am i owned by the player and is what i touched an NPC? if so, i should do damage.
			{
				CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), 12.0f, DMG_SHOCK);
				dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 1.0f);
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
				//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this);
			}
			if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && !pOther->IsNPC())
			{
				CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), 12.0f, DMG_BULLET | DMG_NEVERGIB);
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 1.0f);
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
			}

			if (GetOwnerEntity() && !GetOwnerEntity()->IsPlayer() && pOther->IsPlayer()) //am i owned by something other than the player? did i touch the player? if so, i should do damage
			{
				CTakeDamageInfo dmgInfo(this, GetOwnerEntity(), 12.0f, DMG_BULLET);
				dmgInfo.AdjustPlayerDamageTakenForSkillLevel();
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);//scale damage
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
			}
			ApplyMultiDamage();
			
		}
		UTIL_Remove(this);
	}
}
void CShotgunPellet::KillIt(void)
{
	m_pGlowTrail == NULL;
	UTIL_Remove(this); //remove it
}
void CShotgunPellet::ItemPostFrame(void)
{
	if (GetAbsVelocity() == 0)
	{
		KillIt();
	}
}
extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;

class CWeaponShotgun : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponShotgun, CBaseHLCombatWeapon );

	DECLARE_SERVERCLASS();

private:
	bool	m_bNeedPump;		// When emptied completely
	bool	m_bDelayedFire1;	// Fire primary when finished reloading
	bool	m_bDelayedFire2;	// Fire secondary when finished reloading
	


public:
	void	Precache( void );

	int CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector vitalAllyCone = VECTOR_CONE_3DEGREES;
		static Vector cone = VECTOR_HALFCONE_20DEGREES;

		if( GetOwner() && (GetOwner()->Classify() == CLASS_PLAYER_ALLY_VITAL) )
		{
			// Give Alyx's shotgun blasts more a more directed punch. She needs
			// to be at least as deadly as sfbhe would be with her pistol to stay interesting (sjb)
			return vitalAllyCone;
		}

		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 3; }

	virtual float			GetMinRestTime();
	virtual float			GetMaxRestTime();

	virtual float			GetFireRate( void );

	const char *GetTracerType(void) { return "AR2Tracer"; }

	bool StartReload( void );
	bool Reload( void );
	void FillClip( void );
	void FinishReload( void );
	void CheckHolsterReload( void );
	void Pump( void );
//	void WeaponIdle( void );
	void ItemHolsterFrame( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void DryFire( void );
	static int gm_nBarrelPoseParam;
	//void HandleAnimEvent(animevent_t* pEvent);


	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	DECLARE_ACTTABLE();
	CWeaponShotgun(void);
};

IMPLEMENT_SERVERCLASS_ST(CWeaponShotgun, DT_WeaponShotgun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_shotgun, CWeaponShotgun );
PRECACHE_WEAPON_REGISTER(weapon_shotgun);

BEGIN_DATADESC( CWeaponShotgun )

	DEFINE_FIELD( m_bNeedPump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDelayedFire1, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDelayedFire2, FIELD_BOOLEAN ),

END_DATADESC()

acttable_t	CWeaponShotgun::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
	{ ACT_WALK,						ACT_WALK_RIFLE,						true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

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

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HLR_SUPERSHOTGUN_FIRE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HLR_SUPERSHOTGUN_RELOAD, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_RANGE_ATTACK1, ACT_HLR_SUPERSHOTGUN_FIRE, false },
};

IMPLEMENT_ACTTABLE(CWeaponShotgun);

int CWeaponShotgun::gm_nBarrelPoseParam = -1;

void CWeaponShotgun::Precache( void )
{
	PrecacheParticleSystem("muzzleflash_orange_large_core");
	CBaseCombatWeapon::Precache();
	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponShotgun::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT( npc != NULL );
	WeaponSound( SINGLE_NPC );
	pOperator->DoMuzzleFlash();

	float adjustedspeed = g_pGameRules->SkillAdjustValue(1400.0f);	

	QAngle	angShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	vecShootDir = npc->GetShootEnemyDir(vecShootOrigin);

	//pOperator->FireBullets( 0, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0 );
	for ( int i = 0; i < 5; i++ )
	{
		Vector vecSpread(random->RandomFloat(-0.08716, 0.08716), random->RandomFloat(-0.08716, 0.08716), random->RandomFloat(-0.08716, 0.08716));
		Vector vecSpreadShot = (vecShootDir + vecSpread);
		VectorNormalize(vecSpreadShot);
		QAngle angAiming;
		VectorAngles(vecShootDir, angAiming);
		CShotgunPellet *pPellet = CShotgunPellet::Create(vecShootOrigin, angAiming, pOperator);
		pPellet->SetAbsVelocity(vecSpreadShot * adjustedspeed);
		pPellet->m_pGlowTrail->SetTransparency(kRenderTransAdd, 45, 45, 255, 100, kRenderFxNone);
		pPellet->SetRenderColor(255, 0, 0);
		QAngle angRand = RandomAngle(0, 360);
		pPellet->SetLocalAngles(angRand);
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShotgun::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack( pOperator, true );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_SHOTGUN_FIRE:
		{
			FireNPCPrimaryAttack( pOperator, false );
			break;
		}
		case AE_SUPERSHOTGUN_OPENBARREL:
		{
			SetPoseParameter(gm_nBarrelPoseParam, 45.0f);
			break;
		}
		case AE_SUPERSHOTGUN_CLOSEBARREL:
		{
			SetPoseParameter(gm_nBarrelPoseParam, 0.0f);
			break;
		}

		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}


//-----------------------------------------------------------------------------
// Purpose:	When we shipped HL2, the shotgun weapon did not override the
//			BaseCombatWeapon default rest time of 0.3 to 0.6 seconds. When
//			NPC's fight from a stationary position, their animation events
//			govern when they fire so the rate of fire is specified by the
//			animation. When NPC's move-and-shoot, the rate of fire is 
//			specifically controlled by the shot regulator, so it's imporant
//			that GetMinRestTime and GetMaxRestTime are implemented and provide
//			reasonable defaults for the weapon. To address difficulty concerns,
//			we are going to fix the combine's rate of shotgun fire in episodic.
//			This change will not affect Alyx using a shotgun in EP1. (sjb)
//-----------------------------------------------------------------------------
float CWeaponShotgun::GetMinRestTime()
{
	if( hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE )
	{
		return 1.2f;
	}
	
	return BaseClass::GetMinRestTime();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CWeaponShotgun::GetMaxRestTime()
{
	if( hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE )
	{
		return 1.5f;
	}

	return BaseClass::GetMaxRestTime();
}

//-----------------------------------------------------------------------------
// Purpose: Time between successive shots in a burst. Also returned for EP2
//			with an eye to not messing up Alyx in EP1.
//-----------------------------------------------------------------------------
float CWeaponShotgun::GetFireRate()
{
	if( hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE )
	{
		return 0.8f;
	}

	return 0.7;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponShotgun::StartReload( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	// If shotgun totally emptied then a pump animation is needed
	
	//NOTENOTE: This is kinda lame because the player doesn't get strong feedback on when the reload has finished,
	//			without the pump.  Technically, it's incorrect, but it's good for feedback...

	if (m_iClip1 <= 0)
	{
		m_bNeedPump = false;
	}

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	pPlayer->SetAnimation(PLAYER_RELOAD);
	SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	// Make shotgun shell visible
	SetBodygroup(1,0);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

	m_bInReload = true;
	
	gm_nBarrelPoseParam = LookupPoseParameter("barrel_angle");
	SetPoseParameter(gm_nBarrelPoseParam, 45.0f);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponShotgun::Reload( void )
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	FillClip();
	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);
	SendWeaponAnim( ACT_VM_RELOAD );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::FinishReload( void )
{
	// Make shotgun shell invisible
	SetBodygroup(1,1);

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	SetPoseParameter(gm_nBarrelPoseParam, 0.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::FillClip( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	// Add them to the clip
	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 )
	{
		if (Clip1() < GetMaxClip1())
		{
			if (Clip1() > 0)
			{
				m_iClip1++;
				pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
			}
			else
			{
				m_iClip1 += 2;
				pOwner->RemoveAmmo(2, m_iPrimaryAmmoType);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play weapon pump anim
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::Pump( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;
	
	m_bNeedPump = false;
	
	//WeaponSound( SPECIAL1 );

	// Finish reload animation
	//SendWeaponAnim( ACT_SHOTGUN_PUMP );

	pOwner->m_flNextAttack	= gpGlobals->curtime;
	m_flNextPrimaryAttack	= gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponShotgun::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponShotgun::SecondaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}
	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.15;
		}
		return;
	}
	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
	if (mat_classic_render.GetInt() == 0)
		WeaponSound(SINGLE);
	else
		WeaponSound(SPECIAL2);
	pPlayer->DoMuzzleFlash();
	
	SendWeaponAnim(GetPrimaryAttackActivity());
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;
	//Disabled so we can shoot all the time that we want
	m_iClip1--;
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	//We will not shoot bullets anymore

	/*FireBulletsInfo_t info;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_vecSrc = vecSrc;
	info.m_vecSpread = GetBulletSpread();
	info.m_vecDirShooting = vecAiming;
	info.m_pAttacker = GetOwnerEntity();
	info.m_iTracerFreq = 1;*/
	if (pPlayer->HasOverdrive())
	{
		pPlayer->FireBullets(20, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, false, false);
		//info.m_iShots = 28;
		//FireActualBullet(info, 6800, GetTracerType());
		//info.m_iShots = 28;
		//FireBullets(info);
	}
	else
	{
		pPlayer->FireBullets(14, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, false, false);
		//info.m_iShots = 14;
		//info.m_iShots = 14;
		//FireActualBullet(info, 6800, GetTracerType());
		//FireBullets(info);
	}
	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);
	DispatchParticleEffect("muzzleflash_orange_large_core", vecSrc, angAiming, this);
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
	pPlayer->CreateMuzzleLight(255, 200, 0, vecSrc);
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner());

	ConVarRef mvox("cl_hev_gender");

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0, mvox.GetBool());
	}
	
}
//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponShotgun::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	pPlayer->m_nButtons &= ~IN_ATTACK2;

	if (m_iClip1 == 1 && !pPlayer->HasOverdrive())
	{
		SecondaryAttack();
		return;	
	}
	// MUST call sound before removing a round from the clip of a CMachineGun
	if (mat_classic_render.GetInt() == 0)
		WeaponSound(WPN_DOUBLE);
	else
		WeaponSound(SPECIAL1);

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Don't fire again until fire animation has completed
	ConVarRef classicpos("r_classic_weapon_pos");

		// Shotgun uses same clip for primary and secondary attacks
	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	int right = classicpos.GetBool() ? 0 : 3;
	Vector vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * right + vUp * -2.0f;
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	trace_t trh;
	bool bHull = false;
	AI_TraceHull(vecSrc, vecSrc + (vecAiming * 128.f), Vector(-32, -32, -32), Vector(32, 32, 32), MASK_SHOT, GetOwnerEntity(), COLLISION_GROUP_NPC, &trh);
	if (trh.m_pEnt && trh.m_pEnt->IsNPC() && newshotgun.GetBool())
	{
		bHull = true;
	}
	FireBulletsInfo_t info;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_vecSrc = vecSrc;
	info.m_vecSpread = GetBulletSpread();
	info.m_vecDirShooting = vecAiming;
	info.m_pAttacker = GetOwnerEntity();
	info.m_iShots = 30;
	info.m_iTracerFreq = 1;
	
	if (bHull)
		info.m_flDamage = 0.01f;

	// Fire the bullets
	if (pPlayer->HasOverdrive())
	{
		m_iClip1--;
		//pPlayer->FireBullets(50, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, true, false);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
		FireBullets(info);
	}
	else
	{
		//pPlayer->FireBullets(30, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, true, false);
		m_iClip1 -= 2;
		m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
		FireBullets(info);
	}

	if (trh.m_pEnt && bHull)
	{
		CTakeDamageInfo dmgInfo(GetOwnerEntity(), GetOwnerEntity(), 180.f, DMG_BUCKSHOT);
		CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, (trh.endpos - trh.startpos).Normalized(), pPlayer->Weapon_ShootPosition());
		trh.m_pEnt->TakeDamage(dmgInfo);
	}

	QAngle angles = pPlayer->GetLocalAngles();

	angles.x += random->RandomInt(0, 0);
	angles.y += random->RandomInt(0, 0);
	angles.z = 0;

	pPlayer->SnapEyeAngles(angles);

	pPlayer->ViewPunch(QAngle(-12, random->RandomFloat(0, 2), 0));

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 1.0 );
	pPlayer->CreateMuzzleLight(255, 200, 0,vecSrc);
	DispatchParticleEffect("muzzleflash_orange_large_core", vecSrc, angAiming, this);
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2 );
	ConVarRef mvox("cl_hev_gender");

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0, mvox.GetBool()); 
	}

	if( m_iClip1 )
	{
		// pump so long as some rounds are left.
		m_bNeedPump = false;
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, false, GetClassname() );
}
	
//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponShotgun::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
	{
		return;
	}
	
	if (m_bInReload)
	{
		// If I'm primary firing and have one round stop reloading and fire
		if ((pOwner->m_nButtons & IN_ATTACK ) && (m_iClip1 >=1))
		{
			m_bInReload		= false;
			m_bNeedPump		= false;
			m_bDelayedFire1 = true;
		}
		// If I'm secondary firing and have one round stop reloading and fire
		else if ((pOwner->m_nButtons & IN_ATTACK2 ) && (m_iClip1 >=2))
		{
			m_bInReload		= false;
			m_bNeedPump		= false;
			m_bDelayedFire2 = true;
		}
		else if (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <=0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			if (m_iClip1 < GetMaxClip1())
			{
				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				return;
			}
		}
	}
	else
	{			
		// Make shotgun shell invisible
		SetBodygroup(1,1);
	}

	if ((m_bNeedPump) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		//Pump();
		return;
	}
	
	// Shotgun uses same timing and ammo for secondary attack
	if ((m_bDelayedFire2 || pOwner->m_nButtons & IN_ATTACK2)&&(m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		m_bDelayedFire2 = false;
		
		if ( (m_iClip1 <= 1 && UsesClipsForAmmo1()))
		{
			// If only one shell is left, do a single shot instead	
			if ( m_iClip1 == 1 )
			{
				SecondaryAttack();
			}
			else if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else
			{
				StartReload();
			}
		}

		// Fire underwater?
		else if (GetOwner()->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			if ( pOwner->m_afButtonPressed & IN_ATTACK )
			{
				 m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			SecondaryAttack();
		}
	}
	else if ( (m_bDelayedFire1 || pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		m_bDelayedFire1 = false;
		if ( (m_iClip1 <= 0 && UsesClipsForAmmo1()) || ( !UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType) ) )
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else
			{
				StartReload();
			}
		}
		// Fire underwater?
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
			if ( pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK )
			{
				 m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if ( pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	else 
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if ( !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime ) 
		{
			// weapon isn't useable, switch.
			if ( !(GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && pOwner->SwitchToNextBestWeapon( this ) )
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if ( m_iClip1 <= 0 && !(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->curtime )
			{
				if (StartReload())
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}
		}

		WeaponIdle( );
		return;
	}


}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponShotgun::CWeaponShotgun(void)
{
	m_bReloadsSingly = true;

	m_bNeedPump = false;
	m_bDelayedFire1 = false;
	m_bDelayedFire2 = false;

	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 500;
	m_fMinRange2 = 0.0;
	m_fMaxRange2 = 200;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShotgun::ItemHolsterFrame( void )
{
	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;

	// If it's been longer than three seconds, reload
	if ( ( gpGlobals->curtime - m_flHolsterTime ) > sk_auto_reload_time.GetFloat() )
	{
		// Reset the timer
		m_flHolsterTime = gpGlobals->curtime;
	
		if ( GetOwner() == NULL )
			return;

		if ( m_iClip1 == GetMaxClip1() )
			return;

		// Just load the clip with no animations
		int ammoFill = MIN( (GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount( GetPrimaryAmmoType() ) );
		
		GetOwner()->RemoveAmmo( ammoFill, GetPrimaryAmmoType() );
		m_iClip1 += ammoFill;
	}
}

//==================================================
// Purpose: 
//==================================================
/*
void CWeaponShotgun::WeaponIdle( void )
{
	//Only the player fires this way so we can cast
	CBasePlayer *pPlayer = GetOwner()

	if ( pPlayer == NULL )
		return;

	//If we're on a target, play the new anim
	if ( pPlayer->IsOnTarget() )
	{
		SendWeaponAnim( ACT_VM_IDLE_ACTIVE );
	}
}
*/
