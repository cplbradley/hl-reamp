#include "hud.h"
#include "cbase.h"
#include "hud_BlueKey.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"
static ConVar show_blue("show_blue", "0", 0, "toggles beta icon in upper right corner");

using namespace vgui;

DECLARE_HUDELEMENT(CHudBlueKey);

CHudBlueKey::CHudBlueKey(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBlueKey")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	//AW Create Texture for Looking around
	m_nBlueKey = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nBlueKey, "HUD/BlueKey", true, true);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}
void CHudBlueKey::Paint()
{
	SetPaintBorderEnabled(false);
	surface()->DrawSetTexture(m_nBlueKey);
	surface()->DrawTexturedRect(0, 0, 32, 64);
}
void CHudBlueKey::togglePrint()
{
	if (!show_blue.GetBool())
		this->SetVisible(false);
	else
		this->SetVisible(true);
}
void CHudBlueKey::OnThink()
{
	togglePrint();

	BaseClass::OnThink();
}