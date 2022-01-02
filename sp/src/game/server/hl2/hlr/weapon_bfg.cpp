#include "cbase.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "sdk/sdk_gamerules.h"
#include "vstdlib/random.h"
#include "particle_parse.h"
#include "hl2_gamerules.h"
#include "gamestats.h"

#include "tier0/memdbgon.h"

#define BFG_PROJECTILE_MODEL "models/spitball_large.mdl"
//#define NULL_MODEL "models/props/null.mdl"
class CProjectileOverdrivenBFG : public CBaseCombatCharacter
{
	DECLARE_CLASS(CProjectileOverdrivenBFG, CBaseCombatCharacter);
public:
	void Precache(void);
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
	void DoDamage(void);
	void Kill(void);
	bool	CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;



	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(bfg_overdriven_projectile, CProjectileOverdrivenBFG);
BEGIN_DATADESC(CProjectileOverdrivenBFG)
END_DATADESC()
bool CProjectileOverdrivenBFG::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_SOLID, false);

	return true;
}
unsigned int CProjectileOverdrivenBFG::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}

void CProjectileOverdrivenBFG::Precache(void)
{
	PrecacheModel(BFG_PROJECTILE_MODEL);
	PrecacheParticleSystem("bfg_trail_red");
	PrecacheParticleSystem("bfg_explosion_core_overdrive");
	PrecacheScriptSound("Weapon_BFG.ExplodeOverdrive");
}
void CProjectileOverdrivenBFG::Spawn(void)
{
	Precache();
	SetModel(BFG_PROJECTILE_MODEL);
	UTIL_SetSize(this, -Vector(1.0f, 1.0f, 1.0f), Vector(1.0f, 1.0f, 1.0f));
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	SetTouch(&CProjectileOverdrivenBFG::Touch);
	SetGravity(0.3f);
	SetRenderColor(255, 0, 0);
	DispatchParticleEffect("bfg_trail_red", PATTACH_ABSORIGIN_FOLLOW, this, "root", false); //i'm super gooey
}
void CProjectileOverdrivenBFG::Touch(CBaseEntity *pOther)
{
	if (pOther == NULL)
		return;
	if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsPlayer())
	{
		return;
	}
	if (pOther->IsSolid())
	{

		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER)) //is it a trigger?
			return; //ignore it, keep going
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE) //is it another projectile
			return; //ignore it, keep going
		StopParticleEffects(this);
		SetMoveType(MOVETYPE_NONE);
		SetSolid(SOLID_NONE);
		AddFlag(EF_NODRAW);
		//SetModelName(NULL_STRING);
		EmitSound("Weapon_BFG.ExplodeOverdrive");
		DispatchParticleEffect("bfg_explosion_core_overdrive", GetAbsOrigin(), GetAbsAngles(), this);
		SetThink(&CProjectileOverdrivenBFG::DoDamage);
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime + 2.0f);
	}
}

void CProjectileOverdrivenBFG::DoDamage(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
	AddFlag(EF_NODRAW);
	g_pGameRules->RadiusDamage(CTakeDamageInfo(this, pPlayer, 50000, DMG_SONIC), GetAbsOrigin(), 8192, CLASS_PLAYER, true);
	SetThink(&CProjectileOverdrivenBFG::Kill);
	SetNextThink(gpGlobals->curtime + 0.5f);
	UTIL_ScreenShake(pPlayer->WorldSpaceCenter(), 20, 10, 5, 2048, SHAKE_START);
	color32 red = { 255, 193, 155, 200 };
	UTIL_ScreenFade(pPlayer, red, 4.0f, 1.0f, FFADE_IN);
}
void CProjectileOverdrivenBFG::Kill(void)
{
	SetThink(NULL);
	UTIL_Remove(this);
}
class CProjectileBFG : public CBaseCombatCharacter
{
	DECLARE_CLASS(CProjectileBFG, CBaseCombatCharacter);
public:
	void Precache(void);
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
	void DoDamage(void);
	void Kill(void);
	bool	CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;

	DECLARE_DATADESC();

};
LINK_ENTITY_TO_CLASS(bfg_projectile, CProjectileBFG);
BEGIN_DATADESC(CProjectileBFG)
END_DATADESC()
bool CProjectileBFG::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_SOLID, false);

	return true;
}
unsigned int CProjectileBFG::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CProjectileBFG::Precache(void)
{
	PrecacheParticleSystem("bfg_explosion_core");
	PrecacheParticleSystem("bfg_trail");
	PrecacheScriptSound("Weapon_BFG.Explode");
	PrecacheModel(NULL_MODEL);
	PrecacheModel(BFG_PROJECTILE_MODEL);
}
void CProjectileBFG::Spawn(void)
{
	Precache();
	SetModel(BFG_PROJECTILE_MODEL);
	UTIL_SetSize(this, -Vector(1.0f, 1.0f, 1.0f), Vector(1.0f, 1.0f, 1.0f));
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	SetTouch(&CProjectileBFG::Touch);
	SetGravity(0.3f);
	SetRenderColor(0, 215, 255);
	DispatchParticleEffect("bfg_trail", PATTACH_ABSORIGIN_FOLLOW, this, "root", false); //i'm super gooey
}
void CProjectileBFG::Touch(CBaseEntity *pOther)
{
	if (pOther == NULL)
		return;
	if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsPlayer())
	{
		return;
	}
	if (pOther->IsSolid())
	{
		
		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER)) //is it a trigger?
			return; //ignore it, keep going
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE) //is it another projectile
			return; //ignore it, keep going
		StopParticleEffects(this);
		SetMoveType(MOVETYPE_NONE);
		SetSolid(SOLID_NONE);
		AddFlag(EF_NODRAW);
		SetModel(NULL_MODEL);
		EmitSound("Weapon_BFG.Explode");
		DispatchParticleEffect("bfg_explosion_core", GetAbsOrigin(), GetAbsAngles(), this);
		SetThink(&CProjectileBFG::DoDamage);
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime + 0.6f);
	}


}
void CProjectileBFG::DoDamage(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
	AddFlag(EF_NODRAW);
	g_pGameRules->RadiusDamage(CTakeDamageInfo(this, pPlayer, 800, DMG_SONIC), GetAbsOrigin(), 800, CLASS_PLAYER, true);
	SetThink(&CProjectileBFG::Kill);
	SetNextThink(gpGlobals->curtime + 0.5f);
}
void CProjectileBFG::Kill(void)
{
	SetThink(NULL);
	UTIL_Remove(this);
}


class CWeaponBFG : public CBaseHLCombatWeapon
{
	
public:
	DECLARE_CLASS(CWeaponBFG, CBaseHLCombatWeapon);
	void Precache(void);
	void PrimaryAttack(void);
	void LaunchBall(void);
	void WackyFOVShit(void);
	void ResetFOV(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

LINK_ENTITY_TO_CLASS(weapon_bfg, CWeaponBFG);
PRECACHE_WEAPON_REGISTER(weapon_bfg);

BEGIN_DATADESC(CWeaponBFG)
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponBFG,DT_WeaponBFG)
END_SEND_TABLE()

void CWeaponBFG::Precache(void)
{
	UTIL_PrecacheOther("bfg_projectile");
	PrecacheParticleSystem("bfg_weapon_core");
	PrecacheScriptSound("Weapon_BFG.Single");
	BaseClass::Precache();
}
void CWeaponBFG::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (m_flNextPrimaryAttack >= gpGlobals->curtime)
		return;
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;
	int iAttachment = LookupAttachment("muzzle");

	EmitSound("Weapon_BFG.Single");
	pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	DispatchParticleEffect("bfg_weapon_core", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), iAttachment, true);
	m_flNextPrimaryAttack = (gpGlobals->curtime + 6.0f);
	WackyFOVShit();
	SetThink(&CWeaponBFG::LaunchBall);
	SetNextThink(gpGlobals->curtime + 0.9f);
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
}
void CWeaponBFG::LaunchBall(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	ResetFOV();
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);
	Vector vecAiming;
	QAngle angAiming = pPlayer->EyeAngles();
	AngleVectors(angAiming, &vecAiming);
		
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	if (!pPlayer->HasOverdrive())
	{
		CProjectileBFG *pOrb = (CProjectileBFG *)CreateEntityByName("bfg_projectile");
		pOrb->Spawn();
		pOrb->SetOwnerEntity(pPlayer);
		UTIL_SetOrigin(pOrb, vecSrc);
		pOrb->SetLocalAngles(QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		pOrb->SetLocalAngularVelocity(
			QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		pOrb->SetAbsVelocity(vecAiming * 1200.0f);
	}
	else
	{
		CProjectileOverdrivenBFG *pOrb = (CProjectileOverdrivenBFG *)CreateEntityByName("bfg_overdriven_projectile");
		pOrb->Spawn();
		pOrb->SetOwnerEntity(pPlayer);
		UTIL_SetOrigin(pOrb, vecSrc);
		pOrb->SetLocalAngles(QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		pOrb->SetLocalAngularVelocity(
			QAngle(random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500),
			random->RandomFloat(-250, -500)));
		pOrb->SetAbsVelocity(vecAiming * 1200.0f);
	}
	
}

bool CWeaponBFG::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	ResetFOV();
	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponBFG::WackyFOVShit(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	int baseFOV = pPlayer->GetFOV();
	pPlayer->SetFOV(this, baseFOV + 10, 1.0f);
}

void CWeaponBFG::ResetFOV(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	pPlayer->SetFOV(this, 0, 0.1f);
}