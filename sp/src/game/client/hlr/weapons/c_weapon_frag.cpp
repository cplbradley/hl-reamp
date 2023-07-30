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

using namespace vgui;


class C_HudFragCharge : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(C_HudFragCharge, Panel);
public:
	C_HudFragCharge(const char* pElementName);
	virtual bool ShouldDraw() { return true; }
	virtual void Paint();
	virtual void Init();
	virtual void ApplySettings(KeyValues* inResourceData);
	bool bActive;
	float fMult;
	
private:
	int iBaseX;
	int iBaseY;
	
	CPanelAnimationVarAliasType(float, fWide, "barwidth", "2", "proportional_float");
};
DECLARE_HUDELEMENT(C_HudFragCharge);
C_HudFragCharge::C_HudFragCharge(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudFragCharge")
{
	Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(true);
	SetEnabled(true);
	SetActive(true);
	SetPaintBackgroundEnabled(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void C_HudFragCharge::ApplySettings(KeyValues* inResourceData)
{
	BaseClass::ApplySettings(inResourceData);
	GetPos(iBaseX, iBaseY);
}

void C_HudFragCharge::Init()
{
	fMult = 0.0f;
	bActive = false;
}
void C_HudFragCharge::Paint()
{
	float prog = fMult / 2.0f;
	gHUD.DrawProgressBar(0, 0, fWide, GetTall(), prog, gHUD.GetDefaultColor(), CHud::HUDPB_VERTICAL_NOBG);

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
	SetPos(iBaseX + screen[0] * screenX, iBaseY - screen[1] * screenY);
}


class C_WeaponFrag : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponFrag, C_BaseHLCombatWeapon);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
public:
	C_WeaponFrag();
	void UpdateHud();
	virtual void	OnDataChanged(DataUpdateType_t updateType);
private:
	float m_fChargedMultiplier;
	bool m_bIsCharging;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_frag, C_WeaponFrag);
IMPLEMENT_CLIENTCLASS_DT(C_WeaponFrag, DT_WeaponFrag, CWeaponFrag)
RecvPropFloat(RECVINFO(m_fChargedMultiplier)),
RecvPropBool(RECVINFO(m_bIsCharging)),
END_RECV_TABLE()

C_WeaponFrag::C_WeaponFrag()
{
	m_fChargedMultiplier = 0.0f;
	m_bIsCharging = false;
}

void C_WeaponFrag::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (updateType == DATA_UPDATE_DATATABLE_CHANGED)
		UpdateHud();
}

void C_WeaponFrag::UpdateHud()
{
	C_HudFragCharge* pHud = (C_HudFragCharge*)GET_HUDELEMENT(C_HudFragCharge);
	if (!pHud)
		return;
	pHud->fMult = m_fChargedMultiplier;
	//pHud->bActive = m_bIsCharging;
}


