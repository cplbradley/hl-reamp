#include "cbase.h"
#include "beam_shared.h"
#include "vphysics_interface.h"
#include "baseplayer_shared.h"
#include "debugoverlay_shared.h"
#include "rope_helpers.h"
#include "rope_shared.h"

#include "tier0/memdbgon.h"

#define USE_MESH_ROPES 0

#define DEFAULT_ROPE "cable/rope_vlg.vmt"
#define MAX_ROPE_SEGMENTS 32


class C_ClimbRopeSegment : public C_BaseAnimating
{
	DECLARE_CLASS(C_ClimbRopeSegment, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();
public:
	void Spawn();
	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t type);
	bool bAmGrabbed;
	bool bAmAnchor;
	float fWidth;
	CHandle<C_ClimbRopeSegment> hNextSegment;
	int nearestindex;
	virtual int DrawModel(int flags);
	bool bDrawRopes;

	virtual bool ShouldDraw() { return bDrawRopes; }

	CBeam* beamrope;

	void CreateRopeSides(CMeshBuilder& builder, Vector& vecStart, Vector& vecEnd, float fRadius);
	void CreateBeams();

};
IMPLEMENT_CLIENTCLASS_DT(C_ClimbRopeSegment,DT_RopeSegment,CClimbRopeSegment)
RecvPropEHandle(RECVINFO(hNextSegment)),
RecvPropFloat(RECVINFO(fWidth)),
END_RECV_TABLE()

void C_ClimbRopeSegment::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
		Spawn();

	BaseClass::OnDataChanged(type);
}

void C_ClimbRopeSegment::Spawn()
{
	ConDColorMsg(Color(0,255,255,255),"Rope Segment Client Spawn\n");
	SetNextClientThink(CLIENT_THINK_ALWAYS);
	bDrawRopes = true;
	
}

void C_ClimbRopeSegment::ClientThink()
{
#if USE_MESH_ROPES
#else
	CreateBeams();
#endif
	BaseClass::ClientThink();

}
void C_ClimbRopeSegment::CreateBeams()
{
	if (!hNextSegment)
		return;

#if USE_MESH_ROPES
		CMeshBuilder builder;
		CMatRenderContextPtr pRenderContext(materials);
		IMaterial* pMat = materials->FindMaterial(DEFAULT_ROPE, TEXTURE_GROUP_OTHER);
		IMesh* mesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, pMat);
		builder.Begin(mesh, MATERIAL_QUADS, 16);

		Vector vecStart = GetAbsOrigin();
		Vector vecEnd = hNextSegment->GetAbsOrigin();

		CreateRopeSides(builder, vecStart, vecEnd, fWidth);
		builder.End();

		mesh->Draw();
		pRenderContext.SafeRelease();
#else
	if (!beamrope)
		beamrope = CBeam::BeamCreate("cable/rope.vmt", fWidth * 2);

	beamrope->SetStartPos(this->GetAbsOrigin());
	beamrope->SetEndPos(hNextSegment->GetAbsOrigin());
	beamrope->RelinkBeam();

	if (!bDrawRopes)
		beamrope->AddEffects(EF_NODRAW);
	else
		beamrope->ClearEffects();
#endif
}

void C_ClimbRopeSegment::CreateRopeSides(CMeshBuilder& meshbuilder, Vector& vecStart, Vector& vecEnd, float fRadius)
{
	Vector vecNormal = vecEnd - vecStart;
	Vector sideVector = vecNormal.Cross(Vector(0, 0, 1)).Normalized();

	for (int i = 0; i < 8; i++)
	{
		double angle = 2.0f * M_PI * (float)i / 8;
		double angledif = 2.0f * M_PI / 8;

		Vector p0 = vecStart + fRadius * (cos(angle) * sideVector + sin(angle) * Vector(0, 0, 1));
		Vector p1 = vecStart + fRadius * (cos(angle + angledif) * sideVector + sin(angledif) * Vector(0, 0, 1));
		Vector p2 = p0 + vecNormal;
		Vector p3 = p1 + vecNormal;
		Vector vecNorm1 = (p0 - vecStart).Normalized();
		Vector vecNorm2 = (p1 - vecEnd).Normalized();
		Vector vecNorm3 = (p2 - vecStart).Normalized();
		Vector vecNorm4 = (p3 - vecEnd).Normalized();

		float corner1 = 1 / fWidth;
		float corner2 = 1 / vecNormal.Length();

		float fNorm1[3] = { vecNorm1.x,vecNorm1.y,vecNorm1.z };
		float fNorm2[3] = { vecNorm2.x,vecNorm2.y,vecNorm2.z };
		float fNorm3[3] = { vecNorm3.x,vecNorm3.y,vecNorm3.z };
		float fNorm4[3] = { vecNorm4.x,vecNorm4.y,vecNorm4.z };

		meshbuilder.Position3fv(p0.Base());
		meshbuilder.Color4ub(0, 255, 255, 255);
		meshbuilder.Normal3fv(fNorm1);
		meshbuilder.TexCoord2f(0, 0, 0);
		meshbuilder.AdvanceVertex();

		meshbuilder.Position3fv(p1.Base());
		meshbuilder.Color4ub(255, 255, 255, 255);
		meshbuilder.Normal3fv(fNorm2);
		meshbuilder.TexCoord2f(0, 0, corner2);
		meshbuilder.AdvanceVertex();

		meshbuilder.Position3fv(p2.Base());
		meshbuilder.Color4ub(0, 255, 255, 255);
		meshbuilder.Normal3fv(fNorm3);
		meshbuilder.TexCoord2f(0, corner1, 0);
		meshbuilder.AdvanceVertex();

		meshbuilder.Position3fv(p3.Base());
		meshbuilder.Color4ub(0, 255, 255, 255);
		meshbuilder.Normal3fv(fNorm4);
		meshbuilder.TexCoord2f(0, corner1, corner2);
		meshbuilder.AdvanceVertex();

	}

	for (int i = 0; i < 8; ++i) {
		int nextIndex = (i + 1) % 8;

		meshbuilder.FastIndex(i * 4);
		meshbuilder.FastIndex(i * 4 + 1);
		meshbuilder.FastIndex(nextIndex * 4);

		meshbuilder.FastIndex(i * 4 + 1);
		meshbuilder.FastIndex(nextIndex * 4 + 1);
		meshbuilder.FastIndex(nextIndex * 4);
	}
}

int C_ClimbRopeSegment::DrawModel(int flags)
{
#if USE_MESH_ROPES
	CreateBeams();
#endif
	if (!bDrawRopes)
		return 0;
	return BaseClass::DrawModel(flags);
}

class C_ClimbableRope : public C_BaseAnimating
{
	DECLARE_CLASS(C_ClimbableRope, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

public:
	C_ClimbableRope();
	void Spawn();
	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t type);
	void UpdateTraces();
	virtual int DrawModel(int flags);


	
	virtual bool ShouldDraw() {
		return true;
	}
private:
	CHandle<C_ClimbRopeSegment> rpSegment[MAX_ROPE_SEGMENTS];
	CHandle<C_ClimbRopeSegment> rpNearestSegment;
	float m_fRopeWidth;
	int m_iRopeSegments;
	char ropetexture[MAX_PATH];
	Vector vecOffsetRopePos;
	CBeam* ropeBeam[MAX_ROPE_SEGMENTS];
	int iActualNumofSegments;
};

IMPLEMENT_CLIENTCLASS_DT(C_ClimbableRope, DT_ClimbableRope, CClimbableRope)
RecvPropArray3(RECVINFO_ARRAY(rpSegment),RecvPropEHandle(RECVINFO(rpSegment))),
RecvPropEHandle(RECVINFO(rpNearestSegment)),
RecvPropInt(RECVINFO(m_iRopeSegments)),
RecvPropFloat(RECVINFO(m_fRopeWidth)),
END_RECV_TABLE()

BEGIN_DATADESC(C_ClimbableRope)
DEFINE_AUTO_ARRAY(rpSegment,FIELD_EHANDLE)
END_DATADESC()

C_ClimbableRope::C_ClimbableRope()
{
	m_fRopeWidth = 0.0f;
	m_iRopeSegments = -1;
	vecOffsetRopePos.Init();
}

void C_ClimbableRope::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		Spawn();
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}

	BaseClass::OnDataChanged(type);
}

void C_ClimbableRope::Spawn()
{
	ConDColorMsg(Color(0,255,0,255),"Rope Client Spawn\n");
	PrecacheMaterial(DEFAULT_ROPE);
}

ConVar drawropestraces("drawropestraces", "0");

void C_ClimbableRope::ClientThink()
{
	UpdateTraces();
}

void C_ClimbableRope::UpdateTraces()
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;

	for (int i = 0; i < m_iRopeSegments; i++)
	{
		trace_t tr, tr2;
		C_ClimbRopeSegment* segment1 = rpSegment[i].Get();
		if (!segment1)
		{
			Msg("Rope Segment %i not found", i);
			return;
		}
		C_ClimbRopeSegment* segment2 = rpSegment[i + 1];
		if (!segment2)
		{
			return;
		}
		if (rpNearestSegment && rpSegment[i - 1] && rpSegment[i]->entindex() == rpNearestSegment->entindex())
		{
			if (player->m_bClimbingRope)
				rpSegment[i - 1]->bDrawRopes = false;
		}
		else
			rpSegment[i]->bDrawRopes = true;

		UTIL_TraceHull(segment1->GetAbsOrigin(), segment2->GetAbsOrigin(), Vector(-48, -48, -48), Vector(48, 48, 48), MASK_PLAYERSOLID_BRUSHONLY, NULL, &tr);
		UTIL_TraceLine(segment1->GetAbsOrigin(), segment2->GetAbsOrigin(), MASK_PLAYERSOLID_BRUSHONLY, NULL, &tr2);
		int red = 0;
		int green = 255;

		if (segment1->bAmGrabbed)
		{
			red = 255;
			green = 0;
		}

		if (drawropestraces.GetBool())
		{
			DebugDrawLine(tr2.startpos, tr2.endpos, red, green, 0, false, 0.05f);
			QAngle debugangle;
			Vector debugvector = (tr.endpos - tr.startpos).Normalized();
			VectorAngles(debugvector, debugangle);
			debugoverlay->AddSweptBoxOverlay(tr.startpos, tr.endpos, Vector(-48, -48, -48), Vector(48, 48, 48), debugangle, 0, green, red, 0, 0.05f);
			if(rpNearestSegment)
			NDebugOverlay::Cross3D(rpNearestSegment->GetAbsOrigin(), 16, 0, 255, 255, false, 0.05f);
		}

		segment1->bAmGrabbed = (tr.m_pEnt && tr.m_pEnt->IsPlayer());
	}

	if (rpNearestSegment)
		player->m_vecRopeSegmentPosition = rpNearestSegment->GetAbsOrigin();
}

int C_ClimbableRope::DrawModel(int flags)
{
	return BaseClass::DrawModel(flags);
}