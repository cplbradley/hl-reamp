//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//


#ifndef NPC_ATTACKCHOPPER_H
#define NPC_ATTACKCHOPPER_H

#ifdef _WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Creates an avoidance sphere
//-----------------------------------------------------------------------------
CBaseEntity *CreateHelicopterAvoidanceSphere( CBaseEntity *pParent, int nAttachment, float flRadius, bool bAvoidBelow = false );

// Chopper gibbage
void Chopper_BecomeChunks( CBaseEntity *pChopper );
void Chopper_CreateChunk( CBaseEntity *pChopper, const Vector &vecChunkPos, const QAngle &vecChunkAngles, const char *pszChunkName, bool bSmall );
void Chopper_PrecacheChunks( CBaseEntity *pChopper );

class CBombDropSensor : public CBaseEntity
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CBombDropSensor, CBaseEntity);

	void Spawn();

	// Drop a bomb at a particular location
	void	InputDropBomb(inputdata_t &inputdata);
	void	InputDropBombStraightDown(inputdata_t &inputdata);
	void	InputDropBombAtTarget(inputdata_t &inputdata);
	void	InputDropBombAtTargetAlways(inputdata_t &inputdata);
	void	InputDropBombDelay(inputdata_t &inputdata);
};


class CHelicopterChunk : public CBaseAnimating
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CHelicopterChunk, CBaseAnimating);

	virtual void Spawn(void);
	virtual void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);

	static CHelicopterChunk *CreateHelicopterChunk(const Vector &vecPos, const QAngle &vecAngles, const Vector &vecVelocity, const char *pszModelName, int chunkID);

	int		m_nChunkID;

	CHandle<CHelicopterChunk>	m_hMaster;
	IPhysicsConstraint			*m_pTailConstraint;
	IPhysicsConstraint			*m_pCockpitConstraint;

protected:

	void	CollisionCallback(CHelicopterChunk *pCaller);

	void	FallThink(void);

	bool	m_bLanded;
};


#endif // NPC_ATTACKCHOPPER_H


