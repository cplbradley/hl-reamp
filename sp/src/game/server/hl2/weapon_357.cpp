//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		357 - hand gun
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
#include "particle_parse.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "gamestats.h"
#include "beam_shared.h"
#define PHYSCANNON_BEAM_SPRITE "sprites/physbeam.vmt"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
extern ConVar sk_plr_357_pushamount;
//-----------------------------------------------------------------------------
// CWeapon357
//-----------------------------------------------------------------------------

class CWeapon357 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeapon357, CBaseHLCombatWeapon);
public:

	CWeapon357(void);

			void	PrimaryAttack(void);
	virtual void	SecondaryAttack(void);
	virtual void	ItemPostFrame(void);
	virtual void	ItemBusyFrame(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
			void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

private:
	void   	DrawBeam(void);
public:
	float	WeaponAutoAimScale()	{ return 0.6f; }
			void	Precache(void);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
private:
	void	ToggleZoom(void);
	void	CheckZoomToggle(void);
	bool				m_bInZoom;
	void	StopEffects(void);
};

LINK_ENTITY_TO_CLASS(weapon_357, CWeapon357);

PRECACHE_WEAPON_REGISTER(weapon_357);

IMPLEMENT_SERVERCLASS_ST(CWeapon357, DT_Weapon357)
END_SEND_TABLE()

BEGIN_DATADESC(CWeapon357)
	DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon357::CWeapon357(void)
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;
	m_bInZoom = false;
}
void CWeapon357::Precache(void)
{
	PrecacheParticleSystem("railgun_beam");
	BaseClass::Precache();
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon357::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	switch (pEvent->event)
	{
	case EVENT_WEAPON_RELOAD:
	{
		CEffectData data;

		// Emit six spent shells
		for (int i = 0; i < 6; i++)
		{
			data.m_vOrigin = pOwner->WorldSpaceCenter() + RandomVector(-4, 4);
			data.m_vAngles = QAngle(90, random->RandomInt(0, 360), 0);
			data.m_nEntIndex = entindex();

			DispatchEffect("ShellEject", data);
		}

		break;
	}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon357::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 14) //if we don't have enough charge to fire, don't let player fire
	{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.15;
			return;
	}

	m_iPrimaryAttacks++; //i shot, track that
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname()); //i shot, tell the gamestats i did that

	WeaponSound(SINGLE); //pew noise
	pPlayer->DoMuzzleFlash(); //muzzle flash

	SendWeaponAnim(ACT_VM_PRIMARYATTACK); //viewmodel animation
	pPlayer->SetAnimation(PLAYER_ATTACK1); //worldmodel animation

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5; //1.5 second delay between shots
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75; //0.75 second delay between zooms

	
	if (!pPlayer)
		return; //Always validate a pointer
	pPlayer->RemoveAmmo(15, m_iPrimaryAmmoType); //remove 15 charge from stock
	Vector vecAbsStart, vecAbsEnd, vecDir;
	Vector vecSrc = pPlayer->Weapon_ShootPosition(); //start at headlevel
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT); //autoaiming for lower difficulties
	AngleVectors(pPlayer->EyeAngles(), &vecDir);//convert angle into vector


	Vector vecRev = -vecDir;
	pPlayer->VelocityPunch(vecRev * sk_plr_357_pushamount.GetFloat());

	DrawBeam(); //trigger beam draw
	pPlayer->FireBullets(10, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0); //shoot 15 bullets as 1 bullet

	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();

	angles.x += random->RandomInt(0, 0);
	angles.y += random->RandomInt(0, 0);
	angles.z = 0;

	pPlayer->SnapEyeAngles(angles);

	//pPlayer->ViewPunch(QAngle(-8, random->RandomFloat(0, 2), 0));

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner());

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &startPos - where the beam should begin
//          &endPos - where the beam should end
//          width - what the diameter of the beam should be (units?)
//-----------------------------------------------------------------------------
void CWeapon357::DrawBeam(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return; //Always validate a pointer

	// Create Vector for direction
	Vector vecDir;

	// Take the Player's EyeAngles and turn it into a direction
	AngleVectors(pPlayer->EyeAngles(), &vecDir);

	// Get the Start/End
	Vector vecAbsStart = pPlayer->EyePosition();
	Vector vecAbsEnd = vecAbsStart + (vecDir * MAX_TRACE_LENGTH);

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	Vector vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * 3.0f + vUp * -3.0f;

	trace_t tr; // Create our trace_t class to hold the end result
	// Do the TraceLine, and write our results to our trace_t class, tr.
	UTIL_TraceLine(vecAbsStart, vecAbsEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr);

	Vector vecPartStart;
	Vector vecEndPos = tr.endpos;
	DispatchParticleEffect("railgun_beam", vecSrc, tr.endpos, GetAbsAngles(), this);
}
void CWeapon357::SecondaryAttack(void)
{

}
void CWeapon357::CheckZoomToggle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_ATTACK2)
	{
		ToggleZoom();
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon357::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon357::ItemPostFrame(void)
{
	// Allow zoom toggling
	CheckZoomToggle();
	BaseClass::ItemPostFrame();
}
void CWeapon357::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 45, 0.1f))
		{
			m_bInZoom = true;
		}
	}
}
bool CWeapon357::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	StopEffects();
	return BaseClass::Holster(pSwitchingTo);
}
void CWeapon357::StopEffects(void)
{
	// Stop zooming
	if (m_bInZoom)
	{
		ToggleZoom();
	}
}