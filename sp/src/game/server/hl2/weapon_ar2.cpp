//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basecombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "weapon_ar2.h"
#include "grenade_ar2.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "ai_memory.h"
#include "soundent.h"
#include "hl2_player.h"
#include "EntityFlame.h"
#include "particle_parse.h"
#include "weapon_flaregun.h"
#include "te_effect_dispatch.h"
#include "prop_combine_ball.h"
#include "beam_shared.h"
#include "npc_combine.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "actual_bullet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#define MINFIRERATE 0.24f
#define MAXFIRERATE 0.06f
#define MAXOVERDRIVEFIRERATE 0.03f
#define MAXFOCUSFIRERATE 0.09f

ConVar sk_weapon_ar2_alt_fire_radius( "sk_weapon_ar2_alt_fire_radius", "10" );
ConVar sk_weapon_ar2_alt_fire_duration( "sk_weapon_ar2_alt_fire_duration", "2" );
ConVar sk_weapon_ar2_alt_fire_mass( "sk_weapon_ar2_alt_fire_mass", "150" );

enum {
	WEAPON_STATE_FULLSPEED,
	WEAPON_STATE_NOTFULLSPEED
};
//=========================================================
//=========================================================

BEGIN_DATADESC( CWeaponAR2 )
	
	DEFINE_FIELD( m_flfirerate,		FIELD_FLOAT),
	DEFINE_FIELD( m_flDelayedFire,	FIELD_TIME ),
	DEFINE_FIELD( m_bShotDelayed,	FIELD_BOOLEAN ),
	DEFINE_SOUNDPATCH(m_pWoundSound),
	DEFINE_FIELD(m_iWeaponState, FIELD_INTEGER),
	//DEFINE_FIELD( m_nVentPose, FIELD_INTEGER ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponAR2, DT_WeaponAR2)

END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_ar2, CWeaponAR2 );
PRECACHE_WEAPON_REGISTER(weapon_ar2);

acttable_t	CWeaponAR2::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },		// FIXME: hook to AR2 unique

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },

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
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },


	{ ACT_HL2MP_IDLE, ACT_HLR_IDLE_CHAINGUN, false },
	{ ACT_HL2MP_RUN, ACT_HLR_RUN_CHAINGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HLR_JUMP_CHAINGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, false },
//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
};

IMPLEMENT_ACTTABLE(CWeaponAR2);

CWeaponAR2::CWeaponAR2( )
{
	m_fMinRange1	= 65;
	m_fMaxRange1	= 2048;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;

	m_nShotsFired	= 0;
	m_nVentPose		= -1;
	m_flfirerate = MINFIRERATE;

	m_iMuzzleR = 0;
	m_iMuzzleG = 128;
	m_iMuzzleB = 250;

	vMuzzleFlashLightColor = Vector(0, 128, 250);

	m_bAltFiresUnderwater = false;
	m_bPlayingWoundSound = false;
	m_iWeaponState = WEAPON_STATE_NOTFULLSPEED;
	//InitWoundSound();
}


float CWeaponAR2::GetMaxFirerate()
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner->HasOverdrive())
		return MAXOVERDRIVEFIRERATE;
	else if (AmFocusFiring())
		return MAXFOCUSFIRERATE;
	else
		return MAXFIRERATE;
}

void CWeaponAR2::Precache( void )
{
	BaseClass::Precache();
	PrecacheScriptSound("Chaingun.Wine");
	UTIL_PrecacheOther( "prop_combine_ball" );
	UTIL_PrecacheOther( "env_entity_dissolver" );
	PrecacheParticleSystem("smg_core");
}


bool CWeaponAR2::AmFocusFiring(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner->m_nButtons & IN_ATTACK2)
		return true;
	else
		return false;
}

void CWeaponAR2::ToggleZoom(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (!m_bNarrowfov)
	{
		pPlayer->SetFOV(this, pPlayer->GetDefaultFOV() - 15, 0.5f);
		m_bNarrowfov = true;
	}
	else
	{
		pPlayer->SetFOV(this, 0, 0.5f);
		m_bNarrowfov = false;
	}
}
//-----------------------------------------------------------------------------
// Purpose: Handle grenade detonate in-air (even when no ammo is left)
//-----------------------------------------------------------------------------
void CWeaponAR2::ItemPostFrame(void)
{
	// See if we need to fire off our secondary round
	if (m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire)
	{
		DelayedAttack();
	}

	// Update our pose parameter for the vents
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner->m_nButtons & IN_ATTACK)
	{

		m_flfirerate -= 0.003f;
	}

	if (AmFocusFiring())
	{
		if (!m_bNarrowfov)
			ToggleZoom();
		pOwner->SetMaxSpeed(220.0f);
		m_flfirerate = GetMaxFirerate();
	}
	else
	{
		if (m_bNarrowfov)
			ToggleZoom();
		pOwner->SetMaxSpeed(450.0f);
	}

	if (m_flfirerate > MINFIRERATE)
		m_flfirerate = MINFIRERATE;
	
	/* {
		if (m_flfirerate < MAXOVERDRIVEFIRERATE)
			m_flfirerate = MAXOVERDRIVEFIRERATE;
		if (m_flfirerate == MAXOVERDRIVEFIRERATE && pOwner->m_nButtons & IN_ATTACK)
			m_iWeaponState = WEAPON_STATE_FULLSPEED;
		else
		{
			m_iWeaponState = WEAPON_STATE_NOTFULLSPEED;
			m_bPlayingWoundSound = false;
		}
	}
	else
	{
		if (m_flfirerate < MAXFIRERATE)
			m_flfirerate = MAXFIRERATE;
		if (m_flfirerate == MAXFIRERATE && pOwner->m_nButtons & IN_ATTACK)
			m_iWeaponState = WEAPON_STATE_FULLSPEED;
		else
		{
			m_iWeaponState = WEAPON_STATE_NOTFULLSPEED;
			m_bPlayingWoundSound = false;
		}
	}*/


	if (m_flfirerate < GetMaxFirerate())
		m_flfirerate = GetMaxFirerate();

	if (m_flfirerate == GetMaxFirerate() && pOwner->m_nButtons & IN_ATTACK)
		m_iWeaponState = WEAPON_STATE_FULLSPEED;
	else
	{
		m_iWeaponState = WEAPON_STATE_NOTFULLSPEED;
		m_bPlayingWoundSound = false;
	}

	
	UpdateWeaponSoundState();


	ConVarRef thirdperson("g_thirdperson");
	if (thirdperson.GetBool())
	{
		if (!Q_strcmp(STRING(GetModelName()), GetWorldModel()))
			SetModel(GetWorldModel());
	}
	else
	{
		if (!Q_strcmp(STRING(GetModelName()), GetViewModel()))
			SetModel(GetViewModel());
	}
	BaseClass::ItemPostFrame();
}
void CWeaponAR2::Equip(CBaseCombatCharacter *pOwner)
{
	BaseClass::Equip(pOwner);
	InitWoundSound();
}
void CWeaponAR2::UpdateWeaponSoundState(void)
{
	switch (m_iWeaponState)
	{
	case WEAPON_STATE_NOTFULLSPEED:
		{
			ShutdownWoundSound();
			break;
		}
	case WEAPON_STATE_FULLSPEED:
	{
		if (m_bPlayingWoundSound)
		{
			DevMsg("already playing woundsound\n");
			break;
		}
		if (!m_pWoundSound)
			break;
		CSoundEnvelopeController::GetController().SoundChangeVolume(m_pWoundSound, 1.0f, 1.0f);
		CSoundEnvelopeController::GetController().SoundChangePitch(m_pWoundSound, 300, 40.0f);
		m_bPlayingWoundSound = true;
		DevMsg("playing woundsound\n");
		break;
	}
	}
}
void CWeaponAR2::InitWoundSound(void)
{
	CPASAttenuationFilter filter(this);

	if (!m_pWoundSound)
	{
		m_pWoundSound = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Chaingun.Wine");
		CSoundEnvelopeController::GetController().Play(m_pWoundSound, 0.0f, 100);
	}
}
void CWeaponAR2::ShutdownWoundSound(void)
{
	if (m_pWoundSound)
	{
		CSoundEnvelopeController::GetController().SoundChangeVolume(m_pWoundSound, 0.0f, 0.0f);
		CSoundEnvelopeController::GetController().SoundChangePitch(m_pWoundSound, 100, 0.01f);
		
	}
}
void CWeaponAR2::DestroyWeaponSound(void)
{
	if (m_pWoundSound)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pWoundSound);
		m_pWoundSound = NULL;
	}
}
void CWeaponAR2::WeaponIdle(void)
{
	m_flfirerate += 0.01f;	
	BaseClass::WeaponIdle();
}
void CWeaponAR2::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	int iAttachment = LookupAttachment("muzzle");
	ConVarRef g_thirdperson("g_thirdperson");
	if (g_thirdperson.GetBool())
		DispatchParticleEffect("smg_core", PATTACH_POINT_FOLLOW, this, iAttachment, true);
	else
		DispatchParticleEffect("smg_core", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), iAttachment, true);


	// Abort here to handle burst and auto fire modes
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	//m_nShotsFired++;

	//pPlayer->DoMuzzleFlash();
	/*DevMsg("serverside muzzle R = %i\n", m_iMuzzleR);
	DevMsg("serverside muzzle G = %i\n", m_iMuzzleG);
	DevMsg("serverside muzzle B = %i\n", m_iMuzzleB);*/

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();

	// MUST call sound before removing a round from the clip of a CHLMachineGun
	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		if (iBulletsToFire > m_iClip1)
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}
	if (!UsesClipsForAmmo1())
	{
		pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	}
	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
	Vector vUp, vRight, vForward;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	Vector vecThirdPersonSrc;
	QAngle angThird;


	Vector vecSrc, vecDirShooting;

	if (g_thirdperson.GetBool())
	{
		//Vector vecBarrelPos = pPlayer->WorldSpaceCenter() + (vForward * 16.0f) + (vRight * 6.0f);
		int iMuzzleBone = this->LookupBone("muzzle");
		QAngle qAng;
		Vector vecBarrelPos;
		GetBonePosition(iMuzzleBone, vecBarrelPos, qAng);
		trace_t tr, tr2;
		UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + (vForward * MAX_TRACE_LENGTH), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
		UTIL_TraceLine(pPlayer->WorldSpaceCenter(), vecBarrelPos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr2);
		if (tr2.fraction == 1.0f) //if our barrel position isn't occluded
		{
			vecSrc = vecBarrelPos; //shoot from barrel position
		}
		else //if it is occluded
		{
			vecSrc = pPlayer->WorldSpaceCenter(); //move our shoot position back by the difference
		}

		vecDirShooting = (tr.endpos - vecSrc).Normalized();

	}
	else
	{
		Vector vecSrcPoint = pPlayer->Weapon_ShootPosition() + vRight * 4.0f + vUp * -5.0f + vForward * 8.0f;
		vecSrc = vecSrcPoint + RandomVector(-4, 4);

		vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	}
	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = vecSrc;
	info.m_vecDirShooting = vecDirShooting;
	info.m_vecSpread = GetBulletSpread();
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;
	info.m_pAttacker = pPlayer;
	FireActualBullet(info, 8000, GetTracerType());

	DoMuzzleFlashLight(vMuzzleFlashLightColor, vecSrc);

	//FireBullets(info);

	//Factor in the view kick
	AddViewKick();

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer);

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
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponAR2::GetPrimaryAttackActivity( void )
{
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeaponAR2::DoImpactEffect( trace_t &tr, int nDamageType )
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + ( tr.plane.normal * 1.0f );
	data.m_vNormal = tr.plane.normal;

	DispatchEffect( "AR2Impact", data );

	BaseClass::DoImpactEffect( tr, nDamageType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::DelayedAttack( void )
{
	m_bShotDelayed = false;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	// Deplete the clip completely
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	m_flNextSecondaryAttack = pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration();

	// Register a muzzleflash for the AI
	pOwner->DoMuzzleFlash();
	pOwner->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
	
	WeaponSound( WPN_DOUBLE );
	pOwner->RumbleEffect(RUMBLE_SHOTGUN_DOUBLE, 0, RUMBLE_FLAG_RESTART );

	// Fire the bullets
	Vector vecSrc	 = pOwner->Weapon_ShootPosition( );
	Vector vecAiming = pOwner->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	Vector impactPoint = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	// Fire the bullets
	Vector vecVelocity = vecAiming * 50000.0f;

	// Fire the combine ball
	CreateCombineBall(	vecSrc, 
						vecVelocity, 
						sk_weapon_ar2_alt_fire_radius.GetFloat(), 
						sk_weapon_ar2_alt_fire_mass.GetFloat(),
						sk_weapon_ar2_alt_fire_duration.GetFloat(),
						pOwner );

	// View effects
	color32 white = {255, 255, 255, 64};
	UTIL_ScreenFade( pOwner, white, 0.1, 0, FFADE_IN  );
	
	//Disorient the player
	QAngle angles = pOwner->GetLocalAngles();

	angles.x += random->RandomInt( -2, 2 );
	angles.y += random->RandomInt( -2, 2 );
	angles.z = 0;

	pOwner->SnapEyeAngles( angles );
	
	pOwner->ViewPunch( QAngle( random->RandomInt( -2, -4 ), random->RandomInt( 1, 2 ), 0 ) );

	// Decrease ammo
	pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.01f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::SecondaryAttack( void )
{
	while (m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		SendWeaponAnim(GetPrimaryAttackActivity());
		m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponAR2::CanHolster( void )
{
	if ( m_bShotDelayed )
		return false;

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
//-----------------------------------------------------------------------------
bool CWeaponAR2::Reload( void )
{
	if ( m_bShotDelayed )
		return false;

	return BaseClass::Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	Vector vecShootOrigin, vecShootDir;

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT( npc != NULL );

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
		AngleVectors( angShootDir, &vecShootDir );
	}
	else 
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );
	}

	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );

	// NOTENOTE: This is overriden on the client-side
	// pOperator->DoMuzzleFlash();

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	WeaponSound( WPN_DOUBLE );

	if ( !GetOwner() )
		return;
		
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if ( !pNPC )
		return;
	
	// Fire!
	Vector vecSrc;
	Vector vecAiming;

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecSrc, angShootDir );
		AngleVectors( angShootDir, &vecAiming );
	}
	else 
	{
		vecSrc = pNPC->Weapon_ShootPosition( );
		
		Vector vecTarget;

		CNPC_Combine *pSoldier = dynamic_cast<CNPC_Combine *>( pNPC );
		if ( pSoldier )
		{
			// In the distant misty past, elite soldiers tried to use bank shots.
			// Therefore, we must ask them specifically what direction they are shooting.
			vecTarget = pSoldier->GetAltFireTarget();
		}
		else
		{
			// All other users of the AR2 alt-fire shoot directly at their enemy.
			if ( !pNPC->GetEnemy() )
				return;
				
			vecTarget = pNPC->GetEnemy()->BodyTarget( vecSrc );
		}

		vecAiming = vecTarget - vecSrc;
		VectorNormalize( vecAiming );
	}

	Vector impactPoint = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	float flAmmoRatio = 1.0f;
	float flDuration = RemapValClamped( flAmmoRatio, 0.0f, 1.0f, 0.5f, sk_weapon_ar2_alt_fire_duration.GetFloat() );
	float flRadius = RemapValClamped( flAmmoRatio, 0.0f, 1.0f, 4.0f, sk_weapon_ar2_alt_fire_radius.GetFloat() );

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(	vecSrc, 
		vecVelocity, 
		flRadius, 
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		flDuration,
		pNPC );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	if ( bSecondary )
	{
		FireNPCSecondaryAttack( pOperator, true );
	}
	else
	{
		// Ensure we have enough rounds in the clip
		m_iClip1++;

		FireNPCPrimaryAttack( pOperator, true );
	}
}
void CWeaponAR2::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();
	m_flfirerate = MINFIRERATE;
	//UTIL_GetLocalPlayer()->SetMaxSpeed(450.0f);
	ShutdownWoundSound();
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{ 
		case EVENT_WEAPON_AR2:
			{
				FireNPCPrimaryAttack( pOperator, false );
			}
			break;

		case EVENT_WEAPON_AR2_ALTFIRE:
			{
				FireNPCSecondaryAttack( pOperator, false );
			}
			break;

		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.1f
	#define	MAX_VERTICAL_KICK	0.05f	//Degrees
	#define	SLIDE_LIMIT			1.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
		return;

	float flDuration = m_fFireDuration;

	if( g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE )
	{
		// On the 360 (or in any configuration using the 360 aiming scheme), don't let the
		// AR2 progressive into the late, highly inaccurate stages of its kick. Just
		// spoof the time to make it look (to the kicking code) like we haven't been
		// firing for very long.
		flDuration = MIN( flDuration, 0.75f );
	}

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, flDuration, SLIDE_LIMIT );
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponAR2::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 3.0,		0.85	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
