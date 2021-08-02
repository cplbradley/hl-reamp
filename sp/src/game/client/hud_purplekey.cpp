#include "hud.h"
#include "cbase.h"
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

	//AW Create Texture for Looking around
	m_nPurpleKey = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nPurpleKey, "HUD/PurpleKey", true, true);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}
void CHudPurpleKey::Init(void)
{
	bShowKey = false;
	HOOK_HUD_MESSAGE(CHudPurpleKey, PurpleKey);
}
void CHudPurpleKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	surface()->DrawSetTexture(m_nPurpleKey);
	surface()->DrawTexturedRect(0, 0, 32, 64);
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

DECLARE_HUDELEMENT(CHudKeyPanel);

CHudKeyPanel::CHudKeyPanel(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudKeyPanel")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudKeyPanel::Init(void)
{
}

void CHudKeyPanel::Paint()
{
	SetPaintBackgroundEnabled(true);
}