#include "cbase.h"
#include "hud.h"
#include "hud_deathskull.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

#include "vgui\ILocalize.h"

#include "tier0/memdbgon.h"
using namespace vgui;

DECLARE_HUDELEMENT(CHudDeathScreen);
DECLARE_HUD_MESSAGE(CHudDeathScreen, DeathScreen);

CHudDeathScreen::CHudDeathScreen(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudDeathScreen")
{
	Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetPaintBackgroundEnabled(true);
	SetEnabled(true);
	SetVisible(false);
	SetPaintBackgroundType(0);
	alpha = 0;
	Init();
}

void CHudDeathScreen::Init()
{
	HOOK_HUD_MESSAGE(CHudDeathScreen, DeathScreen);
	alpha = 0;
}

void CHudDeathScreen::VidInit()
{
	Init();
}
void CHudDeathScreen::MsgFunc_DeathScreen(bf_read& msg)
{
	char name[128];
	msg.ReadString(name, sizeof(name));
	SetKiller(name);
	panelwidth = 0;
	SetVisible(true);
	fDrawTime = gpGlobals->curtime + 1.f;
	SetWide(1);
}

bool CHudDeathScreen::SetKiller(const char* name)
{
	if (name == NULL || name[0] == L'\0')
		return false;

	const char* chLabel = g_pVGuiLocalize->FindAsUTF8("#HLR_KilledBy");
	char chlocal[256];
	V_strncpy(chlocal, chLabel, sizeof(chlocal));


	if (name[0] == '#')
	{
		const char* local = g_pVGuiLocalize->FindAsUTF8(name);
		V_strncat(chlocal, local, sizeof(chlocal));
		const char* output = chlocal;
		V_strncpy(szKiller, output, sizeof(szKiller));
		return true;

	}
	else
	{
		V_strncat(chlocal, name,sizeof(chlocal));
		const char* output = chlocal;
		V_strncpy(szKiller, output, sizeof(szKiller));
		return true;
	}

	return false;
}
void CHudDeathScreen::OnThink()
{
	C_BasePlayer* local = C_BasePlayer::GetLocalPlayer();
	if (!local)
		return;

	int health = local->GetHealth();

	(health <= 0) ? SetVisible(true) : SetVisible(false);
}

void CHudDeathScreen::Paint()
{
	if (fDrawTime > gpGlobals->curtime)
		return;

	int vx, vy, vw, vh;
	vgui::surface()->GetFullscreenViewport(vx, vy, vw, vh);
	float screenWidth = vw;
	float screenHeight = vh;

	float x = screenWidth / 2;
	float y = screenHeight / 2;
	int xInset = g_pMatSystemSurface->DrawTextLen(m_hTextFont, "_");

	int maxW = g_pMatSystemSurface->DrawTextLen(m_hTextFont, szKiller) + (xInset * 4);
	int maxH = surface()->GetFontTall(m_hTextFont) + xInset;

	panelwidth += 4;
	panelwidth = Clamp(panelwidth, 0, maxW);

	SetSize(panelwidth, maxH);
	SetPos(x - (panelwidth * 0.5),y + (maxH * 3));

	if (panelwidth >= maxW)
		alpha++;

	alpha = Clamp(alpha, 0, 200);

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(Color(255, 0, 0, alpha));
	
	int xOffset = xInset * 0.5f;
	int yOffset = xInset * 0.5f;
	wchar_t tempstring[256];
	g_pVGuiLocalize->ConvertANSIToUnicode(szKiller, tempstring, sizeof(tempstring));
	surface()->DrawSetTextPos(xOffset, yOffset);
	surface()->DrawUnicodeString(tempstring);
	surface()->DrawSetTextFont(m_hGlowFont);
	surface()->DrawSetTextColor(Color(255, 0, 0, alpha));
	surface()->DrawSetTextPos(xOffset, yOffset);
	surface()->DrawUnicodeString(tempstring);

}


DECLARE_HUDELEMENT(CHudDeathSkull);


CHudDeathSkull::CHudDeathSkull(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudDeathSkull")
{
	Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	m_icon = gHUD.GetIcon("deathskull");

	SetVisible(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetEnabled(true);
	SetAlpha(255);
	m_health = -1;

	//AW Create Texture for Looking around
	/*m_nBlueKey = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nBlueKey, "HUD/BlueKey", true, true);*/

	//SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}
void CHudDeathSkull::VidInit(void)
{
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();

	if (local)
		m_health = local->GetHealth();

	m_icon = gHUD.GetIcon("deathskull");

	
}
void CHudDeathSkull::OnThink()
{
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		m_health = local->GetHealth();
	}

	(m_health <= 0) ? SetVisible(true) : SetVisible(false); 
}



void CHudDeathSkull::Paint()
{
	if (!m_icon)
	{
		DevMsg("CAN'T FIND THE ICON FUCK\n");
		return;
	}

	Color white = Color(255, 255, 255, 255);

	/*int x0 = ScreenWidth() / 2;
	int y0 = ScreenHeight() / 2;
	int x1 = (x0 - (m_icon->Width() / 2));
	int y1 = (y0 - (m_icon->Height() / 2));
	m_icon->DrawSelf(x1, y1, m_icon->Width(), m_icon->Height(), Color(255, 255, 255, 255));*/

	int vx, vy, vw, vh;
	vgui::surface()->GetFullscreenViewport(vx, vy, vw, vh);
	float screenWidth = vw;
	float screenHeight = vh;

	float x = screenWidth / 2;
	float y = screenHeight / 2;

	int iTextureW = m_icon->Width();
	int iTextureH = m_icon->Height();

	m_icon->DrawSelf(x - (iTextureW / 2), y - (iTextureH / 2), white);


}