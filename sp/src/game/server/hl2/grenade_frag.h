//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_FRAG_H
#define GRENADE_FRAG_H

#include "Sprite.h"
#include "SpriteTrail.h"
#include "cbase.h"
#include "basegrenade_shared.h"
class CBaseGrenade;
class CGrenadeFrag;
struct edict_t;

CBaseGrenade *Fraggrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned, bool bAccelerated = false );
CBaseGrenade *Gasgrenade_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer);
CBaseGrenade *Stickynade_Create(const Vector &position, const QAngle &angles, const Vector &vecocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner);
bool	Fraggrenade_WasPunted( const CBaseEntity *pEntity );
bool	Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity ); 
static	CGrenadeFrag *Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned);

class CGrenadeFrag : public CBaseGrenade
{
	DECLARE_CLASS(CGrenadeFrag, CBaseGrenade);

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

	~CGrenadeFrag(void);

public:
	void	Spawn(void);
	void	OnRestore(void);
	void	Precache(void);
	//bool	CreateVPhysics(void);
	void	CreateEffects(void);
	void	NadeTouch(CBaseEntity *pOther);
	void	Explode(trace_t *pTrace, int bitsDamageType);
	void	SetTimer(float detonateDelay, float warnDelay);
	void	SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity);
	int		OnTakeDamage(const CTakeDamageInfo &inputInfo);

	void	BlipSound();
	void	DelayThink();
	//void	Detonate(void);
//	void	VPhysicsUpdate(IPhysicsObject *pPhysics);
	void	OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);
	void	SetCombineSpawned(bool combineSpawned) { m_combineSpawned = combineSpawned; }
	bool	IsCombineSpawned(void) const { return m_combineSpawned; }
	void	SetPunted(bool punt) { m_punted = punt; }
	bool	WasPunted(void) const { return m_punted; }
	float		iBounces;
	void	SetAccelerated(bool accelerated) { m_bAccelerated = accelerated; }

	// this function only used in episodic.
#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
#endif 

	void	InputSetTimer(inputdata_t &inputdata);

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	m_combineSpawned;
	bool	m_punted;
	float	m_flAlpha;
	float	m_flScale;
	bool	m_bAccelerated;

	CNetworkHandle(CBaseEntity, m_hThrower);
};
////
////
//gas grenade
////
////
class CGrenadeGas : public CBaseGrenade
{
	DECLARE_CLASS(CGrenadeGas, CBaseGrenade);

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

	~CGrenadeGas(void);

public:
	void	Spawn(void);
	void	OnRestore(void);
	void	Precache(void);
	//bool	CreateVPhysics(void);
	void	CreateEffects(void);
	void	NadeTouch(CBaseEntity *pOther);
	void	Explode(trace_t *pTrace, int bitsDamageType);
	void	SetTimer(float detonateDelay, float warnDelay);
	void	SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity);
	int		OnTakeDamage(const CTakeDamageInfo &inputInfo);
	void	BlipSound() { EmitSound("Grenade.Blip"); }
	void	DelayThink();
	//void	Detonate(void);
	//void	VPhysicsUpdate(IPhysicsObject *pPhysics);
	float	iBounces;

	// this function only used in episodic.
#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
#endif 

	void	InputSetTimer(inputdata_t &inputdata);

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	
	CNetworkHandle(CBaseEntity, m_hThrower);
};
/////
/////
//gas grenade tempent
/////
/////

class CGasTemp : public CBaseCombatCharacter
{
	DECLARE_CLASS(CGasTemp, CBaseCombatCharacter);
	DECLARE_DATADESC();

public:
	void Spawn(void);
	void EmitDamage(void);

	float m_fLastDamage;
	float m_fNextDamage;
	float m_fFinalDamage;

};

class CStickyGrenade : public CBaseGrenade
{
	DECLARE_CLASS(CStickyGrenade, CBaseGrenade);
	DECLARE_DATADESC();

public:
	void Spawn(void);
	void Precache(void);
	void Explode(void);
	void StartTimer(void);
	void OnTouch(CBaseEntity *pOther);
	bool Stick(CBaseEntity *pOther);
	IPhysicsConstraint			*m_pConstraint;
	EHANDLE						m_hConstrainedEntity;
private:

protected:




	//CHandle<CSprite>		m_pMainGlow;
	//CHandle<CSpriteTrail>	m_pGlowTrail;

};
#endif // GRENADE_FRAG_H
