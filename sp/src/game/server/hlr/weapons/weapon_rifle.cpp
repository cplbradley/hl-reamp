//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Pistol - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "particle_parse.h"
#include "hl2_player.h"
#include "actual_bullet.h"
#include "hlr/hlr_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	RIFLE_FASTEST_REFIRE_TIME		0.1f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

//-----------------------------------------------------------------------------
// CWeaponRifle
//-----------------------------------------------------------------------------

class CWeaponRifle : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponRifle, CBaseHLCombatWeapon);

	CWeaponRifle(void);

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	ItemPostFrame(void);
	void	ItemPreFrame(void);
	void	ItemBusyFrame(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);

	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	bool m_bInZoom;

	void	ToggleZoom(void);
	void CheckZoomToggle(void);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity(void);

	const char *GetTracerType(void) { return "AirboatGunHeavyTracer"; }

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		cone = VECTOR_CONE_0DEGREES;

		return cone;
	}

	virtual int	GetMinBurst()
	{
		return 1;
	}

	virtual int	GetMaxBurst()
	{
		return 3;
	}

	virtual float GetFireRate(void)
	{
		return 0.5f;
	}

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	int tracermodelindex;
};


IMPLEMENT_SERVERCLASS_ST(CWeaponRifle, DT_WeaponRifle)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_rifle, CWeaponRifle);
PRECACHE_WEAPON_REGISTER(weapon_rifle);

BEGIN_DATADESC(CWeaponRifle)

DEFINE_FIELD(m_flSoonestPrimaryAttack, FIELD_TIME),
DEFINE_FIELD(m_flLastAttackTime, FIELD_TIME),
DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),

END_DATADESC()

acttable_t	CWeaponRifle::m_acttable[] =
{
	{ ACT_IDLE, ACT_IDLE_PISTOL, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_PISTOL, true },
	{ ACT_RANGE_ATTACK1, ACT_HLR_FIRE_RIFLE, true },
	{ ACT_RELOAD, ACT_RELOAD_PISTOL, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_PISTOL, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_PISTOL, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_PISTOL_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_PISTOL_LOW, false },
	{ ACT_COVER_LOW, ACT_COVER_PISTOL_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_PISTOL_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_PISTOL, false },
	{ ACT_WALK, ACT_WALK_PISTOL, false },
	{ ACT_RUN, ACT_RUN_PISTOL, false },


	{ ACT_HL2MP_IDLE, ACT_HLR_IDLE_RIFLE, false },
	{ ACT_HL2MP_RUN, ACT_HLR_RUN_RIFLE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HLR_FIRE_RIFLE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_RANGE_ATTACK1, ACT_HLR_FIRE_RIFLE, true },
};


IMPLEMENT_ACTTABLE(CWeaponRifle);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponRifle::CWeaponRifle(void)
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;

	m_fMinRange1 = 24;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 24;
	m_fMaxRange2 = 200;

	m_bFiresUnderwater = true;
	m_bInZoom = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRifle::Precache(void)
{
	BaseClass::Precache();
	PrecacheParticleSystem("muzzleflash_orange_core_model");
	tracermodelindex = PrecacheModel("models/utils/player_heavytracer.mdl");
	PrecacheParticleSystem("bullettrail");
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponRifle::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponRifle::PrimaryAttack(void)
{
	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + RIFLE_FASTEST_REFIRE_TIME;
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner());

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	pOwner->HasOverdrive() ? m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f : m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	int iAttachment = LookupAttachment("muzzle");
	Vector vecSrc;
	
	Vector vUp, vRight, vForward;
	pOwner->EyeVectors(&vForward, &vRight, &vUp);
	if (!m_bInZoom)
		vecSrc = pOwner->Weapon_ShootPosition() + vRight * 5.5f + vUp * -6.0f;
	else
		vecSrc = pOwner->WorldSpaceCenter();
	if (!m_bInZoom)
	{
		if(g_thirdperson.GetBool())
			DispatchParticleEffect("muzzleflash_orange_core_model", PATTACH_POINT_FOLLOW, this, iAttachment, true);
		else
			DispatchParticleEffect("muzzleflash_orange_core_model", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), iAttachment, true);

		QAngle punch = QAngle(random->RandomFloat(-2, -1), random->RandomFloat(-2, 2), 0);
		pOwner->AbsViewPunch(punch / 2);
	}
	else
	{
		QAngle punch = QAngle(random->RandomFloat(-2, -1), random->RandomFloat(-2, 2), 0);
		pOwner->AbsViewPunch(punch / 2);
		//pOwner->SnapEyeAngles(pOwner->EyeAngles() + (punch / 2));
	}
	Vector vecAbsStart = pOwner->EyePosition();
	Vector vecAbsEnd = vecAbsStart + (vForward * MAX_TRACE_LENGTH);
	Vector vecAiming = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DIRECT_ONLY);
	trace_t tr;
	UTIL_TraceLine(vecAbsStart, vecAbsEnd, MASK_SOLID_BRUSHONLY, pOwner, COLLISION_GROUP_NONE, &tr);
	Vector vecDir = (tr.endpos - vecSrc).Normalized();
	FireBulletsInfo_t info;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iShots = 1;
	info.m_vecSrc = vecAbsStart;
	info.m_vecDirShooting = vecAiming;
	info.m_vecSpread = GetBulletSpread();
	info.m_pAttacker = GetOwnerEntity();
	info.m_iDamageType = DMG_BULLET | DMG_DIRECT;
	FireActualBullet(info, 12000, true, NULL, tracermodelindex,true,vecSrc,vecDir);

	pOwner->CreateMuzzleLight(255, 200, 0,vecSrc);

	//pOwner->FireBullets(2, vecSrc, vecDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, -1, -1, 0, NULL, false, false);
	
	SendWeaponAnim(GetPrimaryAttackActivity());
	WeaponSound(SINGLE);
	m_iPrimaryAttacks++;
	pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
	gamestats->Event_WeaponFired(pOwner, true, GetClassname());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRifle::ItemPreFrame(void)
{
	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRifle::ItemBusyFrame(void)
{
	CheckZoomToggle();
	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponRifle::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	if (m_bInReload)
		return;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (((pOwner->m_nButtons & IN_ATTACK) == false) && (m_flSoonestPrimaryAttack < gpGlobals->curtime))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}

	CheckZoomToggle();
}
void CWeaponRifle::SecondaryAttack(void)
{
}
void CWeaponRifle::CheckZoomToggle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_ATTACK2)
	{
		ToggleZoom();
	}
}
void CWeaponRifle::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	CHL2_Player *phl2 = dynamic_cast<CHL2_Player*>(pPlayer);
	CBaseViewModel *pViewModel = pPlayer->GetViewModel();
	StopParticleEffects(pPlayer->GetViewModel());
	if (pPlayer == NULL)
		return;

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;
			phl2->m_HL2Local.m_bZooming = false;
			pViewModel->RemoveEffects(EF_NODRAW);
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();
		}
		if (g_thirdperson.GetBool())
			engine->ClientCommand(pPlayer->edict(), "thirdperson\n");
	}
	else
	{
		if (pPlayer->SetFOV(this, 35, 0.1f))
		{
			m_bInZoom = true;
			phl2->m_HL2Local.m_bZooming = true;
			pViewModel->AddEffects(EF_NODRAW);
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();
		}
		if (g_thirdperson.GetBool())
			engine->ClientCommand(pPlayer->edict(), "firstperson\n");
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeaponRifle::GetPrimaryAttackActivity(void)
{
	int i = RandomInt(0, 2);
	switch (i)
	{
	case 0:
	{
		return ACT_VM_PRIMARYATTACK;
		break;
	}
	case 1:
	{
		return ACT_VM_RECOIL1;
		break;
	}
	case 2:
	{
		return ACT_VM_RECOIL2;
		break;
	}
	default:
	{
		return ACT_VM_PRIMARYATTACK;
		break;
	}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRifle::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	StopParticleEffects(pOwner->GetViewModel());
	if (m_bInZoom)
	{
		ToggleZoom();
	}
	return BaseClass::Holster(pSwitchingTo);
}