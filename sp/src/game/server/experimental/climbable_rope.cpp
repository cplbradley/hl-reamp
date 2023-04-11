#include "cbase.h"
#include "beam_shared.h"
#include "experimental/climbable_rope.h"
#include "physconstraint.h"
#include "physobj.h"
#include "tier0/memdbgon.h"

#define DEFAULT_ROPE "cable/rope.vmt"
#define DEFAULT_MODEL "models/props_junk/popcan01a.mdl"


LINK_ENTITY_TO_CLASS(climb_rope_segment, CClimbRopeSegment);
BEGIN_DATADESC(CClimbRopeSegment)
END_DATADESC()


CClimbRopeSegment* CClimbRopeSegment::Create(const Vector& vecOrigin, const QAngle& angAngles, bool bAnchor)
{
	CClimbRopeSegment* segment = (CClimbRopeSegment*)CreateEntityByName("climb_rope_segment");
	UTIL_SetOrigin(segment, vecOrigin);
	UTIL_SetSize(segment, Vector(-4, -4, -4), Vector(4, 4, 4));
	segment->Spawn();
	segment->bAmAnchor = bAnchor;
	return segment;
}


void CClimbRopeSegment::Spawn()
{
	Precache();
	SetModel(DEFAULT_MODEL);
	VPhysicsCreate();

}
bool CClimbRopeSegment::VPhysicsCreate()
{
	if (bAmAnchor)
		VPhysicsInitStatic();
	else
		VPhysicsInitNormal(SOLID_NONE, FSOLID_NOT_STANDABLE, false);

	return true;
}
void CClimbRopeSegment::Precache()
{
	PrecacheModel(DEFAULT_MODEL);
}

LINK_ENTITY_TO_CLASS(climbable_rope, CClimbableRope);
BEGIN_DATADESC(CClimbableRope)
END_DATADESC()

void CClimbableRope::Spawn()
{
	Precache();
	SpawnSegements();
	CreateBeams();
	CreateConstraints();

}

void CClimbableRope::Precache()
{
	PrecacheMaterial(DEFAULT_ROPE);
}

void CClimbableRope::SpawnSegements()
{
	Vector vecOrigin = GetAbsOrigin();

	float length = m_fRopeLength / 8.0f;

	for (int i = 0; i < 8; i++)
	{
		
		float offset = length * i;
		Vector offsetPos = Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z - offset);
		bool anchor;

		if (!i)
			anchor = true;
		else
			anchor = false;

		CClimbRopeSegment* pSegment = CClimbRopeSegment::Create(offsetPos, vec3_angle, anchor);
		
			
		rpSegment[i] = pSegment;

		m_vSegmentList.AddToTail(rpSegment[i]);
	}
}

bool CClimbableRope::CreateBeams()
{
	for (int i = 0; i < m_vSegmentList.Count(); i++)
	{
		if (!ropeBeam[i])
			ropeBeam[i] = CBeam::BeamCreate(DEFAULT_ROPE, 8.0f);

		Vector pos[2];
		QAngle ang[2];
		rpSegment[i]->GetAbsOrigin();
		rpSegment[i + 1]->GetAbsOrigin();
		ropeBeam[i]->SetStartPos(pos[0]);
		ropeBeam[i]->SetEndPos(pos[1]);
	}
	return true;
}

void CClimbableRope::CreateConstraints()
{
	constraint_groupparams_t group;
	IPhysicsConstraintGroup* pGroup = physenv->CreateConstraintGroup(group);

	for (int i = 0; i < m_vSegmentList.Count(); i++)
	{
		hl_constraint_info_t info;
		info.pObjects[0] = m_vSegmentList.Element(i)->VPhysicsGetObject();
		info.pObjects[1] = m_vSegmentList.Element(i + 1)->VPhysicsGetObject();
		Vector position[2];
		QAngle angle[2];
		info.pObjects[0]->GetPosition(&position[0], &angle[0]);
		info.pObjects[1]->GetPosition(&position[1], &angle[1]);
		constraint_lengthparams_t params;
		params.InitWorldspace(info.pObjects[0], info.pObjects[1], position[0], position[1], true);
		float length = (position[0] - position[1]).Length();
		params.minLength = length;
		params.totalLength = length;

		physenv->CreateLengthConstraint(info.pObjects[0], info.pObjects[1], pGroup, params);
	}
}