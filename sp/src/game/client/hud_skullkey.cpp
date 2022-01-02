#include "hud.h"
#include "cbase.h"
#include "hud_skullkey.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"
//static ConVar show_red("show_Red", "0", 0, "toggles beta icon in upper right corner");

using namespace vgui;

DECLARE_HUDELEMENT(CHudRedSkullKey);
DECLARE_HUDELEMENT(CHudBlueSkullKey);
DECLARE_HUDELEMENT(CHudPurpleSkullKey);

DECLARE_HUD_MESSAGE(CHudRedSkullKey, RedSkullKey);
DECLARE_HUD_MESSAGE(CHudBlueSkullKey, BlueSkullKey);
DECLARE_HUD_MESSAGE(CHudPurpleSkullKey, PurpleSkullKey);


CHudRedSkullKey::CHudRedSkullKey(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudRedSkullKey")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	//AW Create Texture for Looking around

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}
void CHudRedSkullKey::Init(void)
{
	HOOK_HUD_MESSAGE(CHudRedSkullKey, RedSkullKey);
	bShowKey = 0;
	icon_skullkey = gHUD.GetIcon("skullkey_icon");
}
void CHudRedSkullKey::VidInit()
{
	Init();
}
void CHudRedSkullKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	Color red = Color(230, 75, 0, 200);
	icon_skullkey->DrawSelf(0, -8, red);
}
void CHudRedSkullKey::togglePrint()
{
	if (!bShowKey)
		this->SetVisible(false);
	else
		this->SetVisible(true);
}
void CHudRedSkullKey::MsgFunc_RedSkullKey(bf_read &msg)
{
	bShowKey = msg.ReadByte();
}
void CHudRedSkullKey::OnThink()
{
	togglePrint();

	BaseClass::OnThink();
}
void CHudRedSkullKey::LevelInit(void)
{
	bShowKey = false;
}


CHudBlueSkullKey::CHudBlueSkullKey(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBlueSkullKey")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	//AW Create Texture for Looking around

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}
void CHudBlueSkullKey::Init(void)
{
	HOOK_HUD_MESSAGE(CHudBlueSkullKey, BlueSkullKey);
	bShowKey = 0;
	icon_skullkey = gHUD.GetIcon("skullkey_icon");
}
void CHudBlueSkullKey::VidInit()
{
	Init();
}
void CHudBlueSkullKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	Color blue = Color(0, 180, 255, 200);
	icon_skullkey->DrawSelf(0, -8, blue);
}
void CHudBlueSkullKey::togglePrint()
{
	if (!bShowKey)
		this->SetVisible(false);
	else
		this->SetVisible(true);
}
void CHudBlueSkullKey::MsgFunc_BlueSkullKey(bf_read &msg)
{
	bShowKey = msg.ReadByte();
}
void CHudBlueSkullKey::OnThink()
{
	togglePrint();

	BaseClass::OnThink();
}
void CHudBlueSkullKey::LevelInit(void)
{
	bShowKey = false;
}

CHudPurpleSkullKey::CHudPurpleSkullKey(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudPurpleSkullKey")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	//AW Create Texture for Looking around

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}
void CHudPurpleSkullKey::Init(void)
{
	HOOK_HUD_MESSAGE(CHudPurpleSkullKey, PurpleSkullKey);
	bShowKey = 0;
	icon_skullkey = gHUD.GetIcon("skullkey_icon");
}
void CHudPurpleSkullKey::VidInit()
{
	Init();
}
void CHudPurpleSkullKey::Paint()
{
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	Color purp = Color(75, 0, 100, 200);
	icon_skullkey->DrawSelf(0, -8, purp);
}
void CHudPurpleSkullKey::togglePrint()
{
	if (!bShowKey)
		this->SetVisible(false);
	else
		this->SetVisible(true);
}
void CHudPurpleSkullKey::MsgFunc_PurpleSkullKey(bf_read &msg)
{
	bShowKey = msg.ReadByte();
}
void CHudPurpleSkullKey::OnThink()
{
	togglePrint();

	BaseClass::OnThink();
}
void CHudPurpleSkullKey::LevelInit(void)
{
	bShowKey = false;
}