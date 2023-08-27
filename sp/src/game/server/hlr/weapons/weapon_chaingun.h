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

class CWeaponChaingun : public CHLMachineGun
{
public:
	DECLARE_CLASS(CWeaponChaingun, CHLMachineGun );

	CWeaponChaingun();

	DECLARE_SERVERCLASS();

	void	ItemPostFrame( void );
	void	Precache( void );
	void	WeaponIdle(void);
	//void	PrimaryAttack(void);
	void	SecondaryAttack( void );

	const char *GetTracerType( void ) { return "AirboatGunHeavyTracer"; }

	void	AddViewKick( void );

	void PrimaryAttack(void);
	void SendData();

	void UpdateWeaponSoundState(void);
	int m_iWeaponState;
	void DestroyWeaponSound(void);
	void Equip(CBaseCombatCharacter *pOwner);

	float	GetFireRate(void) { return m_flfirerate;}

	void	InitWoundSound(void);
	void	ShutdownWoundSound(void);
	
	void	ItemHolsterFrame(void);

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
	

	bool AmFocusFiring(void);

	void ToggleZoom(void);

	float m_fPitch;


	float GetMaxFirerate();

	Vector GetSpread(void);

	const WeaponProficiencyInfo_t *GetProficiencyValues();

protected:
	bool					m_bNarrowfov;
	float					m_flfirerate;
	bool					bActive;
	int						iNumShots;
	
	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};


#endif	//WEAPONAR2_H
