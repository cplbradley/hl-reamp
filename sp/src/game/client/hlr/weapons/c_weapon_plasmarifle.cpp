#include "cbase.h"
#include "c_weapon__stubs.h"
#include "model_types.h"
#include "iclientmode.h"
#include "clienteffectprecachesystem.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"
#include "particle_parse.h"
#include "c_te_particlesystem.h"
#include "c_basecombatweapon.h"
#include "c_basehlcombatweapon.h"
#include "particles_simple.h"
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
#include "KeyValues.h"


#include "tier0/memdbgon.h"

class C_HudPlasmaRifleAltProgress : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(C_HudPlasmaRifleAltProgress, vgui::Panel);
public:
	C_HudPlasmaRifleAltProgress(const char* pElementName);
	virtual void ApplySettings(KeyValues* inResourceData);
	void Paint();
	bool ShouldDraw();
	void Init();
	void VidInit();

	float m_fProgress;
	bool bActive;
private:
	CHudTexture* pIcon;
	CPanelAnimationVarAliasType(float, fWide, "wide", "128", "proportional_float");
	CPanelAnimationVarAliasType(float, fTall, "tall", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, iconWide, "iconWide", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, iconTall, "iconTall", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, fBarTall, "barheight", "2", "proportional_float");

	int xpos, ypos;
};

DECLARE_HUDELEMENT(C_HudPlasmaRifleAltProgress);


C_HudPlasmaRifleAltProgress::C_HudPlasmaRifleAltProgress(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudPlasmaAltBar")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetEnabled(true);
	SetActive(true);
	SetPaintBackgroundEnabled(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void C_HudPlasmaRifleAltProgress::VidInit()
{
	Init();
}
void C_HudPlasmaRifleAltProgress::Init()
{
	m_fProgress = 0.0f;
	bActive = false;
	pIcon = gHUD.GetIcon("overheat_icon");
}
void C_HudPlasmaRifleAltProgress::ApplySettings(KeyValues* inResourceData)
{
	BaseClass::ApplySettings(inResourceData);
	GetPos(xpos, ypos);
}
bool C_HudPlasmaRifleAltProgress::ShouldDraw()
{
	if (CBasePlayer::GetLocalPlayer() && !CBasePlayer::GetLocalPlayer()->IsAlive())
		return false;

	return bActive;
}

void C_HudPlasmaRifleAltProgress::Paint()
{
	Vector vecOutColor, vecRed, vecStartColor;
	Color defaultcolor = gHUD.GetDefaultColor();

	vecStartColor = Vector(defaultcolor.r(), defaultcolor.g(), defaultcolor.b());
	vecRed = Vector(255, 0, 0);

	InterpolateVector(m_fProgress, vecStartColor, vecRed, vecOutColor);
	Color outColor = Color(vecOutColor[0], vecOutColor[1], vecOutColor[2], 200);
	float alpha;

	ConVarRef thirdperson("cl_thirdperson_crosshair");
	Vector screen = Vector(0, 0, 0);
	float screenX = ScreenWidth() * 0.5f;
	float screenY = ScreenHeight() * 0.5f;

	if (thirdperson.GetBool())
	{
		CBasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
		Vector vecEyeDirection, vecEndPos, vecAimPoint;
		pPlayer->EyeVectors(&vecEyeDirection);
		VectorMA(pPlayer->EyePosition(), MAX_TRACE_LENGTH, vecEyeDirection, vecEndPos);
		trace_t	trace;
		UTIL_TraceLine(pPlayer->EyePosition(), vecEndPos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &trace);
		vecAimPoint = trace.endpos;
		
		HudTransform(vecAimPoint, screen);
		
		
		//engine->Con_NPrintf(1, "Transform X: %f Transform Y: %f", screen[0] * screenX, screen[1] * screeny);
	}
	

	gHUD.DrawProgressBar(0, 0, GetWide(), fBarTall, m_fProgress, outColor, CHud::HUDPB_HORIZONTAL_INV_NOBG);

	if (m_fProgress >= 1.0f)
		alpha = ((sin(gpGlobals->curtime * 15) * 100) + 100);
	else
		alpha = 0.0f;
	pIcon = gHUD.GetIcon("overheat_icon");

	Color iconColor = Color(255, 0, 0, alpha);
	pIcon->DrawSelf(GetWide() * 0.5f - pIcon->Width() * 0.5f, 0, iconWide, iconTall, iconColor);
	SetPos(xpos + screen[0] * screenX, ypos - screen[1] * screenY);
}

class C_WeaponPlasmaRifle : public C_HLSelectFireMachineGun
{
	DECLARE_CLASS(C_WeaponPlasmaRifle, C_HLSelectFireMachineGun);
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	C_WeaponPlasmaRifle();
	void ClientThink();
	int DrawModel(int flags);
	virtual void	OnDataChanged(DataUpdateType_t updateType);
	bool bActive;
	int iShots;
	//bool bShootingSecondary;
	//Vector vecBeamEnd;
private:
	//CNewParticleEffect* beamFX;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_plasmarifle, C_WeaponPlasmaRifle);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponPlasmaRifle, DT_WeaponPlasmaRifle, CWeaponPlasmaRifle)
RecvPropBool(RECVINFO(bActive)),
RecvPropInt(RECVINFO(iShots)),
//RecvPropVector(RECVINFO(vecBeamEnd)),
END_RECV_TABLE()

C_WeaponPlasmaRifle::C_WeaponPlasmaRifle()
{
	bActive = false;
	iShots = 0;
}

void C_WeaponPlasmaRifle::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

void C_WeaponPlasmaRifle::ClientThink()
{
	BaseClass::ClientThink();

	C_HudPlasmaRifleAltProgress* hudbar = GET_HUDELEMENT(C_HudPlasmaRifleAltProgress);

	if (!hudbar)
		return;

	hudbar->bActive = bActive;

	float numShots = MIN(iShots, 30);
	float fProgress = numShots / 30;
	hudbar->m_fProgress = fProgress;

	/*C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
	C_BaseAnimating* pAnimating = pPlayer->GetViewModel();
	if (!pAnimating)
		return;
	int attachment = pAnimating->LookupAttachment("barrel_vm");

	Vector vecStart, vecDir;
	QAngle angAttach;


	GetAttachment(attachment, vecStart, angAttach);
	QAngle angDir = pPlayer->EyeAngles();
	AngleVectors(angDir, &vecDir);
	VectorNormalize(vecDir);

	trace_t tr;
	UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + (vecDir * MAX_TRACE_LENGTH), MASK_SHOT, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr);
	Vector vecEnd;
	InterpolateVector(0.5f, vecBeamEnd, tr.endpos, vecEnd);
	CParticleProperty* pProp = ParticleProp();
	Vector	vForward, vRight, vUp;

	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	if (bShootingSecondary)
	{
		DevMsg("Firing Secondary\n");
		

		if (!beamFX)
		{
			beamFX = pProp->Create("plasmarifle_altfire_beam", PATTACH_CUSTOMORIGIN, 0, pPlayer->EyePosition() + vForward * 12.0f + vRight * 2.0f + vUp * -3.0f);
		}
		else
		{
			beamFX->SetControlPoint(0, pPlayer->EyePosition() + vForward * 12.0f + vRight * 2.0f + vUp * -3.0f);
			beamFX->SetControlPoint(1, tr.endpos);
			//beamFX->StartEmission();
		}
	}
	else
	{
		if (beamFX)
		{
			beamFX->StopEmission();
			pProp->StopEmissionAndDestroyImmediately(beamFX);
			beamFX = NULL;
		}
	}*/

}

int C_WeaponPlasmaRifle::DrawModel(int flags)
{
	return BaseClass::DrawModel(flags);
}

