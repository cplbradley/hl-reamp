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
#include "weapon_npc_chaingun.h"
#include "grenade_ar2.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "ai_memory.h"
#include "soundent.h"
#include "hl2_player.h"
#include "EntityFlame.h"
#include "weapon_flaregun.h"
#include "te_effect_dispatch.h"
#include "prop_combine_ball.h"
#include "beam_shared.h"
#include "npc_combine.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "hlr/util/hlr_projectile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================

ConVar sk_chaingun_maxburst("sk_chaingun_maxburst", "100", FCVAR_NONE);

BEGIN_DATADESC(CWeaponNPCChaingun)

DEFINE_FIELD(m_flDelayedFire, FIELD_TIME),
DEFINE_FIELD(m_bShotDelayed, FIELD_BOOLEAN),
//DEFINE_FIELD( m_nVentPose, FIELD_INTEGER ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponNPCChaingun, DT_WeaponNPCChaingun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_npc_chaingun, CWeaponNPCChaingun);
PRECACHE_WEAPON_REGISTER(weapon_npc_chaingun);

acttable_t	CWeaponNPCChaingun::m_acttable[] =
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
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
};

IMPLEMENT_ACTTABLE(CWeaponNPCChaingun);

CWeaponNPCChaingun::CWeaponNPCChaingun()
{
	m_fMinRange1 = 65;
	m_fMaxRange1 = 2048;

	m_fMinRange2 = 256;
	m_fMaxRange2 = 1024;

	m_nShotsFired = 0;
	m_nVentPose = -1;

	m_bAltFiresUnderwater = false;
}

void CWeaponNPCChaingun::Precache(void)
{
	BaseClass::Precache();

	UTIL_PrecacheOther("prop_combine_ball");
	UTIL_PrecacheOther("env_entity_dissolver");
	UTIL_PrecacheOther("hlr_pistolprojectile");
}

//-----------------------------------------------------------------------------
// Purpose: Handle grenade detonate in-air (even when no ammo is left)
//-----------------------------------------------------------------------------
void CWeaponNPCChaingun::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponNPCChaingun::GetPrimaryAttackActivity(void)
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
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeaponNPCChaingun::DoImpactEffect(trace_t &tr, int nDamageType)
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
	data.m_vNormal = tr.plane.normal;

	DispatchEffect("AR2Impact", data);

	BaseClass::DoImpactEffect(tr, nDamageType);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponNPCChaingun::CanHolster(void)
{
	if (m_bShotDelayed)
		return false;

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
//-----------------------------------------------------------------------------
bool CWeaponNPCChaingun::Reload(void)
{
	return BaseClass::Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponNPCChaingun::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();

	CAI_BaseNPC* npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

	vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	QAngle angAiming;
	WeaponSoundRealtime(SINGLE_NPC);
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angAiming);
	VectorAngles(vecShootDir, angAiming);
	CHLRPistolProjectile* pPew = (CHLRPistolProjectile*)CreateEntityByName("hlr_pistolprojectile");
	UTIL_SetOrigin(pPew, vecShootOrigin);
	float basespd = 2000.0f;
	float adjustedspeed = g_pGameRules->SkillAdjustValue(basespd);
	Vector vecVelocity = vecShootDir * adjustedspeed;
	pPew->Spawn();
	pPew->SetAbsVelocity(vecVelocity);
	//DispatchParticleEffect("pistol_npc_core", vecShootOrigin, angAiming, pOperator);
	//pOperator->FireBullets(0, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
	pOperator->DoMuzzleFlash();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponNPCChaingun::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
		m_iClip1++;
		FireNPCPrimaryAttack(pOperator, true);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponNPCChaingun::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
int CWeaponNPCChaingun::GetMinBurst()
{
	return sk_chaingun_maxburst.GetInt();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponNPCChaingun::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	8.0f	//Degrees
#define	SLIDE_LIMIT			5.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	float flDuration = m_fFireDuration;

	if (g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE)
	{
		// On the 360 (or in any configuration using the 360 aiming scheme), don't let the
		// AR2 progressive into the late, highly inaccurate stages of its kick. Just
		// spoof the time to make it look (to the kicking code) like we haven't been
		// firing for very long.
		flDuration = MIN(flDuration, 0.75f);
	}

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, flDuration, SLIDE_LIMIT);
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponNPCChaingun::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 3.0, 0.85 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}