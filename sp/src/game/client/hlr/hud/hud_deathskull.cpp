#include "cbase.h"
#include "hud.h"
#include "hud_deathskull.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"
using namespace vgui;

DECLARE_HUDELEMENT(CHudDeathSkull);

CHudDeathSkull::CHudDeathSkull(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudDeathSkull")
{
	Panel *pParent = g_pClientMode->GetViewport();
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