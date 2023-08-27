#include "cbase.h"
#include "beam_shared.h"
#include "experimental/climbable_rope.h"
#include "physconstraint.h"
#include "physobj.h"

#include "gamemovement.h"
#include "in_buttons.h"

#include "tier0/memdbgon.h"

#define DEFAULT_ROPE "cable/rope.vmt"
#define DEFAULT_MODEL "models/utils/rope_joint.mdl"
#define ROPE_KNOT "models/utils/rope_knot.mdl"


extern IGameMovement* g_pGameMovement;

LINK_ENTITY_TO_CLASS(climb_rope_segment, CClimbRopeSegment);
BEGIN_DATADESC(CClimbRopeSegment)
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CClimbRopeSegment,DT_RopeSegment)
SendPropEHandle(SENDINFO(hNextSegment)),
SendPropFloat(SENDINFO(fWidth)),
END_SEND_TABLE()


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
	//AddEffects(EF_NODRAW);
	VPhysicsCreate();
	SetTransmitState(FL_EDICT_ALWAYS);
	ConColorMsg(Color(255, 255, 0, 255), "Rope Segment Server Spawn\n");
}
bool CClimbRopeSegment::VPhysicsCreate()
{
	return VPhysicsInitNormal(SOLID_VPHYSICS, FSOLID_NOT_SOLID, false);
}
void CClimbRopeSegment::Precache()
{
	PrecacheModel(DEFAULT_MODEL);
	PrecacheModel(ROPE_KNOT);
}

int CClimbRopeSegment::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}

LINK_ENTITY_TO_CLASS(climbable_rope, CClimbableRope);
BEGIN_DATADESC(CClimbableRope)
DEFINE_AUTO_ARRAY(rpSegment,FIELD_EHANDLE),
DEFINE_FIELD(rpNearestSegment,FIELD_EHANDLE),
DEFINE_KEYFIELD(m_fRopeWidth,FIELD_FLOAT,"RopeWidth"),
DEFINE_KEYFIELD(m_iRopeSegments,FIELD_INTEGER,"RopeSegments"),
DEFINE_KEYFIELD(szEndEntity,FIELD_STRING,"EndPointEnt"),
DEFINE_FIELD(bCanBeGrabbed,FIELD_BOOLEAN),
DEFINE_THINKFUNC(UpdateTraces),


END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CClimbableRope,DT_ClimbableRope)
SendPropArray3(SENDINFO_ARRAY3(rpSegment),SendPropEHandle(SENDINFO_ARRAY(rpSegment))),
SendPropEHandle(SENDINFO(rpNearestSegment)),
SendPropInt(SENDINFO(m_iRopeSegments)),
SendPropFloat(SENDINFO(m_fRopeWidth)),
END_SEND_TABLE()

void CClimbableRope::Spawn()
{
	Precache();
	SpawnSegments();
	CreateConstraints();
	SetChildren();
	SetThink(&CClimbableRope::UpdateTraces);
	SetNextThink(gpGlobals->curtime);
	SetTransmitState(FL_EDICT_ALWAYS);
	RegisterThinkContext("GrabEnable");
	bCanBeGrabbed = true;
	SetModel(DEFAULT_MODEL);
}

void CClimbableRope::Precache()
{
	PrecacheModel(DEFAULT_MODEL);
}

void CClimbableRope::OnRestore()
{
	BaseClass::OnRestore();
	Spawn();
}
int CClimbableRope::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}

void CClimbableRope::SpawnSegments()
{
	CBaseEntity* pTarget = gEntList.FindEntityByName(NULL, szEndEntity);

	if (!pTarget)
		return;

	Vector vecEnd = pTarget->GetAbsOrigin();
	Vector vecOrigin = GetAbsOrigin();
	Vector vecDir = (vecEnd - vecOrigin).Normalized();

	float absLength = (vecEnd - vecOrigin).Length();
	

	float length = absLength / ((float)m_iRopeSegments - 1);

	for (int i = 0; i < m_iRopeSegments; i++)
	{
		float offset = length * i;
		Vector offsetPos = vecOrigin + (vecDir * offset);
		bool anchor;

		if (i == 0)
		{
			anchor = true;
			Msg("Anchor = true\n");
		}
		else
			anchor = false;

		if (!rpSegment.Get(i))
		{
			CClimbRopeSegment* segment = CClimbRopeSegment::Create(offsetPos, vec3_angle, anchor);
			rpSegment.Set(i, segment);
			rpSegment[i]->fWidth = m_fRopeWidth;
		}
	}

	rpSegment[m_iRopeSegments - 1]->SetModel(ROPE_KNOT);
	rpSegment[m_iRopeSegments - 1]->ClearEffects();
}

void CClimbableRope::CreateConstraints()
{
	constraint_groupparams_t group;
	group.Defaults();
	group.additionalIterations = m_iRopeSegments * 2;
	IPhysicsConstraintGroup* pGroup = physenv->CreateConstraintGroup(group);

	for (int i = 0; i < m_iRopeSegments; i++)
	{
		hl_constraint_info_t info;

		IPhysicsConstraint* pConstraint;
		constraint_ballsocketparams_t params;
		params.Defaults();

		CClimbRopeSegment* ropeseg1 = rpSegment.Get(i);
		CClimbRopeSegment* ropeseg2 = rpSegment.Get(i+1);

		if (!ropeseg1 || !ropeseg2)
		{
			Msg("Rope segment %i not found?\n", i);
			continue;
		}

		bool motion;
		i == 0 ? motion = false : motion = true;

		ropeseg1->VPhysicsGetObject()->EnableMotion(motion);
		ropeseg1->VPhysicsGetObject()->Wake();

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
		info.pObjects[0]->Wake();
		info.pObjects[1]->Wake();


	}
	pGroup->Activate();
}

void CClimbableRope::UpdateTraces()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;

	if (!bCanBeGrabbed)
		return;

	for (int i = 0; i < m_iRopeSegments; i++)
	{
		trace_t tr, tr2;
		CClimbRopeSegment* segment1 = rpSegment.Get(i);
		CClimbRopeSegment* segment2 = rpSegment.Get(i + 1);

		if (!segment2)
			break;

		Vector scale = Vector(0.5, 0.5, 1);
		int red = 0;
		int green = 255;

		if (segment1->bAmGrabbed)
		{
			red = 255;
			green = 0;
		}
		UTIL_TraceHull(segment1->GetAbsOrigin(), segment2->GetAbsOrigin(), Vector(-48,-48,-48), Vector(48, 48, 48), MASK_PLAYERSOLID_BRUSHONLY, NULL, &tr);
		UTIL_TraceLine(segment1->GetAbsOrigin(), segment2->GetAbsOrigin(), MASK_ALL, NULL, &tr2);

		if (tr2.m_pEnt && tr2.m_pEnt->IsPlayer())
		{
			if (!pPlayer->m_bClimbingRope)
			{
				rpSegment[i]->VPhysicsGetObject()->ApplyForceCenter(pPlayer->GetAbsVelocity() * 10);
				pPlayer->m_bClimbingRope = true;
				pPlayer->SetMoveType(MOVETYPE_ROPE);
			}
		}
		if (tr.m_pEnt && tr.m_pEnt->IsPlayer() && pPlayer->m_bClimbingRope)
		{
			rpSegment.Get(i)->bAmGrabbed = true;
			pPlayer->curRope = this;
		}
		else
			rpSegment.Get(i)->bAmGrabbed = false;
	}
	Vector viewAng, flatAng;
	AngleVectors(pPlayer->EyeAngles(), &viewAng);
	flatAng = viewAng;
	flatAng[2] = 0;
	float fDot = DotProduct(flatAng, viewAng);

	bool viewAngShouldClimb = (fDot < 0.1f);


	if (pPlayer->GetGroundEntity() != NULL)
	{
		pPlayer->m_bClimbingRope = false;
		pPlayer->curRope = nullptr;
	}


	if (pPlayer->m_bClimbingRope)
	{
		for (int i = 0; i < m_iRopeSegments; i++)
		{
			bPlayerConnected = rpSegment.Get(i)->bAmGrabbed;
		}
	}

	CBaseEntity* pEnt = GetNearestRopePoint(pPlayer);

	if (pEnt)
	{
		rpNearestSegment = dynamic_cast<CClimbRopeSegment*>(pEnt);
		if (rpNearestSegment->VPhysicsGetObject())
		{
			Vector vecVel, vecPos;
			rpNearestSegment->VPhysicsGetObject()->GetVelocity(&vecVel, NULL);
			rpNearestSegment->VPhysicsGetObject()->GetPosition(&vecPos, NULL);
			pPlayer->m_vecRopeSegmentVelocity = vecVel;
		}
	}

	CClimbRopeSegment* nextsegmentup = nullptr;
	CClimbRopeSegment* nextsegmentdown = nullptr;

	for (int i = 0; i < m_iRopeSegments; i++)
	{
		if (rpSegment.Get(i) && pEnt && rpSegment[i]->entindex() == pEnt->entindex())
		{
			nextsegmentup = rpSegment[i - 1];
			nextsegmentdown = rpSegment[i + 1];
		}
	}
	if (viewAng[2] > 0)
	{
		if (pPlayer->m_nButtons & IN_BACK)
		{
			if (!nextsegmentdown)
			{
				pPlayer->m_vecRopeSegmentNormal = vec3_origin;
			}
			else
			{
				pPlayer->m_vecRopeSegmentNormal = pPlayer->WorldSpaceCenter() - nextsegmentdown->GetAbsOrigin();
				//DebugDrawLine(pPlayer->WorldSpaceCenter(), nextsegmentdown->GetAbsOrigin(), 255, 255, 0, false, 0.05f);
			}
		}
		else
		{
			if (nextsegmentup && nextsegmentup->bAmAnchor)
			{
				pPlayer->m_vecRopeSegmentNormal = vec3_origin;
			}
			else if(nextsegmentup)
			{
				pPlayer->m_vecRopeSegmentNormal = -(pPlayer->WorldSpaceCenter() - nextsegmentup->GetAbsOrigin());
				//DebugDrawLine(pPlayer->WorldSpaceCenter(), nextsegmentup->GetAbsOrigin(), 255, 255, 0, false, 0.05f);
			}
		}
	}
	else
	{
		if (pPlayer->m_nButtons & IN_BACK)
		{
			if (nextsegmentup && nextsegmentup->bAmAnchor)
			{
				pPlayer->m_vecRopeSegmentNormal = vec3_origin;
			}
			else if(nextsegmentup)
			{
				pPlayer->m_vecRopeSegmentNormal = pPlayer->WorldSpaceCenter() - nextsegmentup->GetAbsOrigin();
				DebugDrawLine(pPlayer->WorldSpaceCenter(), nextsegmentup->GetAbsOrigin(), 255, 255, 0, false, 0.05f);
			}
		}
		else
		{
			if (!nextsegmentdown)
			{
				pPlayer->m_vecRopeSegmentNormal = vec3_origin;
			}
			else if (nextsegmentdown)
			{
				pPlayer->m_vecRopeSegmentNormal = -(pPlayer->WorldSpaceCenter() - nextsegmentdown->GetAbsOrigin());
				DebugDrawLine(pPlayer->WorldSpaceCenter(), nextsegmentdown->GetAbsOrigin(), 255, 255, 0, false, 0.05f);
			}
		}
	}

	if (!viewAngShouldClimb && pEnt && pEnt->VPhysicsGetObject())
	{
		Vector vForward, vRight, vUp, vPushDir;
		int right, forward;
		AngleVectors(pPlayer->EyeAngles(), &vForward, &vRight, &vUp);

		if (pPlayer->m_nButtons & IN_FORWARD || pPlayer->m_nButtons & IN_BACK || pPlayer->m_nButtons & IN_RIGHT || pPlayer->m_nButtons & IN_LEFT)
		{
			if (pPlayer->m_nButtons & IN_FORWARD)
				forward = 1;
			else if (pPlayer->m_nButtons & IN_BACK)
				forward = -1;
			else
				forward = 0;

			if (pPlayer->m_nButtons & IN_RIGHT)
				right = 1;
			else if (pPlayer->m_nButtons & IN_LEFT)
				right = -1;
			else
				right = 0;

			Vector vecPushDir = vForward * (40 * forward) + vRight * (40 * right);
			if (!pPlayer->curRope)
				vecPushDir *= 0.001f;
			pEnt->VPhysicsGetObject()->ApplyForceCenter(vecPushDir);
		}

	}

	if (pPlayer->curRope && pPlayer->curRope == this && pPlayer->m_nButtons & IN_JUMP)
	{
		bCanBeGrabbed = false;
		SetContextThink(&CClimbableRope::ReenableGrab, gpGlobals->curtime + 1.0f, "GrabEnable");
	}

	SetNextThink(gpGlobals->curtime);

}

void CClimbableRope::ReenableGrab()
{
	bCanBeGrabbed = true;
	SetThink(&CClimbableRope::UpdateTraces);
	SetNextThink(gpGlobals->curtime);
}
CBaseEntity* CClimbableRope::GetNearestRopePoint(CBasePlayer* player)
{
	CBaseEntity* pSearch = NULL;
	pSearch = gEntList.FindEntityByClassnameNearest("climb_rope_segment", player->WorldSpaceCenter(), 256.0f);
	return pSearch;
}

void CClimbableRope::SetChildren()
{
	for (int i = 0; i < m_iRopeSegments; i++)
	{
		if (rpSegment[i] && rpSegment[i + 1])
			rpSegment[i]->hNextSegment.Set(rpSegment[i + 1]);
	}
}