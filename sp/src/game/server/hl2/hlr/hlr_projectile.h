#include "cbase.h"
#include "basegrenade_shared.h"
#include "Sprite.h"
#include "SpriteTrail.h"

#pragma once


class CHLRVortProjectile : public CBaseCombatCharacter
{
	DECLARE_CLASS(CHLRVortProjectile, CBaseCombatCharacter);
public:
	void	Spawn(void);
	void	Precache(void);
	void	Touch(CBaseEntity *pOther);
	void	KillIt(void);
	bool	CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;
	void	AdjustSpeed(void);


	static CHLRVortProjectile *Create(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL);
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	bool	CreateTrail(void);
	bool	DrawSprite(void);
	DECLARE_DATADESC();
};
class CHLRPistolProjectile : public CBaseCombatCharacter
{
	DECLARE_CLASS(CHLRPistolProjectile, CBaseCombatCharacter)
public:
	void Spawn(void);
	void Precache(void);
	void Touch(CBaseEntity *pOther);
	void KillIt(void);
	void TargetTrackThink(void);
	void SetTargetPos(const Vector &vecTargetpos, const float &fVelocity);
	bool CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;

	Vector vecTarget;
	float flVelocity;

	static CHLRPistolProjectile *Create(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL);
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	bool	CreateTrail(void);
	bool	DrawSprite(void);
	DECLARE_DATADESC();
};
class CHLRFireball : public CBaseCombatCharacter
{
	DECLARE_CLASS(CHLRFireball, CBaseCombatCharacter)
public:
	void Spawn(void);
	void Precache(void);
	void Touch(CBaseEntity *pOther);
	void Detonate(void);
	void Kill(void);
	bool CreateVPhysics(void);
	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }
	DECLARE_DATADESC();
};
class CHLRFireballTemp : public CBaseCombatCharacter
{
	DECLARE_CLASS(CHLRFireballTemp, CBaseCombatCharacter)
public:
	void Spawn(void);
	void Precache(void);
	void Kill(void);
	void Touch(CBaseEntity *pOther);

	float m_flNextDamage;
	DECLARE_DATADESC();
};

class CHLRScannerProjectile : public CBaseAnimating
{
	DECLARE_CLASS(CHLRScannerProjectile,CBaseAnimating)
public:
	void Spawn(void);
	void Precache(void);
	void DispatchEffects(void);
	void Kill(void);
	void Touch(CBaseEntity *pOther);
	DECLARE_DATADESC();
};
