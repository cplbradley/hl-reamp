
#ifndef HLRMP_ROCKET_H
#define HLRMP_ROCKET_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "shareddefs.h"
#include "hlrmp_projectile_base.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_baseanimating.h"
#include "c_te_legacytempents.h"
// Server specific.
#else
#include "baseanimating.h"
#include "smoke_trail.h"
#endif

#ifdef CLIENT_DLL
#define CHLRMPRocket C_HLRMPRocket
#endif

#define TF_ROCKET_RADIUS	(110.0f * 1.1f)	//radius * TF scale up factor



class CHLRMPRocket : public CHLRMPProjectileBase
{
public:

	DECLARE_CLASS(CHLRMPRocket, CHLRMPProjectileBase);
	DECLARE_NETWORKCLASS();

	CHLRMPRocket();
	~CHLRMPRocket();

	void	Precache(void);
	void	Spawn(void);
	CNetworkVector(m_vecSentAngleVector);
	CNetworkVector(m_vecSentVelocity);
	
	

	static CHLRMPRocket *Create(const char *szClassname, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, float flVelocity = 0, float flTime = 3.0f);

protected:

#ifdef CLIENT_DLL

public:

	virtual void	OnDataChanged(DataUpdateType_t type);
	virtual int		DrawModel(int flags);
	C_LocalTempEntity	*pTemp;
private:

	float	 m_flSpawnTime;

	//=============================================================================
	//
	// Server specific.
	//
#else

public:

	DECLARE_DATADESC();

	

	virtual void	RocketTouch(CBaseEntity *pOther);
	

	virtual float	GetDamage() { return m_flDamage; }
	//virtual int		GetDamageType() { return g_aWeaponDamageTypes[GetWeaponID()]; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }
	virtual float	GetRadius() { return TF_ROCKET_RADIUS; }
	//void			DrawRadius(float flRadius);
	void			Explode(trace_t *pTrace, CBaseEntity *pOther);
	

	unsigned int	PhysicsSolidMaskForEntity(void) const;

	//virtual int		GetWeaponID(void) const			{ return TF_WEAPON_ROCKETLAUNCHER; }

	virtual CBaseEntity		*GetEnemy(void)			{ return m_hEnemy; }

	void			SetHomingTarget(CBaseEntity *pHomingTarget);

protected:

	//void			FlyThink(void);

protected:

	void CreateSmokeTrail(void);

	// Not networked.
	float					m_flDamage;

	CHandle<RocketTrail>	m_hRocketTrail;

	float					m_flCollideWithTeammatesTime;
	bool					m_bCollideWithTeammates;


	CHandle<CBaseEntity>	m_hEnemy;

#endif
};

#endif // TF_WEAPONBASE_ROCKET_H