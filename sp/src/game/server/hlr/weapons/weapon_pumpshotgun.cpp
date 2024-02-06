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
#include "grenade_ar2.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "actual_bullet.h"
#include "hl2_shareddefs.h"
#include "hlr/hlr_shareddefs.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;
extern ConVar mat_classic_render;
extern ConVar g_classic_weapon_pos;

ConVar sk_shotgun_pellet_speed("sk_shotgun_pellet_speed", "6800", FCVAR_NONE);
ConVar g_shotgun_focus_speed("g_shotgun_focus_speed", "0.2");

class CWeaponPumpShotgun : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponPumpShotgun, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();

private:
	bool	m_bNeedPump;		// When emptied completely
	bool	m_bDelayedFire1;	// Fire primary when finished reloading
	bool	m_bDelayedFire2;	// Fire secondary when finished reloading
	bool	m_bAimingGrenade;
	bool	m_bFocused;

public:
	void	Precache(void);

	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector vitalAllyCone = VECTOR_CONE_5DEGREES;
		static Vector cone = VECTOR_CONE_6DEGREES;

		if (GetOwner() && (GetOwner()->Classify() == CLASS_PLAYER_ALLY_VITAL))
		{
			// Give Alyx's shotgun blasts more a more directed punch. She needs
			// to be at least as deadly as she would be with her pistol to stay interesting (sjb)
			return vitalAllyCone;
		}

		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 3; }

	virtual float			GetMinRestTime();
	virtual float			GetMaxRestTime();

	virtual float			GetFireRate(void);
	const char *GetTracerType(void) { return "AR2Tracer"; }

	bool StartReload(void);
	bool Reload(void);
	void FillClip(void);
	void FinishReload(void);
	void CheckHolsterReload(void);
	void Pump(void);
	void LaunchGrenade(void);
	//	void WeaponIdle( void );
	void ItemHolsterFrame(void);
	void ItemPostFrame(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void DryFire(void);
	Activity GetDrawActivity(void);

	void SendMessage(int msg);

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

	CWeaponPumpShotgun(void);
};

IMPLEMENT_SERVERCLASS_ST(CWeaponPumpShotgun, DT_WeaponPumpShotgun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_pumpshotgun, CWeaponPumpShotgun);
PRECACHE_WEAPON_REGISTER(weapon_pumpshotgun);

BEGIN_DATADESC(CWeaponPumpShotgun)

DEFINE_FIELD(m_bNeedPump, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDelayedFire1, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDelayedFire2, FIELD_BOOLEAN),

END_DATADESC()

acttable_t	CWeaponPumpShotgun::m_acttable[] =
{
	{ ACT_IDLE, ACT_IDLE_SMG1, true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD, ACT_RELOAD_SHOTGUN, false },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_SHOTGUN_AGITATED, false },//always aims

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

	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SHOTGUN, false },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, false },
};

IMPLEMENT_ACTTABLE(CWeaponPumpShotgun);

void CWeaponPumpShotgun::Precache(void)
{
	PrecacheScriptSound("Weapon_SMG1.Double");
	PrecacheParticleSystem("muzzleflash_orange_core");
	PrecacheParticleSystem("muzzleflash_shotgun_altfire");
	CBaseCombatWeapon::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);
	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
		AngleVectors(angShootDir, &vecShootDir);
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
	}

	pOperator->FireBullets(8, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
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
float CWeaponPumpShotgun::GetMinRestTime()
{
	if (hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE)
	{
		return 1.2f;
	}

	return BaseClass::GetMinRestTime();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CWeaponPumpShotgun::GetMaxRestTime()
{
	if (hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE)
	{
		return 1.5f;
	}

	return BaseClass::GetMaxRestTime();
}

//-----------------------------------------------------------------------------
// Purpose: Time between successive shots in a burst. Also returned for EP2
//			with an eye to not messing up Alyx in EP1.
//-----------------------------------------------------------------------------
float CWeaponPumpShotgun::GetFireRate()
{
	if (hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE)
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
bool CWeaponPumpShotgun::StartReload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
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
		m_bNeedPump = true;
	}

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	// Make shotgun shell visible
	SetBodygroup(1, 0);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponPumpShotgun::Reload(void)
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
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
	SendWeaponAnim(ACT_VM_RELOAD);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::FinishReload(void)
{
	// Make shotgun shell invisible
	SetBodygroup(1, 1);

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::FillClip(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	// Add them to the clip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		if (Clip1() < GetMaxClip1())
		{
			m_iClip1++;
			pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
		}
	}
}
Activity CWeaponPumpShotgun::GetDrawActivity(void)
{
		return ACT_VM_DRAW;
}
//-----------------------------------------------------------------------------
// Purpose: Play weapon pump anim
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::Pump(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bNeedPump = false;
	if (mat_classic_render.GetInt() == 0)
		WeaponSound(SPECIAL1);
	else
		WeaponSound(SPECIAL3);

	// Finish reload animation
	ConVarRef classicpos("r_classic_weapon_pos");

	classicpos.GetBool() ? SendWeaponAnim(ACT_VM_PULLBACK) : SendWeaponAnim(ACT_SHOTGUN_PUMP);

	pOwner->m_flNextAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::DryFire(void)
{
	WeaponSound(EMPTY);
	if (g_classic_weapon_pos.GetBool() == false)
		SendWeaponAnim(ACT_VM_DRYFIRE);
	else
		SendWeaponAnim(ACT_VM_IDLE_SPECIAL);

	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
 	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	if (mat_classic_render.GetInt() == 1)
		WeaponSound(SPECIAL2);
	else
		WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();
	DoMuzzleFlashLight(vMuzzleFlashLightColor, pPlayer->Weapon_ShootPosition());


	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);
	

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	Vector vecAtch;
	GetAttachment(LookupAttachment("muzzle"), vecAtch);
	ConVarRef classicpos("r_classic_weapon_pos");
	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	float right = classicpos.GetBool() ? 0.0f : 3.1f;
	Vector vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * right + vUp * -3.0f;
	Vector	vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);


	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 1.0);
	pPlayer->CreateMuzzleLight(255, 200, 0, vecSrc);
	// Fire the bullets, and force the first shot to be perfectly accuracy
	// Fire the bullets
	//pPlayer->FireBullets(8, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, true, false);

	/*FireBulletsInfo_t info;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_vecSrc = vecSrc;
	info.m_vecSpread = GetBulletSpread();
	info.m_vecDirShooting = vecAiming;
	info.m_pAttacker = GetOwnerEntity();*/
	if (pPlayer->HasOverdrive())
	{
		pPlayer->FireBullets(28, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, true, false);
		//info.m_iShots = 28;
		//FireActualBullet(info, sk_shotgun_pellet_speed.GetInt(), GetTracerType());
	}
	else
	{
		pPlayer->FireBullets(12, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, true, false);
		//info.m_iShots = 14;
		//FireActualBullet(info, sk_shotgun_pellet_speed.GetInt(), GetTracerType());
	}
	pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	pPlayer->ViewPunch(QAngle(random->RandomFloat(-2, -1), random->RandomFloat(-2, 2), 0));
	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);
	//int iAttachment = LookupAttachment("muzzle");
	DispatchParticleEffect("muzzleflash_orange_core", vecSrc, angAiming, this);
	/*
	for (int i = 0; i < 8; i++)
	{
		Vector vecSpread(random->RandomFloat(-0.08716, 0.08716), random->RandomFloat(-0.08716, 0.08716), random->RandomFloat(-0.08716, 0.08716));
		Vector vecSpreadShot = (vecAiming + vecSpread);
		UTIL_Tracer(vecSrc, vecSpread, 0, 0, 5, false, "GaussTracer", 0);
	}
	*/
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner());

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	if (m_iClip1 && !pPlayer->HasOverdrive())
	{
		// pump so long as some rounds are left.
		m_bNeedPump = true;
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::SecondaryAttack(void)
{
	//This is empty so the weapon registers having a secondary attack still, but the actual functionality is handled in ItemPostFrame()
}

void CWeaponPumpShotgun::LaunchGrenade(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	pPlayer->m_nButtons &= ~IN_ATTACK2;
	// MUST call sound before removing a round from the clip of a CMachineGun

	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	// Don't fire again until fire animation has completed
	//m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
	//m_iClip1 -= 2;	// Shotgun uses same clip for primary and secondary attacks
	if (m_flNextSecondaryAttack >= gpGlobals->curtime)
		return;
	ConVarRef classicpos("r_classic_weapon_pos");
	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	float right = classicpos.GetBool() ? 0.0f : 2.f;
	Vector vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * right + vUp * -3.0f;
	Vector	vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	// Don't autoaim on grenade tosses
	Vector vecThrow = vecAiming * 2000.0f + Vector(0,0,150);
	//Create the grenade
	QAngle angles;
	VectorAngles(vecThrow, angles);
	CGrenadeAR2* pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecSrc, angles, pPlayer);
	pGrenade->SetAbsVelocity(vecThrow);
	pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400));
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	pGrenade->SetThrower(GetOwner());
	pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	if (pPlayer->HasOverdrive())
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.75f;
	else
	{
		m_flNextSecondaryAttack = gpGlobals->curtime + 2.0f;
		m_bNeedPump = true;
	}
	SendWeaponAnim(GetSecondaryAttackActivity());
	EmitSound("Weapon_SMG1.Double");
	DispatchParticleEffect("muzzleflash_shotgun_altfire", vecSrc, pPlayer->EyeAngles(), this);

	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());
}
//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponPumpShotgun::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}
#ifndef HLR
	if (m_bInReload)
	{
		// If I'm primary firing and have one round stop reloading and fire
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_iClip1 >= 1))
		{
			m_bInReload = false;
			m_bNeedPump = false;
			m_bDelayedFire1 = true;
		}
		// If I'm secondary firing and have one round stop reloading and fire
		else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_iClip1 >= 2))
		{
			m_bInReload = false;
			m_bNeedPump = false;
			m_bDelayedFire2 = true;
		}
		else if (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
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
		SetBodygroup(1, 1);
	}
#endif
	if ((m_bNeedPump) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		Pump();
		return;
	}
#ifndef HLR
	// Shotgun uses same timing and ammo for secondary attack
	if ((m_bDelayedFire2 || pOwner->m_nButtons & IN_ATTACK2) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		m_bDelayedFire2 = false;

		if ((m_iClip1 <= 1 && UsesClipsForAmmo1()))
		{
			// If only one shell is left, do a single shot instead	
			if (m_iClip1 == 1)
			{
				PrimaryAttack();
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
			if (pOwner->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			SecondaryAttack();
		}
	}
	else if ((m_bDelayedFire1 || pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		m_bDelayedFire1 = false;
		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else
			{
				return;
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
			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
			if (pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	else
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if (!HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			// weapon isn't useable, switch.
			if (!(GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && pOwner->SwitchToNextBestWeapon(this))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip1 <= 0 && !(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->curtime)
			{
				if (StartReload())
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}
		}

		WeaponIdle();
		return;
	}
#endif

	if (pOwner->m_nButtons & IN_ATTACK2)
	{
		m_bAimingGrenade = true;
	}
	else
		m_bAimingGrenade = false;

	if (m_bAimingGrenade)
	{
		if (!m_bFocused)
		{
			pOwner->SetFOV(this, pOwner->GetDefaultFOV() - 15, g_shotgun_focus_speed.GetFloat());
			m_bFocused = true;
			pOwner->m_bFocused = true;
			pOwner->m_fFocusOffsetTime = g_shotgun_focus_speed.GetFloat();
			SendMessage(1);
			pOwner->m_bUseAltCrosshair = true;
		}
	}
	else if (m_bFocused)
	{
		pOwner->SetFOV(this, 0, g_shotgun_focus_speed.GetFloat());
		m_bFocused = false;
		pOwner->m_bFocused = false;
		SendMessage(0);
		pOwner->m_bUseAltCrosshair = false;
	}

	if (pOwner->m_nButtons & IN_ATTACK && gpGlobals->curtime > m_flNextPrimaryAttack && !m_bNeedPump)
	{
		if (m_bAimingGrenade)
			LaunchGrenade();
		else
			PrimaryAttack();
	}
}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponPumpShotgun::CWeaponPumpShotgun(void)
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
void CWeaponPumpShotgun::ItemHolsterFrame(void)
{
	// Must be player held
	if (GetOwner() && GetOwner()->IsPlayer() == false)
		return;

	// We can't be active
	if (GetOwner()->GetActiveWeapon() == this)
		return;

	if (m_bFocused)
	{
		m_bAimingGrenade = false;
		m_bFocused = false;
		CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
		pPlayer->SetFOV(this, 0, g_shotgun_focus_speed.GetFloat());
		pPlayer->m_bFocused = false;
		SendMessage(0);
		pPlayer->m_bUseAltCrosshair = false;
	}

	// If it's been longer than three seconds, reload
	if ((gpGlobals->curtime - m_flHolsterTime) > sk_auto_reload_time.GetFloat())
	{
		// Reset the timer
		m_flHolsterTime = gpGlobals->curtime;

		if (GetOwner() == NULL)
			return;

		m_bNeedPump = false;
	}
}


void CWeaponPumpShotgun::SendMessage(int msg)
{
	CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
	user.MakeReliable();
	UserMessageBegin(user, "FragLaunch");
	WRITE_BOOL(msg);
	MessageEnd();

}

//==================================================
// Purpose: 
//==================================================
/*
void CWeaponPumpShotgun::WeaponIdle( void )
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
}*/