#include "cbase.h"
#include "beam_shared.h"
#include "experimental/climbable_rope.h"
#include "physobj.h"
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS(climb_rope_segment, CClimbRopeSegment);


CClimbRopeSegment* CClimbRopeSegment::Create(const Vector& vecOrigin, const QAngle& angAngles, CBaseEntity* pOwner)
{
	CClimbRopeSegment* segment = (CClimbRopeSegment*)CreateEntityByName("climb_rope_segment");
	UTIL_SetOrigin(segment, vecOrigin);
	segment->Spawn();
	return segment;
}


void CClimbRopeSegment::Spawn()
{
	
}




LINK_ENTITY_TO_CLASS(climbable_rope, CClimbableRope);


void CClimbableRope::Spawn()
{
	Precache();
	SpawnSegements();
	CreateBeams();

}

void CClimbableRope::Precache()
{
	PrecacheMaterial(szRopeName);
}

void CClimbableRope::SpawnSegements()
{
	Vector vecOrigin = GetAbsOrigin();

	float length = m_fRopeLength / 8.0f;

	for (int i = 0; i < 8; i++)
	{
		float offset = length * i;
		Vector offsetPos = Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z + offset);
		CClimbRopeSegment* pSegment = CClimbRopeSegment::Create(offsetPos, vec3_angle);
		if (!rpSegment[i])
			rpSegment[i] = pSegment;

		m_vSegmentList.AddToTail(rpSegment[i]);
	}
}

bool CClimbableRope::CreateBeams()
{
	for (int i = 0; m_vSegmentList.Count(); i++)
	{
		if (!ropeBeam[i])
			ropeBeam[i] = CBeam::BeamCreate(szRopeName, m_fRopeWidth);

		ropeBeam[i]->SetStartPos(rpSegment[i].Get()->GetAbsOrigin());
		ropeBeam[i]->SetEndPos(rpSegment[i + 1].Get()->GetAbsOrigin());
	}
	return true;
}