#include "cbase.h"
#include "rope.h"
#include "beam_shared.h"

#pragma once


class CClimbRopeSegment : public CBaseEntity
{
	DECLARE_CLASS(CClimbRopeSegment, CBaseEntity);
	//DECLARE_DATADESC();
public:

	void Spawn();
	//void Precache();


	static CClimbRopeSegment* Create(const Vector& vecOrigin, const QAngle& angAngles, CBaseEntity* pOwner = NULL);

};

class CClimbableRope : public CBaseEntity
{
	DECLARE_CLASS(CClimbableRope, CBaseEntity);
	//DECLARE_DATADESC();

public:
	void Spawn();
	void Precache();
	void SpawnSegements();
	bool CreateBeams();



	CHandle<CClimbRopeSegment> rpSegment[8];
	CHandle<CBeam> ropeBeam[7];

	

private:
	CUtlVector<CClimbRopeSegment*> m_vSegmentList;
	float m_fRopeLength;
	float m_fRopeWidth;
	const char* szRopeName;
};