///               HLRMP
///             Projectile Base
///				 by just wax
///


#ifndef PROJECTILE_BASE_H
#define PROJECTILE_BASE_H
#ifdef _WIN32
#pragma once
#endif
#ifdef CLIENT_DLL
#include "c_baseanimating.h"
#include "c_te_legacytempents.h"
#include "tempent.h"
#else
#include "baseanimating.h"
#endif

#ifdef CLIENT_DLL
#define CHLRMPProjectileBase C_HLRMPProjectileBase
C_LocalTempEntity *ClientsideProjectileCallback(const CEffectData &data, float flGravityBase, const char *pszParticleName = NULL);
#endif

//=============================================================================
// CFFProjectileBase
//=============================================================================

class CHLRMPProjectileBase : public CBaseAnimating
{
public:
	DECLARE_CLASS(CHLRMPProjectileBase, CBaseAnimating);
	DECLARE_NETWORKCLASS();

public:

	CNetworkVector(m_vecInitialVelocity);


	CHLRMPProjectileBase();
	virtual void Precache();
	virtual void Spawn();

	CNetworkVarForDerived(float, m_flSpawnTime);

	
	

#ifdef CLIENT_DLL

	// Add initial velocity into the interpolation history so that interp works okay
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual int		DrawModel(int flags);
	// Both to catch the end of this projectile's life

	// No shadows for projectiles
	virtual ShadowType_t ShadowCastType() { return SHADOWS_NONE; }
	
	
private:
	// Flag to keep track of whether projectile needs a cleanup

#else
	DECLARE_DATADESC();

	// Specify what velocity we want to have on the client immediately.
	// Without this, the entity wouldn't have an interpolation history initially, so it would
	// sit still until it had gotten a few updates from the server.
	void SetupInitialTransmittedVelocity(const Vector &velocity);

#endif

protected:
	static CHLRMPProjectileBase *Create(const char *pszClassName, const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pOwner, float flVelocity, float flTime, short iProjectileModel, const char *pszDispatchEffect);
private:

};

#endif