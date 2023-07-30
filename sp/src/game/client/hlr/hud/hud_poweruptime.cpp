#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "hl2_shareddefs.h"
#include "hl2_gamerules.h"
#include "engine/IEngineSound.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "ammodef.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "hud_poweruptime.h"

#include "tier0/memdbgon.h"

//	____________ _____ ________  ___  ___   _   _  _____  ______ _   _________   __
//	|  ___| ___ \  ___|  ___|  \/  | / _ \ | \ | |/  ___| |  ___| | | | ___ \ \ / /
//	| |_  | |_/ / |__ | |__ | .  . |/ /_\ \|  \| |\ `--.  | |_  | | | | |_/ /\ V / 
//	|  _| |    /|  __||  __|| |\/| ||  _  || . ` | `--. \ |  _| | | | |    /  \ /  
//	| |   | |\ \| |___| |___| |  | || | | || |\  |/\__/ / | |   | |_| | |\ \  | |  
//	\_|   \_| \_\____/\____/\_|  |_/\_| |_/\_| \_/\____/  \_|    \___/\_| \_| \_/  
//

DECLARE_HUDELEMENT(CHudProgressFF);

CHudProgressFF::CHudProgressFF(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudProgressFF")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudProgressFF::Init(void)
{
	m_iTimeLeft = 0;
	bShowIcon = false;
	m_icon_powerup = gHUD.GetIcon("icon_powerupFF");
	m_empty = gHUD.GetIcon("empty");
	m_icon_background = gHUD.GetIcon("icon_powerupFF");
	m_icon_glow = gHUD.GetIcon("icon_powerupFF_glow");
	SetPaintBackgroundEnabled(false);
}

bool CHudProgressFF::ShouldDraw()
{
	CBasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return false;

	if (!pPlayer->IsAlive())
	{
		bShowIcon = false;
		return false;
	}
	return bShowIcon;
}
void CHudProgressFF::OnThink(void)
{
	TogglePrint();
	//float perc = (float)m_iTimeLeft / 30.0f;
	//DevMsg("hud element timeleft = %f", perc);
}

void CHudProgressFF::VidInit(void)
{
	Init();
}
void CHudProgressFF::Paint()
{
	if (!ShouldDraw())
		return;
	Color clrNormal = Color(255, 0, 0, 200);
	Color clrBG = Color(128, 0, 0, 64);

	float perc = (float)m_iTimeLeft / 29.0f; //we do it out of 29 to give a second of 0% 
	gHUD.DrawIconProgressBar(m_IconX, m_IconY, m_icon_powerup, m_empty, 1.0 - perc, clrNormal, CHud::HUDPB_VERTICAL);
	m_icon_background->DrawSelf(m_IconX, m_IconY, clrBG);
	m_icon_glow->DrawSelf(m_IconX, m_IconY + 2, clrNormal);

	//gHUD.DrawIconProgressBarExt(m_IconX, m_IconY, m_icon_powerup->Width(), m_icon_powerup->Height(), m_icon_powerup, m_empty, 1.0 - perc, clrNormal, CHud::HUDPB_VERTICAL);
}
void CHudProgressFF::TogglePrint(void)
{
	if (bShowIcon && !IsVisible())
		SetVisible(true);
	if (!bShowIcon && IsVisible())
	{
		SetVisible(false);
	}
}
void CHudProgressFF::LevelInit(void)
{
	SetVisible(false);
}


//-------------------------------------------------------------------------------------------------//
//  ______   ____    ____  _______ .______       _______  .______       __  ____    ____  _______  //
// /  __  \  \   \  /   / |   ____||   _  \     |       \ |   _  \     |  | \   \  /   / |   ____| //
//|  |  |  |  \   \/   /  |  |__   |  |_)  |    |  .--.  ||  |_)  |    |  |  \   \/   /  |  |__    //
//|  |  |  |   \      /   |   __|  |      /     |  |  |  ||      /     |  |   \      /   |   __|   //
//|  `--'  |    \    /    |  |____ |  |\  \----.|  '--'  ||  |\  \----.|  |    \    /    |  |____  //
// \______/      \__/     |_______|| _| `._____||_______/ | _| `._____||__|     \__/     |_______| //
//-------------------------------------------------------------------------------------------------//

DECLARE_HUDELEMENT(CHudProgressOD);

CHudProgressOD::CHudProgressOD(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudProgressOD")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudProgressOD::Init(void)
{
	m_iTimeLeft = 0;
	bShowIcon = false;
	m_icon_powerup = gHUD.GetIcon("icon_powerupOD");
	m_empty = gHUD.GetIcon("empty");
	m_icon_background = gHUD.GetIcon("icon_powerupOD");
	m_icon_glow = gHUD.GetIcon("icon_powerupOD_glow");
	SetPaintBackgroundEnabled(false);
}

void CHudProgressOD::OnThink(void)
{
	TogglePrint();
	//float perc = (float)m_iTimeLeft / 30.0f;
	//DevMsg("hud element timeleft = %f", perc);
}

void CHudProgressOD::VidInit(void)
{
	Init();
}
void CHudProgressOD::Paint()
{
	if (!bShowIcon)
		return;
	Color clrNormal = Color(255, 105, 0, 200);
	Color clrBG = Color(90, 30, 0, 64);

	float perc = (float)m_iTimeLeft / 29.0f; //we do it out of 29 to give a second of displaying 0% 
	gHUD.DrawIconProgressBar(m_IconX, m_IconY, m_icon_powerup, m_empty, 1.0 - perc, clrNormal, CHud::HUDPB_VERTICAL);
	m_icon_background->DrawSelf(m_IconX, m_IconY, GetWide(), GetTall(), clrBG);
	m_icon_glow->DrawSelf(m_IconX, m_IconY + 2, GetWide(), GetTall(), clrNormal);
	//gHUD.DrawIconProgressBarExt(m_IconX, m_IconY, m_icon_powerup->Width(), m_icon_powerup->Height(), m_icon_powerup, m_empty, 1.0 - perc, clrNormal, CHud::HUDPB_VERTICAL);
}
void CHudProgressOD::TogglePrint(void)
{
	if (bShowIcon && !IsVisible())
		SetVisible(true);
	if (!bShowIcon && IsVisible())
	{
		SetVisible(false);
	}
}
void CHudProgressOD::LevelInit(void)
{
	SetVisible(false);
}


//////////////////////////////////////////////////////////
//	   ___  _   _  _   ___       _ _   _ __  __ ___		//
//	  / _ \| | | |/_\ |   \   _ | | | | |  \/  | _ \	//
//	 | (_) | |_| / _ \| |) | | || | |_| | |\/| |  _/	//
//	  \__\_\\___/_/ \_\___/   \__/ \___/|_|  |_|_|		//
//														//
//////////////////////////////////////////////////////////

DECLARE_HUDELEMENT(CHudProgressQJ);

CHudProgressQJ::CHudProgressQJ(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudProgressQJ")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudProgressQJ::Init(void)
{
	m_iTimeLeft = 0;
	bShowIcon = false;
	m_icon_powerup = gHUD.GetIcon("icon_powerupQJ");
	m_empty = gHUD.GetIcon("empty");
	m_icon_background = gHUD.GetIcon("icon_powerupQJ");
	m_icon_glow = gHUD.GetIcon("icon_powerupQJ_glow");
	SetPaintBackgroundEnabled(false);
}

void CHudProgressQJ::OnThink(void)
{
	TogglePrint();
	//float perc = (float)m_iTimeLeft / 30.0f;
	//DevMsg("hud element timeleft = %f", perc);
}

void CHudProgressQJ::VidInit(void)
{
	Init();
}
void CHudProgressQJ::Paint()
{
	if (!bShowIcon)
		return;
	Color clrNormal = Color(255, 220, 0, 200);
	Color clrBG = Color(120, 100, 0, 64);

	float perc = (float)m_iTimeLeft / 29.0f; //we do it out of 29 to give a second of displaying 0% 
	//gHUD.DrawIconProgressBar(m_IconX, m_IconY, m_icon_powerup, m_empty, 1.0 - perc, clrNormal, CHud::HUDPB_VERTICAL);
	gHUD.DrawIconProgressBar(m_IconX, m_IconY, m_icon_powerup, m_empty, 1.0 - perc, clrNormal, CHud::HUDPB_VERTICAL);
	m_icon_background->DrawSelf(m_IconX, m_IconY, GetWide(), GetTall(),clrBG);
	m_icon_glow->DrawSelf(m_IconX, m_IconY + 2, GetWide(), GetTall(), clrNormal);
	
}
void CHudProgressQJ::TogglePrint(void)
{
	if (bShowIcon && !IsVisible())
		SetVisible(true);
	if (!bShowIcon && IsVisible())
	{
		SetVisible(false);
	}
}
void CHudProgressQJ::LevelInit(void)
{
	SetVisible(false);
}