//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Crowbar - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basecombatcharacter.h"
#include "basecombatweapon.h"
#include "basehlcombatweapon.h"
#include "hl2_player.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "rumble_shared.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_crowbar.h"
#include "gamestats.h"
#define FURYBAR_RANGE
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponFurybar : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponFurybar, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponFurybar();
	
	float		GetRange(void)		{ return	CROWBAR_RANGE; }
	float		GetFireRate(void)		{ return	CROWBAR_REFIRE; }
	void		Equip(CBaseCombatCharacter *pOwner);
	bool		Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	void		ItemPostFrame(void);
	void		Precache(void);
	void		End(void);
	void		ActivateTimer(void);
	void		Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary);
	float		m_fSwitchTime; 
	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);
	


	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);
	void		SecondaryAttack(void)	{ return; }

	// Animation event
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
};

IMPLEMENT_SERVERCLASS_ST(CWeaponFurybar, DT_WeaponFurybar)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS(weapon_furybar, CWeaponFurybar);
PRECACHE_WEAPON_REGISTER(weapon_furybar);
#endif

acttable_t CWeaponFurybar::m_acttable[] =
{
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, false },
};

IMPLEMENT_ACTTABLE(CWeaponFurybar);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponFurybar::CWeaponFurybar(void)
{
	
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponFurybar::GetDamageForActivity(Activity hitActivity)
{
	return 1000.0f;
}
void CWeaponFurybar::Precache(void)
{
	BaseClass::Precache();
	PrecacheScriptSound("Furybar.Hit");
	PrecacheScriptSound("Powerup.Pickup");
}
//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponFurybar::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}


//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------

int CWeaponFurybar::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity();

	// Project where the enemy will be in a little while
	float dt = 0.9f;
	dt += random->RandomFloat(-0.3f, 0.2f);
	if (dt < 0.0f)
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA(pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos);

	Vector vecDelta;
	VectorSubtract(vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta);

	if (fabs(vecDelta.z) > 70)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D();
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize(vecDelta.AsVector2D());
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D(vecDelta.AsVector2D(), vecForward.AsVector2D());
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


void CWeaponFurybar::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
	if (gpGlobals->curtime >= m_fSwitchTime)
	{
		End();
	}
}
//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponFurybar::HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors(GetAbsAngles(), &vecDirection);

	CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
	if (pEnemy)
	{
		Vector vecDelta;
		VectorSubtract(pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta);
		VectorNormalize(vecDelta);

		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize(vecDelta2D);
		if (DotProduct2D(vecDelta2D, vecDirection.AsVector2D()) > 0.8f)
		{
			vecDirection = vecDelta;
		}
	}

	Vector vecEnd;
	VectorMA(pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd);
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack(pOperator->Weapon_ShootPosition(), vecEnd,
		Vector(-16, -16, -16), Vector(36, 36, 36), 1000.0f, DMG_CLUB, 0.75);

	// did I hit someone?
	if (pHurt)
	{
		// play sound
		WeaponSound(MELEE_HIT);

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine(pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit);
		ImpactEffect(traceHit);
	}
	else
	{
		WeaponSound(MELEE_MISS);
	}
}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponFurybar::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit(pEvent, pOperator);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
void CWeaponFurybar::Equip(CBaseCombatCharacter *pOwner)
{
	if (!pOwner)
		return;
	if (pOwner->IsPlayer() == true)
	{
		ActivateTimer();
	}
	EmitSound("Powerup.Pickup");
	BaseClass::Equip(pOwner);
}
bool CWeaponFurybar::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (gpGlobals->curtime < m_fSwitchTime)
		return false;
	
	return BaseClass::Holster(pSwitchingTo);
}
void CWeaponFurybar::ActivateTimer(void)
{
	color32 red = { 200, 0, 0, 128 };
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	pPlayer->SetMaxSpeed(650.0f);
	UTIL_ScreenFade(pPlayer, red, 0.5, 0, FFADE_IN);
	m_fSwitchTime = gpGlobals->curtime + 30.0f;
}
void CWeaponFurybar::End(void)
{
	color32 red = { 200, 0, 0, 128 };
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	CBaseCombatWeapon *pLast = pPlayer->Weapon_GetLast();
	if (pLast)
	{
		pPlayer->SetActiveWeapon(pLast);
		pLast->m_flNextPrimaryAttack = (gpGlobals->curtime + 0.5f);
	}
		
	pPlayer->RemovePlayerItem(this);
	pPlayer->SetMaxSpeed(450.0f);
	UTIL_ScreenFade(pPlayer, red, 0.5, 0, FFADE_IN);

}