#include "cbase.h"
#include "hud.h"
#include "hud_purplekey.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"
//static ConVar show_blue("show_blue", "0", 0, "toggles beta icon in upper right corner");

using namespace vgui;

DECLARE_HUDELEMENT(CHudPurpleKey);
DECLARE_HUD_MESSAGE(CHudPurpleKey, PurpleKey);

CHudPurpleKey::CHudPurpleKey(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudPurpleKey")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
}
void CHudPurpleKey::Init(void)
{
	bShowKey = false;
	HOOK_HUD_MESSAGE(CHudPurpleKey, PurpleKey);
	m_icon = gHUD.GetIcon("keycard_icon");
}
void CHudPurpleKey::VidInit(void)
{
	Init();
}
void CHudPurpleKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	Color purp = Color(75, 0, 100, 200);
	m_icon->DrawSelf(0, -16, purp);
}
void CHudPurpleKey::MsgFunc_PurpleKey(bf_read &msg)
{
	bShowKey = msg.ReadByte();
}
void CHudPurpleKey::togglePrint()
{
	if (!bShowKey)
		this->SetVisible(false);
	else
		this->SetVisible(true);
}
void CHudPurpleKey::OnThink()
{
	togglePrint();

	BaseClass::OnThink();
}
void CHudPurpleKey::LevelInit(void)
{
	bShowKey = false;
}

DECLARE_HUDELEMENT_DEPTH(CHudKeyPanel,70);

CHudKeyPanel::CHudKeyPanel(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudKeyPanel")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	//SetVisible(false);
	//SetAlpha(255);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);

}

void CHudKeyPanel::Init(void)
{
	
}

void CHudKeyPanel::Paint()
{
	m_background = gHUD.GetIcon("hud_key_background");
	

	m_background->DrawSelf(0, 0, GetWide(), GetTall(), Color(255, 255, 255, 255));
	SetPaintBackgroundEnabled(false);

}