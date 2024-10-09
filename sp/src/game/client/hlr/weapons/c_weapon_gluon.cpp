#include "cbase.h"
#include "c_baseplayer.h"
#include "c_weapon__stubs.h"
#include "c_basecombatweapon.h"
#include "c_basehlcombatweapon.h"
#include "particle_parse.h"
#include "debugoverlay_shared.h"
#include "soundenvelope.h"
#include "hlr/hlr_shareddefs.h"
#include "iclientmode.h"
#include "beam_shared.h"
#include "hud.h"
#include "hudelement.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "hud_crosshair.h"
#include "VGuiMatSurface\IMatSystemSurface.h"
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include "view_scene.h"
#include "hlr/weapons/c_weapon_gluon.h"


ConVar g_gluon_segments("g_gluon_segments", "16", FCVAR_CHEAT);

string_t sGluon = "weapon_gluon";

#define BEAM_SEGMENTS 32
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
	float uuu = u * u * u;
	float ttt = tt * t;

	Vector p;
	p.x = (uuu)*p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
	p.y = (uuu)*p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
	p.z = (uuu)*p0.z + 3 * uu * t * p1.z + 3 * u * tt * p2.z + ttt * p3.z;

	return p;
}


DECLARE_HUDELEMENT(C_GluonHudDot);

C_GluonHudDot::C_GluonHudDot(const char* ElementName) : CHudElement(ElementName), BaseClass(NULL, "HudGluonDot")
{
	Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(true);
	SetEnabled(true);
	SetActive(true);
	SetPaintBackgroundEnabled(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void C_GluonHudDot::Init()
{
	if (!pDot)
	{
		pDot = gHUD.GetIcon("gluondot");
	}

	SetSize(ScreenWidth(), ScreenHeight());
	bDraw = false;
}

bool C_GluonHudDot::ShouldDraw()
{
	return bDraw;
}

void C_GluonHudDot::OnThink()
{
}
void C_GluonHudDot::Paint()
{
	if (!ShouldDraw())
		return;

	if (!pDot)
	{
		pDot = gHUD.GetIcon("gluondot");
	}

	Vector vecTarget;

	ConVarRef thirdperson("cl_thirdperson_crosshair");

	CBasePlayer* player = CBasePlayer::GetLocalPlayer();

	float crossx, crossy;

	crossx = ScreenWidth() / 2;
	crossy = ScreenHeight() / 2;


	Vector vecEyeDirection, vecEndPos, vecAimPoint;
	player->EyeVectors(&vecEyeDirection);
	VectorMA(player->EyePosition(), MAX_TRACE_LENGTH, vecEyeDirection, vecEndPos);
	trace_t	trace;
	UTIL_TraceLine(player->EyePosition(), vecEndPos, MASK_SHOT, player, COLLISION_GROUP_NONE, &trace);
	vecAimPoint = trace.endpos;


	if (bUseHoverPoint)
	{
		vecTarget = m_vecHoverPoint;
	}
	else
	{
		fLerp += 0.01f;
		fLerp = Clamp(fLerp, 0.f, 1.f);

		float f = easeInOut(fLerp);

		if (f < 1.f)
			InterpolateVector(f, m_vecHoverPoint, vecAimPoint, vecTarget);
		else
			vecTarget = vecAimPoint;
	}

	Vector vecPos = Vector(0, 0, 0);

	HudTransform(vecTarget, vecPos);

	crossx += 0.5 * vecPos[0] * ScreenWidth() + 0.5;
	crossy -= 0.5 * vecPos[1] * ScreenHeight() + 0.5;

	crossx -= pDot->Width() / 2;
	crossy -= pDot->Height() / 2;

	pDot->DrawSelf(crossx, crossy, gHUD.GetDefaultColor());

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
	bool m_bOverdrive;
};

IMPLEMENT_CLIENTCLASS_DT(C_GluonTarget,DT_GluonTarget,CGluonTarget)
RecvPropVector(RECVINFO(m_vecDestination)),
RecvPropBool(RECVINFO(m_bOverdrive)),
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
	float speedmod = 5.f;
	if (m_bOverdrive)
		speedmod = 30.f;
	SetAbsVelocity(GetAbsOrigin() + (vecDir * fLength * speedmod));
}


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


	bool m_bDrawCross;

private:

	CNewParticleEffect* beamFX[BEAM_SEGMENTS];
	CNewParticleEffect* muzzleFX;

	bool m_bFiring;
	Vector m_vecDamagePoint;

	CHandle<C_GluonTarget> pTarget;

	float fNextTick;
	float fPrevTick;
	float flInterval;

	bool m_bOverdrive;


	CSoundPatch* m_sBaseLayer;
	CSoundPatch* m_sMidLayer;
	Vector vecnext;

	C_GluonHudDot* pDot;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_gluon,C_WeaponGluon)
IMPLEMENT_CLIENTCLASS_DT(C_WeaponGluon,DT_WeaponGluon,CWeaponGluon)
RecvPropBool(RECVINFO(m_bFiring)),
RecvPropVector(RECVINFO(m_vecDamagePoint)),
RecvPropEHandle(RECVINFO(pTarget)),
RecvPropBool(RECVINFO(m_bOverdrive)),
RecvPropBool(RECVINFO(m_bDrawCross)),
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
		PrecacheParticleSystem("gluon_muzzle_overdrive");
		PrecacheParticleSystem("gluon_centerbeam_overdrive");
		PrecacheScriptSound("Gluon.BaseLayer");
		PrecacheScriptSound("Gluon.MidLayer");
		PrecacheScriptSound("Gluon.ODLayer");
		
	}

	BaseClass::OnDataChanged(type);
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

	if(!pDot)
		pDot = (C_GluonHudDot*)GET_HUDELEMENT(C_GluonHudDot);
	if (pDot)
	{
		pDot->bDraw = m_bDrawCross;
	}

	BaseClass::ClientThink();
}

void C_WeaponGluon::StartFX()
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;

	Vector vecStart, vecEnd, vecQuarter, vecThreeQuarter;
	
	player->GetViewModel()->GetAttachment(LookupAttachment("muzzle"), vecStart);

	ConVarRef thirdperson("g_thirdperson");

	if (thirdperson.GetBool())
		GetAttachment(LookupAttachment("muzzle"), vecStart);

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
		if (thirdperson.GetBool())
		{
			if (m_bOverdrive)
				muzzleFX = ParticleProp()->Create("gluon_muzzle_overdrive", PATTACH_POINT_FOLLOW, "muzzle");
			else
				muzzleFX = ParticleProp()->Create("gluon_muzzle", PATTACH_POINT_FOLLOW, "muzzle");
		}
		else
		{
			if (m_bOverdrive)
				muzzleFX = player->GetViewModel()->ParticleProp()->Create("gluon_muzzle_overdrive", PATTACH_POINT_FOLLOW, "muzzle");
			else
				muzzleFX = player->GetViewModel()->ParticleProp()->Create("gluon_muzzle", PATTACH_POINT_FOLLOW, "muzzle");
		}
	}

	CPASAttenuationFilter filter(this);
	if (!m_sBaseLayer)
	{
		if(!m_bOverdrive)
			m_sBaseLayer = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Gluon.BaseLayer");
		else
			m_sBaseLayer = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "Gluon.ODLayer");
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


	if (pDot)
	{
		pDot->bUseHoverPoint = true;
		pDot->m_vecHoverPoint = vecEnd;
	}
}

void C_WeaponGluon::StopFX()
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;
	ConVarRef thirdperson("g_thirdperson");

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

	for (int i = 0; i < g_gluon_segments.GetInt(); i++)
	{
		if (beamFX[i])
		{
			thirdperson.GetBool() ? ParticleProp()->StopEmissionAndDestroyImmediately(beamFX[i]) : player->GetViewModel()->ParticleProp()->StopEmissionAndDestroyImmediately(beamFX[i]);
			beamFX[i] = NULL;
		}
	}

	if (muzzleFX)
	{
		thirdperson.GetBool() ? ParticleProp()->StopEmissionAndDestroyImmediately(muzzleFX) : player->GetViewModel()->ParticleProp()->StopEmissionAndDestroyImmediately(muzzleFX);
		muzzleFX = NULL;
	}

	if (pDot && pDot->bUseHoverPoint == true)
	{
		pDot->fLerp = 0.f;
		pDot->bUseHoverPoint = false;
	}
}



void C_WeaponGluon::DrawSimpleBeam(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4)
{
	C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
	if (!player)
		return;

	ConVarRef thirdperson("g_thirdperson");

	for (int i = 0; i < 4; i++)
	{
		if (!beamFX[i])
		{
			const char* beam = m_bOverdrive ? "gluon_centerbeam_overdrive" : "gluon_centerbeam";
			DevMsg("creating gluon particle %\n", beam);
			thirdperson.GetBool() ? beamFX[i] = ParticleProp()->Create(beam, PATTACH_CUSTOMORIGIN) : beamFX[i] = player->GetViewModel()->ParticleProp()->Create(beam, PATTACH_CUSTOMORIGIN);
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

	ConVarRef thirdperson("g_thirdperson");

	for (int i = 0; i < g_gluon_segments.GetInt(); i++)
	{
		cp1 = BezierCurve(p0, p1, p2, p3, (float)i / g_gluon_segments.GetInt());
		//NDebugOverlay::Cross3D(cp1, 4.f, 255, 0, 0, false, 0.02f);
		cp2 = BezierCurve(p0, p1, p2, p3, (float)(i + 1) / g_gluon_segments.GetInt());
		if (!beamFX[i])
		{
			const char* beam = m_bOverdrive ? "gluon_centerbeam_overdrive" : "gluon_centerbeam";
			DevMsg("creating particle %\n", beam);
			thirdperson.GetBool() ? beamFX[i] = ParticleProp()->Create(beam, PATTACH_CUSTOMORIGIN) : beamFX[i] = player->GetViewModel()->ParticleProp()->Create(beam, PATTACH_CUSTOMORIGIN);
		}
		beamFX[i]->SetControlPoint(1, cp1);
		beamFX[i]->SetControlPoint(2, cp2);
	}
}


