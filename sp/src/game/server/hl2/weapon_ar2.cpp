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
#include "hlr/weapons/actual_bullet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#define MINFIRERATE 0.2f //5hz
#define MAXFIRERATE 0.0625f //16hz
#define MAXOVERDRIVEFIRERATE 0.03571f //28hz
#define MAXFOCUSFIRERATE 0.09091f //11hz

#define CHAINGUN_FULLSPEED_STANDARD "Chaingun.Fullspeed"
#define CHAINGUN_FULLSPEED_OVERDRIVE "Chaingun.Overdrive"
#define CHAINGUN_FULLSPEED_FOCUS "Chaingun.Focus"
#define CHAINGUN_SINGLE "Chaingun.Single"

enum {
	WEAPON_STATE_FULLSPEED,
	WEAPON_STATE_NOTFULLSPEED
};
//=========================================================
//=========================================================

BEGIN_DATADESC( CWeaponChaingun )
	
	DEFINE_FIELD( m_flfirerate,		FIELD_FLOAT),
	DEFINE_SOUNDPATCH(m_pWoundSound),
	DEFINE_SOUNDPATCH(m_pChaingun),
	DEFINE_SOUNDPATCH(m_pOverdrive),
	DEFINE_SOUNDPATCH(m_pFocus),
	DEFINE_FIELD(m_iWeaponState, FIELD_INTEGER),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponChaingun, DT_WeaponChaingun)

END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_chaingun, CWeaponChaingun);
PRECACHE_WEAPON_REGISTER(weapon_chaingun);

acttable_t	CWeaponChaingun::m_acttable[] =
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

IMPLEMENT_ACTTABLE(CWeaponChaingun);

CWeaponChaingun::CWeaponChaingun( )
{
	m_nShotsFired	= 0;
	m_flfirerate = MINFIRERATE;

	m_fPitch = 100.0f;

	bActive = false;
	iNumShots = 0;
	vMuzzleFlashLightColor = Vector(0, 128, 250);

	m_bPlayingWoundSound = false;
	m_iWeaponState = WEAPON_STATE_NOTFULLSPEED;
	
	//InitWoundSound();
}


float CWeaponChaingun::GetMaxFirerate()
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner->HasOverdrive())
		return MAXOVERDRIVEFIRERATE;
	else if (AmFocusFiring())
		return MAXFOCUSFIRERATE;
	else
		return MAXFIRERATE;
}

Vector CWeaponChaingun::GetSpread(void)
{
	Vector cone;

	if (AmFocusFiring())
		cone = VECTOR_CONE_1DEGREES;
	else
	{
		cone = VECTOR_CONE_8DEGREES * (1.0f - (iNumShots / 50) * 0.6f);
	}

	return cone;
}
void CWeaponChaingun::Precache( void )
{
	BaseClass::Precache();
	PrecacheScriptSound("Chaingun.Whine");
	PrecacheScriptSound(CHAINGUN_FULLSPEED_STANDARD);
	PrecacheScriptSound(CHAINGUN_FULLSPEED_FOCUS);
	PrecacheScriptSound(CHAINGUN_FULLSPEED_OVERDRIVE);
	PrecacheScriptSound(CHAINGUN_SINGLE);
	PrecacheParticleSystem("smg_core");
}


bool CWeaponChaingun::AmFocusFiring(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner->m_nButtons & IN_ATTACK2)
		return true;
	else
		return false;
}

void CWeaponChaingun::ToggleZoom(void)
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
void CWeaponChaingun::ItemPostFrame(void)
{
	// Update our pose parameter for the vents
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		ShutdownAllSounds();

	if (pOwner->m_nButtons & IN_ATTACK)
	{

		m_flfirerate -= 0.003f;
	}

	if (~pOwner->m_nButtons & IN_ATTACK && bShooting)
	{
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		bShooting = false;
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

	if (AmFocusFiring())
		iNumShots = 70;

	if (m_flfirerate < GetMaxFirerate())
		m_flfirerate = GetMaxFirerate();

	if (m_flfirerate == GetMaxFirerate() && pOwner->m_nButtons & IN_ATTACK)
	{
		m_iWeaponState = WEAPON_STATE_FULLSPEED;
	}
	else
	{
		iNumShots--;
		m_iWeaponState = WEAPON_STATE_NOTFULLSPEED;
		m_bPlayingWoundSound = false;
	}

	if (iNumShots < 0)
		iNumShots = 0;
	else if (!AmFocusFiring() && iNumShots > 50)
		iNumShots = 50;

	bActive = pOwner->GetActiveWeapon() == this;

	UpdateWeaponSoundState();
	SendData();

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
void CWeaponChaingun::Equip(CBaseCombatCharacter *pOwner)
{
	BaseClass::Equip(pOwner);
	InitWoundSound();
}
void CWeaponChaingun::UpdateWeaponSoundState(void)
{
	switch (m_iWeaponState)
	{
	case WEAPON_STATE_NOTFULLSPEED:
		{
		ShutdownAllSounds();
			break;
		}
	case WEAPON_STATE_FULLSPEED:
	{
		CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return;

		//InitWoundSound();

		if (pPlayer->HasOverdrive())
		{
			CSoundEnvelopeController::GetController().Play(m_pOverdrive, 1.0f, 100.0f);
			ShutdownChaingunSound();
			ShutdownFocusSound();
		}
		else if (AmFocusFiring())
		{
			CSoundEnvelopeController::GetController().Play(m_pFocus, 1.0f, 100.0f);
			ShutdownOverdriveSound();
			ShutdownChaingunSound();
		}
		else
		{
			CSoundEnvelopeController::GetController().Play(m_pChaingun, 1.0f, 100.0f);
			ShutdownFocusSound();
			ShutdownOverdriveSound();
		}
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
void CWeaponChaingun::InitWoundSound(void)
{
	CPASAttenuationFilter filter(this);

	if (!m_pWoundSound)
	{
		m_pWoundSound = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Chaingun.Whine");
		CSoundEnvelopeController::GetController().Play(m_pWoundSound, 0.0f, 100);
	}
	if (!m_pChaingun)
		m_pChaingun = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Chaingun.Fullspeed");
	if (!m_pFocus)
		m_pFocus = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Chaingun.Focus");
	if (!m_pOverdrive)
		m_pOverdrive = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Chaingun.Overdrive");
}
void CWeaponChaingun::ShutdownWoundSound(void)
{
	if (m_pWoundSound)
	{
		CSoundEnvelopeController::GetController().SoundChangePitch(m_pWoundSound, 100, 0.0f);
		CSoundEnvelopeController::GetController().SoundChangeVolume(m_pWoundSound, 0.0f, 0.0f);
	}
}

void CWeaponChaingun::ShutdownChaingunSound()
{
	if (m_pChaingun)
		CSoundEnvelopeController::GetController().Shutdown(m_pChaingun);
}
void CWeaponChaingun::ShutdownFocusSound()
{
	if (m_pFocus)
		CSoundEnvelopeController::GetController().Shutdown(m_pFocus);
}
void CWeaponChaingun::ShutdownOverdriveSound()
{
	if (m_pOverdrive)
		CSoundEnvelopeController::GetController().Shutdown(m_pOverdrive);
}
void CWeaponChaingun::DestroyWeaponSound(void)
{
	if (m_pWoundSound)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pWoundSound);
		m_pWoundSound = NULL;
	}
}

void CWeaponChaingun::ShutdownAllSounds()
{
	ShutdownChaingunSound();
	ShutdownFocusSound();
	ShutdownOverdriveSound();
	ShutdownWoundSound();
}
void CWeaponChaingun::WeaponIdle(void)
{
	m_flfirerate += 0.01f;	
	BaseClass::WeaponIdle();
}
void CWeaponChaingun::PrimaryAttack(void)
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
		if (m_iWeaponState != WEAPON_STATE_FULLSPEED)
			WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
		bShooting = true;
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

	if(GetFireRate() == GetMaxFirerate() && !AmFocusFiring())
		iNumShots++;

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
	info.m_vecSpread = GetSpread();
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;
	info.m_pAttacker = pPlayer;

	if (!AmFocusFiring())
		FireActualBullet(info, 8000, GetTracerType());
	else
		pPlayer->FireBullets(info);

	pPlayer->CreateMuzzleLight(0, 150, 255,vecSrc);

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
Activity CWeaponChaingun::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChaingun::SecondaryAttack( void )
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	while (m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		if(!(pOwner->m_nButtons & IN_ATTACK))
		SendWeaponAnim(GetSecondaryAttackActivity());
		m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
	}
}

void CWeaponChaingun::SendData()
{
	CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
	user.MakeReliable();
	UserMessageBegin(user, "HudChaingun");
	WRITE_BOOL(bActive);
	WRITE_BYTE(iNumShots);
	MessageEnd();
}
void CWeaponChaingun::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();
	m_flfirerate = MINFIRERATE;
	iNumShots = 0;
	bActive = false;
	SendData();
	//UTIL_GetLocalPlayer()->SetMaxSpeed(450.0f);
	ShutdownAllSounds();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChaingun::AddViewKick( void )
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
const WeaponProficiencyInfo_t * CWeaponChaingun::GetProficiencyValues()
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
