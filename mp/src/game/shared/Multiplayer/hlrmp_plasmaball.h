
#ifndef HLRMP_PLASMABALL_H
#define HLRMP_PLASMABALL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "shareddefs.h"
#include "hlrmp_projectile_base.h"
#include "sprite.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_baseanimating.h"
// Server specific.
#else
#include "baseanimating.h"
#endif

#ifdef CLIENT_DLL
#define CHLRMPPlasmaBall C_HLRMPPlasmaBall
#endif




class CHLRMPPlasmaBall : public CHLRMPProjectileBase
{
public:

	DECLARE_CLASS(CHLRMPPlasmaBall, CHLRMPProjectileBase);
	DECLARE_NETWORKCLASS();

	CHLRMPPlasmaBall();
	~CHLRMPPlasmaBall();

	void	Precache(void);
	void	Spawn(void);





protected:

#ifdef CLIENT_DLL

public:

	virtual void	OnDataChanged(DataUpdateType_t type);
	virtual int		DrawModel(int flags);
	//CHandle<CSprite> m_pSprite;

private:

	float	 m_flSpawnTime;

	//=============================================================================
	//
	// Server specific.
	//
#else

public:

	DECLARE_DATADESC();

	static CHLRMPPlasmaBall *Create(const char *szClassname, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL);

	virtual void	Touch(CBaseEntity *pOther);
	void			Explode(CBaseEntity *pOther);

	virtual float	GetDamage() { return m_flDamage; }
	//virtual int		GetDamageType() { return g_aWeaponDamageTypes[GetWeaponID()]; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }
	//virtual float	GetRadius() { return TF_ROCKET_RADIUS; }
	//void			DrawRadius(float flRadius);

	unsigned int	PhysicsSolidMaskForEntity(void) const;

	//virtual int		GetWeaponID(void) const			{ return TF_WEAPON_ROCKETLAUNCHER; }

	virtual CBaseEntity		*GetEnemy(void)			{ return m_hEnemy; }

protected:

	// Not networked.
	float					m_flDamage;

	float					m_flCollideWithTeammatesTime;
	bool					m_bCollideWithTeammates;


	CHandle<CBaseEntity>	m_hEnemy;

#endif
};

#endif