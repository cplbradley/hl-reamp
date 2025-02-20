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
#include "hl2_shareddefs.h"
#include "hl2_player.h"
#include "hlr/hlr_shareddefs.h"
#define PHYSCANNON_BEAM_SPRITE "sprites/physbeam.vmt"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


enum ChargeState_t {
	CHARGESTATE_IDLE,
	CHARGESTATE_CHARGING,
	CHARGESTATE_CHARGED,
};
extern ConVar sk_plr_357_pushamount;
extern ConVar mat_classic_render;
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
	void StartCharging();
	void ReleaseChargedShot();
	void ResetFOV();
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
			void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
			void    GetControlPanelInfo(int nPanelIndex, const char *&pPanelName);
			ChargeState_t GetChargeState() { return state; }

private:
	void   	DrawBeam(void);
	void DrawLargeBeam(void);

	bool m_bCharging;
	bool m_bCharged;
	float m_fDamageScale;
	float m_fFullyChargedTime;
	float m_fSoonestChargeShot;
	float m_fNextRemove;
	float m_fStartTime;

	ChargeState_t state;

public:
	float	WeaponAutoAimScale()	{ return 0.6f; }
			void	Precache(void);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();
private:
	void	ToggleZoom(void);
	void	CheckZoomToggle(void);
	bool				m_bInZoom;
	void	StopEffects(void);
};

LINK_ENTITY_TO_CLASS(weapon_357, CWeapon357);
LINK_ENTITY_TO_CLASS(weapon_railgun, CWeapon357);

PRECACHE_WEAPON_REGISTER(weapon_357);

IMPLEMENT_SERVERCLASS_ST(CWeapon357, DT_Weapon357)
END_SEND_TABLE()

BEGIN_DATADESC(CWeapon357)
	DEFINE_FIELD(m_bInZoom, FIELD_BOOLEAN),
END_DATADESC()

acttable_t CWeapon357::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SMG1, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SMG1, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SMG1, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, false },
};

IMPLEMENT_ACTTABLE(CWeapon357);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon357::CWeapon357(void)
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;
	m_bInZoom = false;
	m_fDamageScale = 1.f;
}
void CWeapon357::Precache(void)
{
	PrecacheParticleSystem("railgun_beam");
	PrecacheParticleSystem("railgun_overdrive");
	PrecacheParticleSystem("railgun_chargeup");
	PrecacheScriptSound("railgun.overdrive");
	PrecacheScriptSound("Railgun.Chargeup");
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

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 14 && GetChargeState() == CHARGESTATE_IDLE) //if we don't have enough charge to fire, don't let player fire
	{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 1.f;
			return;
	}

	m_iPrimaryAttacks++; //i shot, track that
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname()); //i shot, tell the gamestats i did that

	 //pew noise
	pPlayer->DoMuzzleFlash(); //muzzle flash

	SendWeaponAnim(ACT_VM_PRIMARYATTACK); //viewmodel animation
	pPlayer->SetAnimation(PLAYER_ATTACK1); //worldmodel animation

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5; //1.5 second delay between shots
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75; //0.75 second delay between zooms
	
	if(GetChargeState() == CHARGESTATE_IDLE)
		pPlayer->RemoveAmmo(15, m_iPrimaryAmmoType); //remove 15 charge from stock

	Vector vecAbsStart, vecAbsEnd, vecDir;
	Vector vecSrc = pPlayer->Weapon_ShootPosition(); //start at headlevel
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT); //autoaiming for lower difficulties
	AngleVectors(pPlayer->EyeAngles(), &vecDir);//convert angle into vector

	float downvel = pPlayer->GetAbsVelocity().z; //get downward velocity
	float dot = DotProduct(pPlayer->GetAbsVelocity(), vecDir); //get dot product of shoot angle and movement angle
	Vector vecRev = -vecDir;

	float push = sk_plr_357_pushamount.GetFloat();

	if ((dot > 0.8f) && (downvel < 0)) //if we're falling and looking down
		push += abs(downvel); //add the abs of downard velocity

	Vector vecForward = vecDir;
	vecForward[2] = 0;

	float viewdot = DotProduct(vecForward, vecDir);

	if (viewdot > 0.9) //if i'm looking straight
		pPlayer->VelocityPunch(Vector(0, 0, 200));

	//Msg("view x %f view y %f view z %f forx %f fory %f forz %f viewdot %f\n", vecDir.x, vecDir.y, vecDir.z, vecForward.x, vecForward.y, vecForward.z,viewdot);

	if (GetChargeState() == CHARGESTATE_CHARGED)
		push *= 2;

	if (pPlayer->HasOverdrive())
	{
		pPlayer->VelocityPunch(vecRev * (push * 2));
		DrawLargeBeam(); //trigger beam draw
		pPlayer->FireBullets(30, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0); //shoot 15 bullets as 1 bullet
		EmitSound("railgun.overdrive");
	}
	else
	{
		pPlayer->VelocityPunch(vecRev * push);

		if (GetChargeState() != CHARGESTATE_CHARGED)
			DrawBeam(); //trigger beam draw
		else
			DrawLargeBeam();

		FireBulletsInfo_t info;
		info.m_flDamage = 100.f * m_fDamageScale;
		info.m_iDamageType = DMG_DISSOLVE | DMG_BULLET;
		info.m_iShots = 1;
		info.m_iTracerFreq = 0;
		info.m_vecSrc = vecSrc;
		info.m_vecDirShooting = vecAiming;
		info.m_vecSpread = vec3_origin;
		info.m_flDistance = MAX_TRACE_LENGTH;
		info.m_iAmmoType = m_iPrimaryAmmoType;

		pPlayer->FireBullets(info); //shoot 15 bullets as 1 bullet
		if (mat_classic_render.GetInt() == 0)
			WeaponSound(SINGLE);
		else
			WeaponSound(SPECIAL1);
	}
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
	
	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();

	angles.x += random->RandomInt(0, 0);
	angles.y += random->RandomInt(0, 0);
	angles.z = 0;

	pPlayer->SnapEyeAngles(angles);

	pPlayer->ViewPunch(QAngle(-8, random->RandomFloat(0, 2), 0));

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

	Vector	vecSrc, vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	if (!m_bInZoom)
		vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * 3.0f + vUp * -3.0f;
	else
		vecSrc = pPlayer->Weapon_ShootPosition() + vUp * -6.0f;

	trace_t tr; // Create our trace_t class to hold the end result
	// Do the TraceLine, and write our results to our trace_t class, tr.
	UTIL_TraceLine(vecAbsStart, vecAbsEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	Vector vecPartStart;
	Vector vecEndPos = tr.endpos;
	DispatchParticleEffect("railgun_beam", vecSrc, tr.endpos, GetAbsAngles(), this);
}
void CWeapon357::DrawLargeBeam(void)
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
	DispatchParticleEffect("railgun_overdrive", vecSrc, tr.endpos, GetAbsAngles(), this);
}
void CWeapon357::SecondaryAttack(void)
{

}
void CWeapon357::CheckZoomToggle(void)
{
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
	CBasePlayer* player = ToBasePlayer(GetOwner());

	if (!player)
		return;

	engine->Con_NPrintf(0, "DamageScale %f Actual Damage %f Charge State %i sequence length %f", m_fDamageScale, 100.f * m_fDamageScale,GetChargeState(),m_fFullyChargedTime - m_fStartTime);

	if (player->m_nButtons & IN_ATTACK2 && gpGlobals->curtime > m_flNextPrimaryAttack)
	{
		if (GetChargeState() == CHARGESTATE_IDLE)
		{
			StartCharging();
		}

		if (GetChargeState() == CHARGESTATE_CHARGING)
		{
			if (player->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
				ReleaseChargedShot();

			m_fDamageScale += 0.03f;

			if (gpGlobals->curtime >= m_fFullyChargedTime)
			{
				state = CHARGESTATE_CHARGED;
				SetIdealActivity(ACT_VM_FIDGET);
			}

			if (gpGlobals->curtime > m_fNextRemove)
			{
				player->RemoveAmmo(1, m_iPrimaryAmmoType);
				m_fNextRemove = gpGlobals->curtime + 0.03f;
			}
		}
	}

	if (!(player->m_nButtons & IN_ATTACK2))
	{
		if (GetChargeState() == CHARGESTATE_CHARGED)
			ReleaseChargedShot();

		else if (GetChargeState() == CHARGESTATE_CHARGING)
		{
			m_fDamageScale += 0.03f;
			if (gpGlobals->curtime > m_fNextRemove)
			{
				player->RemoveAmmo(1, m_iPrimaryAmmoType);
				m_fNextRemove = gpGlobals->curtime + 0.03f;
			}

			if (gpGlobals->curtime >= m_fSoonestChargeShot)
				ReleaseChargedShot();
		}
	}

	BaseClass::ItemPostFrame();
}

void CWeapon357::ReleaseChargedShot()
{
	CBasePlayer* player = ToBasePlayer(GetOwner());
	if (!player)
		return;
	player->SetFOV(this, 0, 0.1f);
	StopParticleEffects(player->GetViewModel());
	PrimaryAttack();
	StopSound("Railgun.Chargeup");
	state = CHARGESTATE_IDLE;
	m_fDamageScale = 1.f;
}
void CWeapon357::StartCharging()
{
	CBasePlayer* player = ToBasePlayer(GetOwner());
	if (!player)
		return;
	player->SetFOV(this, player->GetFOV() + 10, 2.f);
	state = CHARGESTATE_CHARGING;
	SetIdealActivity(ACT_VM_PULLBACK);
	m_fFullyChargedTime = gpGlobals->curtime + SequenceDuration();
	m_fSoonestChargeShot = gpGlobals->curtime + 1.f;
	m_fStartTime = gpGlobals->curtime;
	m_fNextRemove = gpGlobals->curtime + 0.03f;
	EmitSound("Railgun.Chargeup");
	DispatchParticleEffect("railgun_chargeup", PATTACH_POINT_FOLLOW, player->GetViewModel(), "chargepoint");
}
void CWeapon357::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	CHL2_Player *phl2 = dynamic_cast<CHL2_Player*>(pPlayer);
	CBaseViewModel *pViewModel = pPlayer->GetViewModel();

	if (pPlayer == NULL)
		return;

	if (m_bInZoom)
	{
		if (g_thirdperson.GetBool())
			engine->ClientCommand(pPlayer->edict(), "thirdperson\n");

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

	}
	else
	{
		if (g_thirdperson.GetBool())
			engine->ClientCommand(pPlayer->edict(), "firstperson\n");

		if (pPlayer->SetFOV(this, 45, 0.1f))
		{
			m_bInZoom = true;
			phl2->m_HL2Local.m_bZooming = true;
			pViewModel->AddEffects(EF_NODRAW);
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();
		}
	}
}

void CWeapon357::GetControlPanelInfo(int nPanelIndex, const char *&pPanelName)
{
	pPanelName = "railgun_battery_screen";
}

bool CWeapon357::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	StopEffects();
	return BaseClass::Holster(pSwitchingTo);
}

void CWeapon357::ResetFOV()
{
	CBasePlayer* player = ToBasePlayer(GetOwner());
	if (!player)
		return;
	player->SetFOV(this, 0, 0.1f);
}
void CWeapon357::StopEffects(void)
{
	ResetFOV();
	// Stop zooming
	if (m_bInZoom)
	{
		ToggleZoom();
	}
}