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

	static CClimbRopeSegment* Create(const Vector& vecOrigin, const QAngle& angAngles, bool bAnchor);

};

class CClimbRopeMotor : public CBaseAnimating
{
	DECLARE_CLASS(CClimbRopeMotor, CBaseAnimating);
	DECLARE_DATADESC();
public:
	void Spawn();
	void UpdatePosition(Vector vecPos);
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

	void UpdateTraces();

	void MoveMotorAlongPath(CClimbRopeMotor* motor, int nearestIndex, float flProgress);
	int FindNearestPointAlongPath(Vector vecNearPos);
	CClimbRopeMotor* CreateMotor(int nearestIndex, float flProgress);

	float GetProgressOnPath(Vector vecMearPos);
	CClimbRopeMotor* pMotor;



	CHandle<CClimbRopeSegment> rpSegment[8];
	CHandle<CBeam> ropeBeam[7];
	CHandle<CClimbRopeMotor*> m_hMotor;

	void CreateConstraints();
	

private:
//	CUtlVector<CClimbRopeSegment*> m_vSegmentList;
	float m_fRopeLength = 512.0f;
	float m_fRopeWidth;
	const char* szRopeName;

	Vector vecSegmentPos[8];
	CUtlVector<Vector*> m_vSegmentPos;
};


Vector secretlerpfunction(Vector a, Vector b, float t);