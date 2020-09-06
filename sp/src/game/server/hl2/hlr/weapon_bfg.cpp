#include "cbase.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "particle_parse.h"
#include "hl2_gamerules.h"
#include "gamestats.h"

#include "tier0/memdbgon.h"

#define BFG_PROJECTILE_MODEL "models/spitball_large.mdl"
//#define NULL_MODEL "models/props/null.mdl"

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

	RadiusDamage(CTakeDamageInfo(this, pPlayer, 800, DMG_SONIC), GetAbsOrigin(), 800, CLASS_PLAYER, NULL);
	SetThink(&CProjectileBFG::Kill);
	SetNextThink(gpGlobals->curtime + 0.5f);
}
void CProjectileBFG::Kill(void)
{
	SetThink(NULL);
	UTIL_Remove(this);
}


class CWeaponBFG : public CBaseCombatWeapon
{
	
public:
	DECLARE_CLASS(CWeaponBFG, CBaseCombatWeapon);
	void Precache(void);
	void PrimaryAttack(void);
	void LaunchBall(void);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};
IMPLEMENT_SERVERCLASS_ST(CWeaponBFG, DT_WeaponBFG)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_bfg, CWeaponBFG);
PRECACHE_WEAPON_REGISTER(weapon_bfg);

BEGIN_DATADESC(CWeaponBFG)
END_DATADESC()

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
	SetThink(&CWeaponBFG::LaunchBall);
	SetNextThink(gpGlobals->curtime + 0.9f);
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
}
void CWeaponBFG::LaunchBall(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);
	Vector vecAiming;
	QAngle angAiming = pPlayer->EyeAngles();
	AngleVectors(angAiming, &vecAiming);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();

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