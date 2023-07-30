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

#ifndef	WEAPONAR2_H
#define	WEAPONAR2_H

#include "basegrenade_shared.h"
#include "basehlcombatweapon.h"
#include "soundenvelope.h"

class CWeaponAR2 : public CHLMachineGun
{
public:
	DECLARE_CLASS( CWeaponAR2, CHLMachineGun );

	CWeaponAR2();

	DECLARE_SERVERCLASS();

	void	ItemPostFrame( void );
	void	Precache( void );
	void	WeaponIdle(void);
	//void	PrimaryAttack(void);
	void	SecondaryAttack( void );
	void	DelayedAttack( void );

	const char *GetTracerType( void ) { return "AirboatGunHeavyTracer"; }

	void	AddViewKick( void );

	void PrimaryAttack(void);
	void SendData();

	void UpdateWeaponSoundState(void);
	int m_iWeaponState;
	void DestroyWeaponSound(void);
	void Equip(CBaseCombatCharacter *pOwner);
	void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void	FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void	Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	int		GetMinBurst( void ) { return 1; }
	int		GetMaxBurst( void ) { return 2; }
	float	GetFireRate(void) { return m_flfirerate;}

	void	InitWoundSound(void);
	void	ShutdownWoundSound(void);
	
	void	ItemHolsterFrame(void);
	bool	CanHolster( void );
	bool	Reload( void );
	CSoundPatch *m_pWoundSound;
	CSoundPatch* m_pChaingun;
	CSoundPatch* m_pFocus;
	CSoundPatch* m_pOverdrive;

	void ShutdownAllSounds();
	void ShutdownChaingunSound();
	void ShutdownFocusSound();
	void ShutdownOverdriveSound();
	bool m_bPlayingWoundSound;
	bool bShooting;
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	Activity	GetPrimaryAttackActivity( void );
	
	void	DoImpactEffect( trace_t &tr, int nDamageType );

	bool AmFocusFiring(void);

	void ToggleZoom(void);

	float m_fPitch;


	float GetMaxFirerate();

	Vector GetSpread(void);

	const WeaponProficiencyInfo_t *GetProficiencyValues();

protected:
	bool					m_bNarrowfov;
	float					m_flfirerate;
	float					m_flDelayedFire;
	bool					m_bShotDelayed;
	int						m_nVentPose;
	bool					bActive;
	int						iNumShots;
	
	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};


#endif	//WEAPONAR2_H
