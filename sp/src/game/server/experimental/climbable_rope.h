#include "cbase.h"
#include "rope.h"
#include "beam_shared.h"
#include "vphysics_interface.h"
#include "physobj.h"

#pragma once


class CClimbRopeSegment : public CBaseAnimating
{
	DECLARE_CLASS(CClimbRopeSegment, CBaseAnimating);
	DECLARE_DATADESC();
public:

	void Spawn();
	
	void Precache();

	bool VPhysicsCreate();

	bool bAmAnchor;

	static CClimbRopeSegment* Create(const Vector& vecOrigin, const QAngle& angAngles, bool bAnchor = false);

};

class CClimbableRope : public CBaseEntity
{
	DECLARE_CLASS(CClimbableRope, CBaseEntity);
	DECLARE_DATADESC();

public:
	void Spawn();
	void Precache();
	void SpawnSegements();
	bool CreateBeams();



	CHandle<CClimbRopeSegment> rpSegment[8];
	CHandle<CBeam> ropeBeam[7];

	void CreateConstraints();
	

private:
	CUtlVector<CClimbRopeSegment*> m_vSegmentList;
	float m_fRopeLength = 256.0f;
	float m_fRopeWidth;
	const char* szRopeName;
};