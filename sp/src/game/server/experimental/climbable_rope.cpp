#include "cbase.h"
#include "beam_shared.h"
#include "experimental/climbable_rope.h"
#include "physconstraint.h"
#include "physobj.h"
#include "tier0/memdbgon.h"

#define DEFAULT_ROPE "cable/rope.vmt"
#define DEFAULT_MODEL "models/items/boxsrounds.mdl"




LINK_ENTITY_TO_CLASS(climb_rope_motor, CClimbRopeMotor);
BEGIN_DATADESC(CClimbRopeMotor)
END_DATADESC()


void CClimbRopeMotor::Spawn()
{
	PrecacheModel(DEFAULT_MODEL);
	SetModel(DEFAULT_MODEL);
	SetRenderColor(255, 0, 0, 255);
}

void CClimbRopeMotor::UpdatePosition(Vector vecPos)
{
	SetAbsOrigin(vecPos);
}
LINK_ENTITY_TO_CLASS(climb_rope_segment, CClimbRopeSegment);
BEGIN_DATADESC(CClimbRopeSegment)
END_DATADESC()


CClimbRopeSegment* CClimbRopeSegment::Create(const Vector& vecOrigin, const QAngle& angAngles, bool bAnchor)
{
	CClimbRopeSegment* segment = (CClimbRopeSegment*)CreateEntityByName("climb_rope_segment");
	UTIL_SetOrigin(segment, vecOrigin);
	UTIL_SetSize(segment, Vector(-4, -4, -4), Vector(4, 4, 4));
	segment->bAmAnchor = bAnchor;
	segment->Spawn();
	return segment;
}


void CClimbRopeSegment::Spawn()
{
	Precache();
	SetModel(DEFAULT_MODEL);
	SetMoveType(MOVETYPE_VPHYSICS);
	SetSolid(SOLID_VPHYSICS);
	//SetRenderMode(kRenderNone);
	VPhysicsCreate();
}
bool CClimbRopeSegment::VPhysicsCreate()
{
	/*if (bAmAnchor)
	{
		Msg("I am an anchor\n");
			
		if (VPhysicsInitStatic())
			Msg("Creating Static VPhysics\n");
		else
			Msg("Failed to create Static VPhysics\n");
	}
	else
	{
		if (VPhysicsInitNormal(SOLID_VPHYSICS, FSOLID_NOT_STANDABLE, false))
		{
			Msg("Creating Dynamic VPhysics\n");
		}
	}*/
	
	return VPhysicsInitNormal(SOLID_VPHYSICS, FSOLID_NOT_SOLID, false);
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
	SetThink(&CClimbableRope::UpdateTraces);
	SetNextThink(gpGlobals->curtime);
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

		if (i == 0)
		{
			anchor = true;
			Msg("Anchor = true\n");
		}
		else
			anchor = false;

		rpSegment[i] = CClimbRopeSegment::Create(offsetPos, vec3_angle, anchor);
		vecSegmentPos[i] = rpSegment[i]->GetAbsOrigin();
		
			
		// = pSegment;

	//	m_vSegmentList.AddToTail(rpSegment[i]);
	}
}

bool CClimbableRope::CreateBeams()
{
	for (int i = 0; i < 8; i++)
	{
		if (!ropeBeam[i])
			ropeBeam[i] = CBeam::BeamCreate(DEFAULT_ROPE, 8.0f);

		ropeBeam[i]->SetStartEntity(rpSegment[i].Get());
		ropeBeam[i]->SetEndEntity(rpSegment[i + 1].Get());
		ropeBeam[i]->RelinkBeam();
		ropeBeam[i]->Activate();
	}

	return true;
}

void CClimbableRope::CreateConstraints()
{
	constraint_groupparams_t group;
	group.Defaults();
	group.additionalIterations = 8;
	IPhysicsConstraintGroup* pGroup = physenv->CreateConstraintGroup(group);

	for (int i = 0; i < 8; i++)
	{
		hl_constraint_info_t info;

		IPhysicsConstraint* pConstraint;
		constraint_ballsocketparams_t params;
		params.Defaults();

		CClimbRopeSegment* ropeseg1 = rpSegment[i].Get();
		CClimbRopeSegment* ropeseg2 = rpSegment[i + 1].Get();

		if (!ropeseg1 || !ropeseg2)
		{
			Msg("Rope segment %i not found?\n", i);
			continue;
		}

		bool motion;
		i == 0 ? motion = false : motion = true;

		ropeseg1->VPhysicsGetObject()->EnableMotion(motion);
		ropeseg1->VPhysicsGetObject()->Wake();

		float drag = 0.8f;
		ropeseg1->VPhysicsGetObject()->SetDragCoefficient(&drag, &drag);

		IPhysicsObject* pObject2 = ropeseg2->VPhysicsGetObject();
	

		if (!pObject2)
		{
			Msg("Rope segment VPhys not found?\n");
			continue;
		}

		info.pObjects[0] = ropeseg1->VPhysicsGetObject();
		info.pObjects[1] = ropeseg2->VPhysicsGetObject();

		
		
		

		info.pObjects[0]->WorldToLocal(&params.constraintPosition[0], ropeseg1->GetAbsOrigin());
		info.pObjects[1]->WorldToLocal(&params.constraintPosition[1], ropeseg2->GetAbsOrigin());

		//params.constraint.torqueLimit = 0;

		params.InitWithCurrentObjectState(info.pObjects[0], info.pObjects[1], ropeseg1->GetAbsOrigin());

		Msg("i should be working now\n");
		pConstraint = physenv->CreateBallsocketConstraint(info.pObjects[0],info.pObjects[1], pGroup, params);
		pConstraint->Activate();

	}

	pGroup->Activate();
}


void CClimbableRope::UpdateTraces()
{

	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;

	for (int i = 0; i < 8; i++)
	{
		trace_t tr;
		CClimbRopeSegment* segment1 = rpSegment[i];
		CClimbRopeSegment* segment2 = rpSegment[i + 1];

		vecSegmentPos[i] = segment1->GetAbsOrigin();

		if (!segment2)
			return;

		UTIL_TraceHull(segment1->GetAbsOrigin(), segment2->GetAbsOrigin(), pPlayer->GetPlayerMins(), pPlayer->GetPlayerMaxs(), MASK_PLAYERSOLID_BRUSHONLY, NULL, &tr);
		DebugDrawLine(tr.startpos, tr.endpos, 0, 255, 0, false, 0.1f);

		if (tr.m_pEnt && tr.m_pEnt->IsPlayer())
		{
			if (!pMotor)
			{
				pMotor = CreateMotor(FindNearestPointAlongPath(pPlayer->GetAbsOrigin()), GetProgressOnPath(pPlayer->GetAbsOrigin()));
			}
			if (!pPlayer->m_bClimbingRope)
			{
				rpSegment[i]->VPhysicsGetObject()->ApplyForceCenter(pPlayer->GetAbsVelocity());
				pPlayer->SetAbsVelocity(pPlayer->GetAbsVelocity() * 0);
				pPlayer->m_bClimbingRope = true;
				pPlayer->SetGravity(0.0f);
			//	pPlayer->SetMoveType(MOVETYPE_FLY);
			//	pPlayer->SetParent(pMotor);
			}

			MoveMotorAlongPath(pMotor, FindNearestPointAlongPath(pMotor->GetAbsOrigin()),1.0f);
		}
		else
		{
			pPlayer->SetParent(NULL);
			pPlayer->m_bClimbingRope = false;
		}
	}

	SetNextThink(gpGlobals->curtime);
}

void CClimbableRope::MoveMotorAlongPath(CClimbRopeMotor* motor, int nearestIndex, float flProgress)
{
	Vector vecPos = secretlerpfunction(vecSegmentPos[nearestIndex], vecSegmentPos[nearestIndex + 1], flProgress);

	motor->UpdatePosition(vecPos);
}

CClimbRopeMotor* CClimbableRope::CreateMotor(int nearestIndex, float flProgress)
{
	Vector spawnPos = secretlerpfunction(vecSegmentPos[nearestIndex], vecSegmentPos[nearestIndex + 1], flProgress);

	CClimbRopeMotor* pMotor = (CClimbRopeMotor*)CreateEntityByName("climb_rope_motor");
	UTIL_SetOrigin(pMotor, spawnPos);
	pMotor->Spawn();

	return pMotor;
}

Vector secretlerpfunction(Vector a, Vector b, float t)
{
	return a + (b - a) * t;
}

int CClimbableRope::FindNearestPointAlongPath(Vector vecNearPos)
{
	int nearestIndex = 0;
	float nearestDistance = FLT_MAX;

	for (int i = 0; i < 8; i++)
	{
		float distance = (vecSegmentPos[i] - vecNearPos).Length();

		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestIndex = i;
		}
	}

	return nearestIndex;
}

float CClimbableRope::GetProgressOnPath(Vector vecNearPos)
{
	int nearestIndex = FindNearestPointAlongPath(vecNearPos);
	float nearestDistance = (vecSegmentPos[nearestIndex] - vecNearPos).Length();

	float remainingDistance = 0.0f;

	for (int i = 0; i < 8; i++)
	{
		remainingDistance += (vecSegmentPos[i + 1] - vecSegmentPos[i]).Length();
	}

	float totalLength = 0.0f;
	for (int i = 0; i < 8; i++)
	{
		totalLength += (vecSegmentPos[i + 1] - vecSegmentPos[i]).Length();
	}

	float progress = 1.0f - ((nearestDistance + remainingDistance) / totalLength);

	return progress;
}