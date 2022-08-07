#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_baseplayer.h"
#include "iclientmode.h"
#include "hl2_shareddefs.h"
#include "hl2_gamerules.h"
#include "engine/IEngineSound.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "hud_environmental_resistance.h"

#include "tier0/memdbgon.h"





DECLARE_HUDELEMENT(CHudEnvironmentResistToxic);

CHudEnvironmentResistToxic::CHudEnvironmentResistToxic(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudEnvironmentResistToxic")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudEnvironmentResistToxic::Init(void)
{
	m_iDamageLeft = 0;
	m_iMaxPoints = 50;
	bShowIcon = false;
	m_icon1 = gHUD.GetIcon("icon_env_toxic");
	m_icon2 = gHUD.GetIcon("icon_env_toxic_glow");
	m_empty = gHUD.GetIcon("empty");
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	alpha = 0;


}
void CHudEnvironmentResistToxic::InitAnimation(void)
{
}
void CHudEnvironmentResistToxic::OnThink(void)
{
	if (!bShowIcon)
		return;
	TogglePrint();
	//float perc = (float)m_iTimeLeft / 30.0f;
	//DevMsg("hud element timeleft = %f", perc);
}

void CHudEnvironmentResistToxic::VidInit(void)
{
	Init();
}
void CHudEnvironmentResistToxic::Paint()
{
	if (!bShowIcon)
		return;
	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
	if (m_iDamageLeft != pPlayer->m_iToxicDamageLeft)
		alpha = 200;
	alpha--;
	if (alpha < 0)
		alpha = 0;

	m_iMaxPoints = pPlayer->m_iMaxToxicDamage;
	m_iDamageLeft = pPlayer->m_iToxicDamageLeft;

	Color clrToxic = Color(115, 160, 0, 200);
	Color clrToxicBG = Color(60, 80, 0, 64);
	Color clrToxicGlow = Color(115, 160, 0, alpha);

	int max = (float)m_iMaxPoints;
	float val = (float)m_iDamageLeft;

	float perc = val / max; //we do it out of 29 to give a second of 0% 
	DevMsg("hud damage is %f\n", val);
	DevMsg("hud max is %i\n", max);
	gHUD.DrawIconProgressBar(m_IconX, m_IconY, m_icon1, m_empty, 1.0 - perc, clrToxic, CHud::HUDPB_VERTICAL);
	m_icon1->DrawSelf(m_IconX, m_IconY, clrToxicBG);
	m_icon2->DrawSelf(m_IconX, m_IconY + 4, clrToxicGlow);
}
void CHudEnvironmentResistToxic::TogglePrint(void)
{
	if (bShowIcon && !IsVisible())
		SetVisible(true);
	if (!bShowIcon && IsVisible())
	{
		SetVisible(false);
	}
}
void CHudEnvironmentResistToxic::LevelInit(void)
{
	SetVisible(false);
}

DECLARE_HUDELEMENT(CHudEnvironmentResistFire);

CHudEnvironmentResistFire::CHudEnvironmentResistFire(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudEnvironmentResistFire")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudEnvironmentResistFire::Init(void)
{
	m_iDamageLeft = 0;
	bShowIcon = false;
	m_icon1 = gHUD.GetIcon("icon_env_fire");
	m_icon2 = gHUD.GetIcon("icon_env_fire_glow");
	m_empty = gHUD.GetIcon("empty");
	alpha = 0;
	clrBurn = Color(255, 120, 0, 200);
	clrBurnBG = Color(120, 50, 0, 64);
	clrBurnGlow = Color(255, 120, 0, 0);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
}

void CHudEnvironmentResistFire::OnThink(void)
{
	TogglePrint();
	
	//float perc = (float)m_iTimeLeft / 30.0f;
	//DevMsg("hud element timeleft = %f", perc);
}

void CHudEnvironmentResistFire::VidInit(void)
{
	Init();
}
void CHudEnvironmentResistFire::Paint()
{
	if (!bShowIcon)
		return;
	if (alpha < 0)
		alpha = 0;
	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
	if (m_iDamageLeft != pPlayer->m_iFireDamageLeft)
		alpha = 200;
	clrBurnGlow = Color(255, 120, 0, alpha);
	alpha--;
	m_iDamageLeft = pPlayer->m_iFireDamageLeft;
	m_iMaxPoints = pPlayer->m_iMaxFireDamage;



	int max = (float)m_iMaxPoints;
	float val = (float)m_iDamageLeft;

	float perc = val / max; //we do it out of 29 to give a second of 0% 
	gHUD.DrawIconProgressBar(m_IconX, m_IconY, m_icon1, m_empty, 1.0 - perc, clrBurn, CHud::HUDPB_VERTICAL);
	m_icon1->DrawSelf(m_IconX, m_IconY, clrBurnBG);
	m_icon2->DrawSelf(m_IconX, m_IconY + 4, clrBurnGlow);
}
void CHudEnvironmentResistFire::TogglePrint(void)
{
	if (bShowIcon && !IsVisible())
		SetVisible(true);
	if (!bShowIcon && IsVisible())
	{
		SetVisible(false);
	}
}
void CHudEnvironmentResistFire::LevelInit(void)
{
	SetVisible(false);
}


DECLARE_HUDELEMENT(CHudEnvironmentResistElectric);

CHudEnvironmentResistElectric::CHudEnvironmentResistElectric(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudEnvironmentResistElectric")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudEnvironmentResistElectric::Init(void)
{
	m_iDamageLeft = 0;
	bShowIcon = false;
	m_icon1 = gHUD.GetIcon("icon_env_electric");
	m_icon2 = gHUD.GetIcon("icon_env_electric_glow");
	m_empty = gHUD.GetIcon("empty");
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	alpha = 0;
}

void CHudEnvironmentResistElectric::OnThink(void)
{
	TogglePrint();
	//float perc = (float)m_iTimeLeft / 30.0f;
	//DevMsg("hud element timeleft = %f", perc);
}

void CHudEnvironmentResistElectric::VidInit(void)
{
	Init();
}
void CHudEnvironmentResistElectric::Paint()
{
	if (!bShowIcon)
		return;

	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
	Color clrShock = Color(50, 175, 255, 200);
	Color clrShockBG = Color(0, 40, 120, 64);
	

	if (m_iDamageLeft != pPlayer->m_iElectricDamageLeft)
		alpha = 255;
	alpha--;
	if (alpha < 0)
		alpha = 0;
	Color clrShowGlow = Color(50, 175, 255, alpha);
	m_iDamageLeft = pPlayer->m_iElectricDamageLeft;
	m_iMaxPoints = pPlayer->m_iMaxElectricDamage;
	int max = (float)m_iMaxPoints;
	float val = (float)m_iDamageLeft;

	float perc = val / max; //we do it out of 29 to give a second of 0% 
	gHUD.DrawIconProgressBar(m_IconX, m_IconY, m_icon1, m_empty, 1.0 - perc, clrShock, CHud::HUDPB_VERTICAL);
	m_icon1->DrawSelf(m_IconX, m_IconY, clrShockBG);
	m_icon2->DrawSelf(m_IconX, m_IconY + 4, clrShowGlow);
}
void CHudEnvironmentResistElectric::TogglePrint(void)
{
	if (bShowIcon && !IsVisible())
		SetVisible(true);
	if (!bShowIcon && IsVisible())
	{
		SetVisible(false);
	}
}
void CHudEnvironmentResistElectric::LevelInit(void)
{
	SetVisible(false);
}