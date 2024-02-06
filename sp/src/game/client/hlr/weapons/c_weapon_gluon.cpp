#include "cbase.h"
#include "c_baseplayer.h"
#include "c_weapon__stubs.h"
#include "c_basecombatweapon.h"
#include "c_basehlcombatweapon.h"
#include "particle_parse.h"
#include "debugoverlay_shared.h"
#include "soundenvelope.h"
#include "hlr/hlr_shareddefs.h"
#include "beam_shared.h"

#define BEAM_SEGMENTS 16
#define SIMPLE_BEAM_MAT ""
float fuckedvalue(float scale)
{
	return sin(gpGlobals->curtime * scale);
}
float shitvalue(float scale)
{
	return cos(gpGlobals->curtime * scale);
}
Vector BezierCurve(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3, float t)
{
	float u = 1.0 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	Vector p;
	p.x = (uuu)*p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
	p.y = (uuu)*p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
	p.z = (uuu)*p0.z + 3 * uu * t * p1.z + 3 * u * tt * p2.z + ttt * p3.z;

	return p;
}
class C_GluonTarget : public C_BaseEntity
{
	DECLARE_CLASS(C_GluonTarget, C_BaseEntity);
	DECLARE_CLIENTCLASS();
public:
	virtual void OnDataChanged(DataUpdateType_t type);
	void SetDirection();
private:
	Vector m_vecDestination;
};

IMPLEMENT_CLIENTCLASS_DT(C_GluonTarget,DT_GluonTarget,CGluonTarget)
RecvPropVector(RECVINFO(m_vecDestination)),
END_RECV_TABLE()

void C_GluonTarget::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
		SetNextClientThink(CLIENT_THINK_ALWAYS);
}

void C_GluonTarget::SetDirection()
{
	Vector vecVec = m_vecDestination - GetAbsOrigin();
	Vector vecDir = vecVec.Normalized();
	float fLength = vecVec.Length();
	SetAbsVelocity(GetAbsOrigin() + (vecDir * fLength * 10.f));
}

ConVar gluon_spline_scale("gluon_spline_scale", "128");

class C_WeaponGluon : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponGluon, C_BaseHLCombatWeapon);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

public:
	C_WeaponGluon();
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink();
	virtual void DrawSimpleBeam(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4);
	virtual void DrawComplexBeam(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3);

	void StartFX();
	void StopFX();

private:

	CNewParticleEffect* beamFX[BEAM_SEGMENTS];
	CNewParticleEffect* muzzleFX;

	bool m_bFiring;
	Vector m_vecDamagePoint;

	CHandle<C_GluonTarget> pTarget;

	float fNextTick;
	float fPrevTick;
	float flInterval;


	CSoundPatch* m_sBaseLayer;
	CSoundPatch* m_sMidLayer;
	Vector vecnext;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_gluon,C_WeaponGluon)
IMPLEMENT_CLIENTCLASS_DT(C_WeaponGluon,DT_WeaponGluon,CWeaponGluon)
RecvPropBool(RECVINFO(m_bFiring)),
RecvPropVector(RECVINFO(m_vecDamagePoint)),
RecvPropEHandle(RECVINFO(pTarget)),
END_RECV_TABLE()


C_WeaponGluon::C_WeaponGluon()
{
	m_vecDamagePoint.Init();
	m_bFiring = false;
	flInterval = 0.01f;
}

void C_WeaponGluon::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
		PrecacheParticleSystem("gluon_centerbeam");
		PrecacheParticleSystem("gluon_muzzle");
		PrecacheScriptSound("Gluon.BaseLayer");
		PrecacheScriptSound("Gluon.MidLayer");
	}
}

void C_WeaponGluon::ClientThink()
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();

	if (!player)
		return;

	if (m_bFiring)
		StartFX();
	else
		StopFX();
}

void C_WeaponGluon::StartFX()
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;

	Vector vecStart, vecEnd, vecQuarter, vecThreeQuarter;

	player->GetViewModel()->GetAttachment(LookupAttachment("muzzle"), vecStart);

	if (pTarget)
	{
		vecEnd = pTarget->GetLocalOrigin();
	}

	trace_t tr;
	QAngle eyeAng;
	Vector vecDir, vecTracePoint;
	eyeAng = player->EyeAngles();
	AngleVectors(eyeAng, &vecDir);
	VectorNormalize(vecDir);
	vecTracePoint = player->Weapon_ShootPosition() + (vecDir * MAX_TRACE_LENGTH);

	UTIL_TraceLine(player->Weapon_ShootPosition(), vecTracePoint, MASK_SHOT, player, COLLISION_GROUP_NONE, &tr);
	
	if (pTarget)
	{
		pTarget->SetDirection();

	}

	Vector vecMidPoint = (tr.endpos + vecStart ) * 0.5f;
	Vector vecStartDir = (vecMidPoint - vecStart).Normalized();
	Vector vecEndDir = (vecEnd - vecMidPoint).Normalized();
	InterpolateVector(0.5f, vecStart, vecMidPoint, vecQuarter);
	InterpolateVector(0.5f, vecMidPoint, vecEnd, vecThreeQuarter);
	float wobble = (sin(gpGlobals->curtime) + fuckedvalue(1.2f) + fuckedvalue(0.8f)) * 10.f;
	vecQuarter += wobble;
	vecThreeQuarter += wobble * -1.f;
	Vector cp1, cp2;


	if (r_efficient_particles.GetBool())
	{
		Vector mid = (vecQuarter + vecThreeQuarter) * 0.5f;
		Vector midmid = (mid + vecMidPoint) * 0.5f;
		DrawSimpleBeam(vecStart, vecQuarter, midmid, vecThreeQuarter, vecEnd);
	}
	else
	{
		DrawComplexBeam(vecStart, vecQuarter, vecThreeQuarter, vecEnd);
	}

	if (!muzzleFX)
	{
		muzzleFX = player->GetViewModel()->ParticleProp()->Create("gluon_muzzle", PATTACH_POINT_FOLLOW, "muzzle");
	}

	CPASAttenuationFilter filter(this);
	if (!m_sBaseLayer)
	{
		m_sBaseLayer = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Gluon.BaseLayer");
		CSoundEnvelopeController::GetController().Play(m_sBaseLayer, 1.0f, 100.f);
		CSoundEnvelopeController::GetController().SoundChangePitch(m_sBaseLayer, 150, 15.f);
	}
	if (!m_sMidLayer)
	{
		m_sMidLayer = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Gluon.MidLayer");
		CSoundEnvelopeController::GetController().Play(m_sMidLayer, 0.f, 100.f);
		CSoundEnvelopeController::GetController().SoundChangeVolume(m_sMidLayer, 0.9f, 3.f);
		CSoundEnvelopeController::GetController().SoundChangePitch(m_sMidLayer, 150, 15.f);
	}


}

void C_WeaponGluon::StopFX()
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;

	if (m_sBaseLayer)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_sBaseLayer);
		m_sBaseLayer = NULL;
	}

	if (m_sMidLayer)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_sMidLayer);
		m_sMidLayer = NULL;
	}

	for (int i = 0; i < BEAM_SEGMENTS; i++)
	{
		if (beamFX[i])
		{
			player->GetViewModel()->ParticleProp()->StopEmissionAndDestroyImmediately(beamFX[i]);
			beamFX[i] = NULL;
		}
	}

	if (muzzleFX)
	{
		player->GetViewModel()->ParticleProp()->StopEmissionAndDestroyImmediately(muzzleFX);
		muzzleFX = NULL;
	}
}



void C_WeaponGluon::DrawSimpleBeam(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4)
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;

	for (int i = 0; i < 4; i++)
	{
		if (!beamFX[i])
		{
			beamFX[i] = player->GetViewModel()->ParticleProp()->Create("gluon_centerbeam", PATTACH_CUSTOMORIGIN);
		}
	}

	beamFX[0]->SetControlPoint(1, p0);
	beamFX[0]->SetControlPoint(2, p1);

	beamFX[1]->SetControlPoint(1, p1);
	beamFX[1]->SetControlPoint(2, p2);

	beamFX[2]->SetControlPoint(1, p2);
	beamFX[2]->SetControlPoint(2, p3);

	beamFX[3]->SetControlPoint(1, p3);
	beamFX[3]->SetControlPoint(2, p4);
}



void C_WeaponGluon::DrawComplexBeam(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3)
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;

	Vector cp1, cp2;

	for (int i = 0; i < BEAM_SEGMENTS; i++)
	{
		cp1 = BezierCurve(p0, p1, p2, p3, (float)i / BEAM_SEGMENTS);
		//NDebugOverlay::Cross3D(cp1, 4.f, 255, 0, 0, false, 0.02f);
		cp2 = BezierCurve(p0, p1, p2, p3, (float)(i + 1) / BEAM_SEGMENTS);
		if (!beamFX[i])
		{
			beamFX[i] = player->GetViewModel()->ParticleProp()->Create("gluon_centerbeam", PATTACH_CUSTOMORIGIN);
		}
		beamFX[i]->SetControlPoint(1, cp1);
		beamFX[i]->SetControlPoint(2, cp2);
	}
}

