//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ITEMS_H
#define ITEMS_H

#ifdef _WIN32
#pragma once
#endif

#include "entityoutput.h"
#include "player_pickup.h"
#include "vphysics/constraints.h"


// Armor given by a battery
#define MAX_NORMAL_BATTERY	100

// Ammo counts given by ammo items


#define SIZE_AMMO_SHOTGUN_LARGE		16
#define SIZE_AMMO_SHOTGUN_SMALL		8
#define SIZE_AMMO_HEAVY_LARGE		100
#define SIZE_AMMO_HEAVY_SMALL		50
#define SIZE_AMMO_ENERGY_LARGE		120
#define SIZE_AMMO_ENERGY_SMALL		60
#define SIZE_AMMO_EXPLOSIVE_LARGE	10
#define SIZE_AMMO_EXPLOSIVE_SMALL	4
#define SIZE_AMMO_ATOMIC			100


#define SIZE_AMMO_PISTOL			20
#define SIZE_AMMO_PISTOL_LARGE		100
#define SIZE_AMMO_SMG1				90
#define SIZE_AMMO_SMG1_LARGE		120
#define SIZE_AMMO_AR2				80
#define SIZE_AMMO_AR2_LARGE			100
#define SIZE_AMMO_RPG_ROUND			2
#define SIZE_AMMO_EXPLOSIVE_BOX		4
#define SIZE_AMMO_SMG1_GRENADE		1
#define SIZE_AMMO_BUCKSHOT			18
#define SIZE_AMMO_357				6
#define SIZE_AMMO_357_LARGE			20
#define SIZE_AMMO_CROSSBOW			6
#define	SIZE_AMMO_AR2_ALTFIRE		1

#define SF_ITEM_START_CONSTRAINED	0x00000001


class CItem : public CBaseAnimating, public CDefaultPlayerPickupVPhysics
{
public:
	DECLARE_CLASS( CItem, CBaseAnimating );

	CItem();

	virtual void Spawn( void );
	virtual void Precache();
	virtual void DelayedKill(void);

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	virtual CBaseEntity* Respawn( void );
	virtual void ItemTouch( CBaseEntity *pOther );
	virtual void Materialize( void );
	virtual bool MyTouch( CBasePlayer *pPlayer ) { return false; };

	// Become touchable when we are at rest
	virtual void OnEntityEvent( EntityEvent_t event, void *pEventData );

	// Activate when at rest, but don't allow pickup until then
	void ActivateWhenAtRest( float flTime = 0.5f );
	void TraceThink(void);
	void SeekThink(void);
	float FindPlayer();

	// IPlayerPickupVPhysics
	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON );
	virtual void OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason );

	virtual int	ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_WCEDIT_POSITION; };
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	Vector	GetOriginalSpawnOrigin( void ) { return m_vOriginalSpawnOrigin;	}
	QAngle	GetOriginalSpawnAngles( void ) { return m_vOriginalSpawnAngles;	}
	void	SetOriginalSpawnOrigin( const Vector& origin ) { m_vOriginalSpawnOrigin = origin; }
	void	SetOriginalSpawnAngles( const QAngle& angles ) { m_vOriginalSpawnAngles = angles; }
	bool	CreateItemVPhysicsObject( void );
	virtual bool	ItemCanBeTouchedByPlayer( CBasePlayer *pPlayer );
	bool	m_bShouldSeek{ false };
	virtual void CheckQuantity(void);

#if defined( HL2MP ) || defined( TF_DLL )
	void	FallThink( void );
	float  m_flNextResetCheckTime;
#endif

	DECLARE_DATADESC();
protected:
	virtual void ComeToRest( void );

private:
	bool		m_bActivateWhenAtRest;
	COutputEvent m_OnPlayerTouch;
	COutputEvent m_OnCacheInteraction;
	
	Vector		m_vOriginalSpawnOrigin;
	QAngle		m_vOriginalSpawnAngles;

	IPhysicsConstraint		*m_pConstraint;
};

#endif // ITEMS_H
