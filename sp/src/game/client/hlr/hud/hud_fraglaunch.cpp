#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h" 
#include "iclientmode.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "hud_crosshair.h"
#include "VGuiMatSurface\IMatSystemSurface.h"
#include <vgui_controls/AnimationController.h>
#include "c_baseplayer.h"

#include "tier0/memdbgon.h"

using namespace vgui;
class CHudFragLaunch : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudFragLaunch, Panel);
public:
	CHudFragLaunch(const char* pElementName);
	virtual bool ShouldDraw();
	virtual void Paint();
	virtual void OnThink();
	void Init();
	virtual void ApplySettings(KeyValues* inResourceData);

	void MsgFunc_FragLaunch(bf_read& msg);

	CHudTexture* pIcon;
private:
	bool bShouldDraw;
	CPanelAnimationVar(vgui::HFont, m_hFont, "TextFont", "WeaponIconsSmall");

};

DECLARE_HUDELEMENT(CHudFragLaunch);
DECLARE_HUD_MESSAGE(CHudFragLaunch, FragLaunch);


CHudFragLaunch::CHudFragLaunch(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "FragLaunch")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetEnabled(true);
	SetPaintBackgroundEnabled(false);
	Init();
	SetHiddenBits(HIDEHUD_NEEDSUIT | HIDEHUD_PLAYERDEAD | HIDEHUD_WEAPON_WHEEL | HIDEHUD_WEAPONSELECTION | HIDEHUD_CINEMATIC_CAMERA);
}
void CHudFragLaunch::ApplySettings(KeyValues* inResourceData)
{
	BaseClass::ApplySettings(inResourceData);
	SetAlpha(0);
}
void CHudFragLaunch::Init()
{
	HOOK_HUD_MESSAGE(CHudFragLaunch, FragLaunch);
	pIcon = gHUD.GetIcon("frag_launch");
	SetAlpha(0);
}

void CHudFragLaunch::OnThink()
{
	CBasePlayer* player = CBasePlayer::GetLocalPlayer();

	if (!player)
		return;

	if (!player->IsAlive())
		SetAlpha(0);

}
bool CHudFragLaunch::ShouldDraw()
{
	if (!GetAlpha())
		return false;
	return true;
}
void CHudFragLaunch::MsgFunc_FragLaunch(bf_read& msg)
{
	bShouldDraw = msg.ReadOneBit();

	if(bShouldDraw)
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowFragLaunch");
	else
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HideFragLaunch");
}

void CHudFragLaunch::Paint()
{
	if(!pIcon)
		pIcon = gHUD.GetIcon("frag_launch");
	int screenCenterX = ScreenWidth() / 2;
	int screenCenterY = ScreenHeight() / 2;
	SetSize(pIcon->Width(), pIcon->Height());
	SetPos(screenCenterX - GetWide() / 2, screenCenterY + GetTall() / 2);
	pIcon->DrawSelf(0, 0, gHUD.GetDefaultColor());
}

