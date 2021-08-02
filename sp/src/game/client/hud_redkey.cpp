#include "hud.h"
#include "cbase.h"
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

	//AW Create Texture for Looking around
	m_nRedKey = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nRedKey, "HUD/RedKey", true, true);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}
void CHudRedKey::Init(void)
{
	HOOK_HUD_MESSAGE(CHudRedKey, RedKey);
	bShowKey = 0;
}
void CHudRedKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	surface()->DrawSetTexture(m_nRedKey);
	surface()->DrawTexturedRect(0, 0, 32, 64);
}
void CHudRedKey::togglePrint()
{
	if (!bShowKey)
		this->SetVisible(false);
	else
		this->SetVisible(true);
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