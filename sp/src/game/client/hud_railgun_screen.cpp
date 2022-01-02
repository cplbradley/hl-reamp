
//=============================================================================//
//
// Purpose: Ask: This is a screen we'll use for the RPG
//
//=============================================================================//
#include "cbase.h"

#include "C_VGuiScreen.h"
#include <vgui/IVGUI.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include "clientmode_hlnormal.h"
#include "hud.h"
#include "hudelement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CRailgunScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS(CRailgunScreen, CVGuiScreenPanel);

public:
	CRailgunScreen(vgui::Panel *parent, const char *panelName);

	virtual bool Init(KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData);
	virtual void OnTick();

private:
	vgui::Label *m_pAmmoCount;

	CHudTexture *pIconFull;
	CHudTexture *pIconEmpty;
};

DECLARE_VGUI_SCREEN_FACTORY(CRailgunScreen, "railgun_battery_screen");


//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CRailgunScreen::CRailgunScreen(vgui::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName, g_hVGuiCombineScheme)
{
}

bool CRailgunScreen::Init(KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData)
{
	// Load all of the controls in
	if (!BaseClass::Init(pKeyValues, pInitData))
		return false;

	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal(GetVPanel());

	// Ask: Here we find a pointer to our AmmoCountReadout Label and store it in m_pAmmoCount
	m_pAmmoCount = dynamic_cast<vgui::Label*>(FindChildByName("RGAmmoCount"));

	pIconFull = gHUD.GetIcon("rg_battery_full");
	pIconEmpty = gHUD.GetIcon("rg_battery_empty");

	return true;
}

void CRailgunScreen::OnTick()
{
	BaseClass::OnTick();

	// Get our player
	CBasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// Get the players active weapon
	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

	// If pWeapon is NULL or it doesn't use primary ammo, don't update our screen
	if (!pWeapon || !pWeapon->UsesPrimaryAmmo())
		return;

	// Our RPG isn't clip-based, so we need to check the player's arsenal of rockets
	int ammo1 = pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());

	// If our Label exist
	if (m_pAmmoCount)
	{
		float maxammo = 30;
		float ammoleft = ammo1 / 15;
		float perc = ammoleft / maxammo;
		Color hud;
		if (ammoleft < 1)
			hud = Color(255, 0, 0);
		else
			hud = Color(150, 235, 255);
		gHUD.DrawIconProgressBar(0, 0, pIconFull, pIconEmpty, perc, hud, CHud::HUDPB_VERTICAL);
	}
}