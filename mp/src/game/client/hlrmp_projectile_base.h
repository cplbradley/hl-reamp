///               HLRMP
///             Projectile Base
///				 by just wax
///


#ifndef FF_PROJECTILE_BASE_H
#define FF_PROJECTILE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "basegrenade_shared.h"

#ifdef CLIENT_DLL
#define CHLRMPProjectileBase C_HLRMPProjectileBase
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

	Vector m_vecInitialVelocity;

	CHLRMPProjectileBase();
	virtual void Precache();
	virtual void Spawn();

	CNetworkVarForDerived(float, m_flSpawnTime);

#ifdef CLIENT_DLL

	// Add initial velocity into the interpolation history so that interp works okay
	virtual void OnDataChanged(DataUpdateType_t type);

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

private:

};

#endif // FF_PROJECTILE_BASE_H