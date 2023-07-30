#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "hud_crosshair.h"
#include "VGuiMatSurface\IMatSystemSurface.h"
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include "hud_macros.h"
#include "c_baseplayer.h"
#include "view_scene.h"

#include "memdbgon.h"

using namespace vgui;


class CHudChaingun : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudChaingun, Panel);
public:
	CHudChaingun(const char* pElementName);
	virtual void ApplySettings(KeyValues* inResourceData);
	void OnThink();
	bool ShouldDraw();
	void Paint();
	void VidInit();
	void Init();
	void MsgFunc_HudChaingun(bf_read& msg);
private:
	CPanelAnimationVarAliasType(float, flBaseRadius, "baseradius", "16", "proportional_float");
	int xpos;
	int ypos;

	bool bActive;
	int RadiusScalar;
};

DECLARE_HUDELEMENT(CHudChaingun);
DECLARE_HUD_MESSAGE(CHudChaingun, HudChaingun);


CHudChaingun::CHudChaingun(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudChaingun")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetEnabled(true);
	SetActive(true);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
	SetPaintBackgroundEnabled(false);
}

void CHudChaingun::VidInit()
{
	Init();
}
void CHudChaingun::Init()
{
	HOOK_HUD_MESSAGE(CHudChaingun, HudChaingun);
	bActive = false;
}
void CHudChaingun::ApplySettings(KeyValues* inResourceData)
{
	BaseClass::ApplySettings(inResourceData);
	GetPos(xpos, ypos);
}

void CHudChaingun::MsgFunc_HudChaingun(bf_read& msg)
{
	bActive = msg.ReadOneBit();
	RadiusScalar = msg.ReadByte();
}

bool CHudChaingun::ShouldDraw()
{
	if (CBasePlayer::GetLocalPlayer() && !CBasePlayer::GetLocalPlayer()->IsAlive())
		return false;
	return bActive;
}
void CHudChaingun::OnThink()
{
	BaseClass::OnThink();
}
void CHudChaingun::Paint()
{
	if (!ShouldDraw())
		return;

	Color crosscolor = gHUD.GetDefaultColor();
	int x = GetWide() * 0.5f;
	int y = GetTall() * 0.5f;

	float scalar = 1.0f - (RadiusScalar / 50.0f) * 0.6f;

	CBasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();

	ConVarRef thirdperson("cl_thirdperson_crosshair");
	Vector screen = Vector(0, 0, 0);
	float screenX = ScreenWidth() * 0.5;
	float screeny = ScreenHeight() * 0.5;

	if (thirdperson.GetBool())
	{
		Vector vecEyeDirection, vecEndPos, vecAimPoint;
		pPlayer->EyeVectors(&vecEyeDirection);
		VectorMA(pPlayer->EyePosition(), MAX_TRACE_LENGTH, vecEyeDirection, vecEndPos);
		trace_t	trace;
		UTIL_TraceLine(pPlayer->EyePosition(), vecEndPos, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &trace);
		vecAimPoint = trace.endpos;
		HudTransform(vecAimPoint, screen);
		//engine->Con_NPrintf(1, "Transform X: %f Transform Y: %f", screen[0] * screenX, screen[1] * screeny);
	}

	surface()->DrawSetColor(crosscolor);
	surface()->DrawOutlinedCircle(x, y, (GetWide()/2) * scalar, 128);
	SetPos(xpos + screen[0] * screenX, ypos - screen[1] * screeny);
}