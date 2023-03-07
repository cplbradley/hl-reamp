#include "cbase.h"
#include "hud.h"
#include "hud_BlueKey.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"
static ConVar show_blue("show_blue", "0", 0, "toggles beta icon in upper right corner");

using namespace vgui;

DECLARE_HUDELEMENT(CHudBlueKey);
DECLARE_HUD_MESSAGE(CHudBlueKey, BlueKey);

CHudBlueKey::CHudBlueKey(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBlueKey")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	//AW Create Texture for Looking around
	/*m_nBlueKey = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nBlueKey, "HUD/BlueKey", true, true);*/

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
}
void CHudBlueKey::VidInit(void)
{
	Init();
}
void CHudBlueKey::Init(void)
{
	bShowKey = false;
	HOOK_HUD_MESSAGE(CHudBlueKey, BlueKey);
	m_icon = gHUD.GetIcon("keycard_icon");
}
void CHudBlueKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	/*surface()->DrawSetTexture(m_nBlueKey);
	surface()->DrawTexturedRect(0, 0, 32, 64);*/
	Color clrblue = Color(0, 180, 255, 200);
	m_icon->DrawSelf(0, -16, clrblue);
}
void CHudBlueKey::MsgFunc_BlueKey(bf_read &msg)
{
	bShowKey = msg.ReadByte();
}
void CHudBlueKey::togglePrint()
{
	if (!bShowKey)
		this->SetVisible(false);
	else
		this->SetVisible(true);
}
void CHudBlueKey::OnThink()
{
	togglePrint();

	BaseClass::OnThink();
}
void CHudBlueKey::LevelInit(void)
{
	bShowKey = false;
}