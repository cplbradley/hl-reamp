#include "cbase.h"
#include "hud.h"
#include "hud_RedKey.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

static ConVar show_red("show_Red", "0", 0, "toggles beta icon in upper right corner");

using namespace vgui;

DECLARE_HUD_MESSAGE(CHudRedKey, RedKey);

CHudRedKey::CHudRedKey(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudRedKey")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);


	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
}
void CHudRedKey::VidInit(void)
{
	Init();
}
void CHudRedKey::Init(void)
{
	HOOK_HUD_MESSAGE(CHudRedKey, RedKey);
	m_nRedKey = gHUD.GetIcon("keycard_icon");
	bShowKey = 0;
}
void CHudRedKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	Color red = Color(230, 75, 0, 200);
	m_nRedKey->DrawSelf(0, -16, red);
	
}
void CHudRedKey::togglePrint()
{
	if (!bShowKey)
		SetVisible(false);
	else
		SetVisible(true);
}
void CHudRedKey::MsgFunc_RedKey(bf_read &msg)
{
	bShowKey = msg.ReadByte();
}
void CHudRedKey::OnThink()
{
	togglePrint();

	BaseClass::OnThink();
}
void CHudRedKey::LevelInit(void)
{
	bShowKey = false;
}