#include "cbase.h"
#include "rope.h"
#include "beam_shared.h"
#include "vphysics_interface.h"
#include "physobj.h"

#pragma once

#define MAX_ROPE_SEGMENTS 32

class CClimbRopeSegment : public CBaseAnimating
{
	DECLARE_CLASS(CClimbRopeSegment, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
public:

	void Spawn();
	void Precache();

	virtual int UpdateTransmitState();

	bool VPhysicsCreate();

	bool bAmAnchor;
	bool bAmGrabbed;

	CNetworkVar(float, fWidth);
	CNetworkHandle(CClimbRopeSegment, hNextSegment);
	static CClimbRopeSegment* Create(const Vector& vecOrigin, const QAngle& angAngles, bool bAnchor);

};

class CClimbableRope : public CBaseAnimating
{
	DECLARE_CLASS(CClimbableRope, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	void Spawn();
	void Precache();
	void SpawnSegments();
	void UpdateTraces();
	void ReenableGrab();
	void SetChildren();

	virtual int UpdateTransmitState();

	virtual void OnRestore();
	typedef CHandle<CClimbRopeSegment> CClimbRopeSegmentHandle;
	CNetworkArray(CClimbRopeSegmentHandle, rpSegment, MAX_ROPE_SEGMENTS);

	void CreateConstraints();

	CBaseEntity* GetNearestRopePoint(CBasePlayer* player);
	CNetworkHandle(CClimbRopeSegment, rpNearestSegment);

	
private:
	CNetworkVar(float, m_fRopeWidth);
	CNetworkVar(int, m_iRopeSegments);


	bool bPlayerConnected = false;

	const char* szEndEntity;

	bool bCanBeGrabbed;
};

