//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONCHAINGUN_H
#define	WEAPONCHAINGUN_H

#include "basegrenade_shared.h"
#include "basehlcombatweapon.h"

class CWeaponChaingun : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS(CWeaponChaingun, CHLSelectFireMachineGun);	
	
	CWeaponChaingun();

	DECLARE_SERVERCLASS();

	void	ItemPostFrame(void);
	void	Precache(void);

	//void	SecondaryAttack(void);
	//void	DelayedAttack(void);

	//const char *GetTracerType(void) { return "AR2Tracer"; }

	void	AddViewKick(void);

	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	//void	FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	int		GetMinBurst(void) { return 100; }
	int		GetMaxBurst(void) { return 100; }
	float	GetFireRate(void) { return 0.1f; }

	bool	CanHolster(void);
	bool	Reload(void);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	Activity	GetPrimaryAttackActivity(void);

	void	DoImpactEffect(trace_t &tr, int nDamageType);

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		cone = VECTOR_CONE_3DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

protected:

	float					m_flDelayedFire;
	bool					m_bShotDelayed;
	int						m_nVentPose;

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};


#endif	//WEAPONAR2_H