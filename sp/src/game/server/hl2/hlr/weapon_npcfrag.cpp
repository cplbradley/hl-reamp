
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
#include "grenade_frag.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
ConVar sk_npc_grenadelauncher_speed("sk_npc_grenadelauncher_speed", "700", FCVAR_REPLICATED);
ConVar sk_npc_grenadelauncher_distmult("sk_npc_grenadelauncher_distmult", "5", FCVAR_REPLICATED);

class CWeaponNPCFrag : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponNPCFrag, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();

	void Precache(void);
	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 1; }

	virtual float			GetFireRate() { return 2; }


	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();
	CWeaponNPCFrag(void);
};

IMPLEMENT_SERVERCLASS_ST(CWeaponNPCFrag, DT_WeaponNPCFrag)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_fraglauncher, CWeaponNPCFrag);
PRECACHE_WEAPON_REGISTER(weapon_fraglauncher);

BEGIN_DATADESC(CWeaponNPCFrag)

END_DATADESC()

acttable_t	CWeaponNPCFrag::m_acttable[] =
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
};

IMPLEMENT_ACTTABLE(CWeaponNPCFrag);

void CWeaponNPCFrag::Precache(void)
{
	CBaseCombatWeapon::Precache();
	UTIL_PrecacheOther("npc_handgrenade");
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponNPCFrag::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);
	

	float basespd = sk_npc_grenadelauncher_speed.GetFloat();
	float adjustedspd = g_pGameRules->AdjustProjectileSpeed(basespd);

	Vector vecTarget;
	
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	vecShootDir = npc->GetEnemy()->WorldSpaceCenter();
	UTIL_PredictedPosition(npc->GetEnemy(), 1.0f, &vecTarget);
	Vector vecMins = -Vector(4, 4, 4);
	Vector vecMaxs = Vector(4, 4, 4);
	//Vector vecLaunch = VecCheckToss(this, vecShootOrigin, vecTarget, -1, 1.0f, true, &vecMins, &vecMaxs);
	Vector vecToss = VecCheckThrow(this, vecShootOrigin, vecTarget, adjustedspd, 1.0, &vecMins, &vecMaxs);
	Vector vecSpin;
	vecSpin.x = random->RandomFloat(-1000.0, 1000.0);
	vecSpin.y = random->RandomFloat(-1000.0, 1000.0);
	vecSpin.z = random->RandomFloat(-1000.0, 1000.0);
	if (vecToss != vec3_origin)
	{
		Fraggrenade_Create(vecShootOrigin, vec3_angle, vecToss, vecSpin, npc, 3.0f, true);
		/*CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::Create("npc_frag_grenade",vecShootOrigin,vec3_angle,npc);
		pGrenade->SetTimer(3.0f, 1.5f);
		pGrenade->SetVelocity(vecToss, vecSpin);
		pGrenade->SetThrower(npc);
		pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
		pGrenade->SetCombineSpawned(true);*/
		WeaponSound(SINGLE_NPC);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	else
	{
		return;

	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponNPCFrag::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
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
void CWeaponNPCFrag::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
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


CWeaponNPCFrag::CWeaponNPCFrag(void)
{

	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 10000;
	m_fMinRange2 = 0.0;
	m_fMaxRange2 = 500;
}