#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_baseplayer.h"
#include "input.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "in_main.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "gamestringpool.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <KeyValues.h>
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Panel.h>
#include "../public/toolframework/ienginetool.h"
#include "vgui_controls/controls.h"
#include <vgui/IInput.h>
#include <vgui_controls/Button.h>
#include "strtools.h"
#include "cdll_util.h"

#include "vgui/ILocalize.h"


// you know the drill
#include "tier0/memdbgon.h"


#define MAX_WHEEL_SLOTS 10

using namespace vgui;

ConVar hud_weaponwheel_restrict_mouse("hud_weaponwheel_restrict_mouse", "1", FCVAR_CLIENTDLL);

class WeaponWheelLabel : public Panel
{
	DECLARE_CLASS_SIMPLE(WeaponWheelLabel, Panel);
public:
	WeaponWheelLabel(Panel* parent);
	char szWeaponName[128];
	char szWeaponSubName[128];

	Color mTextColor;

	void Paint();

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudWeaponWheelText");
};

WeaponWheelLabel::WeaponWheelLabel(Panel* parent)
{
	SetParent(parent);
	SetPaintBackgroundEnabled(false);
}
void WeaponWheelLabel::Paint()
{
	wchar_t tempstring[128];
	wchar_t tempstring2[128];

	g_pVGuiLocalize->ConvertANSIToUnicode(szWeaponName, tempstring, sizeof(tempstring));
	g_pVGuiLocalize->ConvertANSIToUnicode(szWeaponSubName, tempstring2, sizeof(tempstring2));

	int tx1, ty1, tx2, ty2;
	tx1 = (GetWide() / 2) - (g_pMatSystemSurface->DrawTextLen(m_hTextFont, szWeaponName) / 2);
	ty1 = (GetTall() / 2) - (surface()->GetFontTall(m_hTextFont) * 2.5f);
	tx2 = (GetWide() / 2) - (g_pMatSystemSurface->DrawTextLen(m_hTextFont, szWeaponSubName) / 2);
	ty2 = ty1 + (surface()->GetFontTall(m_hTextFont) * 1.25f);

	surface()->DrawSetTextColor(mTextColor);
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(tx1, ty1);
	surface()->DrawUnicodeString(tempstring);
	surface()->DrawSetTextPos(tx2, ty2);
	surface()->DrawUnicodeString(tempstring2);

}
class WeaponWheelButton : public Panel
{
	DECLARE_CLASS_SIMPLE(WeaponWheelButton, Panel);
public:
	WeaponWheelButton(Panel* parent);
	float fAmmoCount;
	float fMaxAmmo;
	void Paint();
	void OnThink();

	void OnMousePressed(MouseCode code);
	bool bInitialized;
	bool bSelected;
	
	Color GetButtonDrawColor();

	bool IsSelected() { return bSelected; }
	C_BaseCombatWeapon* pWeapon;

	char mAmmoIcon;

};

WeaponWheelButton::WeaponWheelButton(Panel* parent)
{
	SetParent(parent);
	SetVisible(false);
	
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetMouseInputEnabled(true);
	bInitialized = false;
}

void WeaponWheelButton::OnThink()
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();

	if (pWeapon)
	{
		bSelected = IsCursorOver();

		if (!bInitialized)
		{
			SetSize(pWeapon->GetSpriteActive()->Width(), pWeapon->GetSpriteActive()->Height());
			bInitialized = true;
		}

		fAmmoCount = pPlayer->GetAmmoCount(pWeapon->m_iPrimaryAmmoType);
		fMaxAmmo = GetAmmoDef()->MaxCarry(pWeapon->m_iPrimaryAmmoType);
	}
}
void WeaponWheelButton::OnMousePressed(MouseCode code)
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();

	if (bSelected)
	{
		if (pPlayer->Weapon_CanSwitchTo(pWeapon))
		{
			::input->MakeWeaponSelection(pWeapon);
		}
	}

	Msg("Mouse Pressed");
}

void WeaponWheelButton::Paint()
{
	if (bInitialized && pWeapon->GetSpriteActive())
	{
		pWeapon->GetSpriteActive()->DrawSelf(0, 0, GetWide(), GetTall(), GetButtonDrawColor());
		pWeapon->GetSpriteInactive()->DrawSelf(0, 0, GetWide(), GetTall(), GetButtonDrawColor());
	}
}

Color WeaponWheelButton::GetButtonDrawColor()
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	Color clr;
	clr = bSelected ? gHUD.GetDefaultColor() : gHUD.GetDefaultBGColor();

	if (pWeapon && !pPlayer->Weapon_CanSwitchTo(pWeapon))
		clr = Color(190, 0, 0, 200);

	return clr;

}
class CHudWeaponWheel : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudWeaponWheel, vgui::Panel);
public:
	CHudWeaponWheel(const char* pElementName);
	void Paint();
	void OnThink();
	void OpenWheel();
	void CloseWheel();
	bool ShouldDraw() { return bShouldDraw; }

	void SetupButtons();
	void UpdateButtons();

	void GetAmmoIcon(const char* pAmmoName);

	
	virtual C_BaseCombatWeapon* GetWeaponOnWheel(int iPos);

private:
	CHudTexture*			pWheelBackground;
	int						m_iSelectedSlot;
	bool bShouldDraw = false;

	float fAvailableAmmo;
	float fMaxAmmo;

	WeaponWheelButton* weaponbox[10];
	WeaponWheelLabel* label;

	wchar_t* icon;

	CPanelAnimationVarAliasType(float, m_barwidth, "bar_width", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_barheight, "bar_height", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_boxheight, "box_height", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_boxwide, "box_wide", "0", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsSmall")

};

DECLARE_HUDELEMENT(CHudWeaponWheel);

void IN_WeaponWheel_Pressed(const CCommand& args)
{
	CHudWeaponWheel* pwheel = GET_HUDELEMENT(CHudWeaponWheel);
	
	if (pwheel)
	{
		Msg("WeaponWheelPressed\n");
		pwheel->OpenWheel();
	}
}
void IN_WeaponWheel_Released(const CCommand& args)
{
	CHudWeaponWheel* pwheel = GET_HUDELEMENT(CHudWeaponWheel);
	
	if (pwheel)
	{
		Msg("WeaponWheelReleased\n");
		pwheel->CloseWheel();
	}
}
static ConCommand openweaponwheel("+weaponwheel", IN_WeaponWheel_Pressed);
static ConCommand closeweaponwheel("-weaponwheel", IN_WeaponWheel_Released);

CHudWeaponWheel::CHudWeaponWheel(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudWeaponWheel")
{
	Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetVisible(false);
	SetupButtons();
	GetAmmoIcon("");

	fAvailableAmmo = 1;
	fMaxAmmo = 1;

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
	pWheelBackground = gHUD.GetIcon("weaponwheel_background");

	PrecacheMaterial("hud/weaponwheel_background");
}
void CHudWeaponWheel::OnThink()
{
	if (!pWheelBackground)
		pWheelBackground = gHUD.GetIcon("weaponwheel_background");

	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();

	if (ShouldDraw())
	{
		for (int i = 0; i < MAX_WHEEL_SLOTS; i++)
		{
			if (weaponbox[i]->IsSelected())
			{
				if(m_iSelectedSlot != i)
				{
					pPlayer->EmitSound("Generic.Click");
					m_iSelectedSlot = i;
				}
				fAvailableAmmo = weaponbox[i]->fAmmoCount;
				fMaxAmmo = weaponbox[i]->fMaxAmmo;
				if (weaponbox[i]->pWeapon)
				{
					V_strcpy(label->szWeaponName, g_pVGuiLocalize->FindAsUTF8(weaponbox[i]->pWeapon->GetWheelName()));
					V_strcpy(label->szWeaponSubName, g_pVGuiLocalize->FindAsUTF8(weaponbox[i]->pWeapon->GetWheelSubName()));

					//Msg("%s Selected\n", STRING(szWeaponName));
					GetAmmoIcon(weaponbox[i]->pWeapon->GetWpnData().szAmmo1);
				}

				pPlayer->Weapon_CanSwitchTo(weaponbox[i]->pWeapon) ? label->mTextColor = gHUD.GetDefaultColor() : label->mTextColor = Color(200, 0, 0, 200);
			}
		}
		if (hud_weaponwheel_restrict_mouse.GetBool())
		{
			int vx, vy, vw, vh;
			vgui::surface()->GetFullscreenViewport(vx, vy, vw, vh);
			float screenWidth = vw;
			float screenHeight = vh;

			float screenCenterX = (screenWidth / 2);
			float screenCenterY = (screenHeight / 2);

			float maxdistance = (GetWide() * 0.4f);

			int cursorX, cursorY;

			vgui::input()->GetCursorPosition(cursorX, cursorY);

			float distance = sqrt(pow(cursorX - screenCenterX, 2) + pow(cursorY - screenCenterY, 2));

			if (distance > maxdistance)
			{
				float angle = atan2(cursorY - screenCenterY, cursorX - screenCenterX);

				int newCursorX = screenCenterX + maxdistance * cos(angle);
				int newCursorY = screenCenterY + maxdistance * sin(angle);

				vgui::input()->SetCursorPos(newCursorX, newCursorY);
			}
		}
	}

	BaseClass::OnThink();
}
void CHudWeaponWheel::OpenWheel()
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if ((pPlayer) && (!(pPlayer->m_Local.m_iHideHUD & HIDEHUD_WEAPON_WHEEL)))
	{
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_WEAPON_WHEEL;
	}
	bShouldDraw = true;
	SetVisible(true);
	RequestFocus();
	MakePopup();
	UpdateButtons();

	engine->ClientCmd("mat_blurdarken 1\n");
	engine->ClientCmd("sv_cheats 1\n");
	engine->ClientCmd("host_timescale 0.3\n");

}

void CHudWeaponWheel::CloseWheel()
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if ((pPlayer) && (pPlayer->m_Local.m_iHideHUD & HIDEHUD_WEAPON_WHEEL))
	{
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_WEAPON_WHEEL;
	}

	for (int i = 0; i < MAX_WHEEL_SLOTS; i++)
	{
		if (weaponbox[i]->bSelected && weaponbox[i]->pWeapon && pPlayer->Weapon_CanSwitchTo(weaponbox[i]->pWeapon))
		{
			::input->MakeWeaponSelection(weaponbox[i]->pWeapon);
		}
	}
	bShouldDraw = false;
	SetVisible(false);
	engine->ClientCmd("mat_blurdarken 0\n");
	engine->ClientCmd("host_timescale 1\n");
}

void CHudWeaponWheel::Paint()
{
	if (!ShouldDraw())
		return;

	if (pWheelBackground)
		pWheelBackground->DrawSelf(0, 0, GetWide(), GetTall(), gHUD.GetDefaultColor());

	int screenCenterX = (GetWide() / 2);
	int screenCenterY = (GetTall() / 2);

	int cursorx, cursory;
	vgui::input()->GetCursorPosition(cursorx, cursory);
	int endposx, endposy, xpos, ypos;
	GetPos(xpos, ypos);
	endposx = cursorx - xpos;
	endposy = cursory - ypos;

	surface()->DrawSetColor(gHUD.GetDefaultBGColor());
	surface()->DrawLine(screenCenterX, screenCenterY, endposx, endposy);
	
	float ammopercentage = fAvailableAmmo / fMaxAmmo;
	Color barclr;

	ammopercentage <= 0 ? barclr = Color(200, 0, 0, 200) : barclr = gHUD.GetDefaultColor();

	int barxpos, barypos, barwide, bartall;
	barxpos = GetWide() * 0.5 - 128;
	barypos = GetTall() * 0.5 + 64;
	barwide = 256;
	bartall = 16;


	gHUD.DrawProgressBar(barxpos, barypos, barwide, bartall, ammopercentage, barclr, CHud::HUDPB_HORIZONTAL_INV);

	wchar_t tempicon[16];


	_snwprintf(tempicon, sizeof(tempicon) / sizeof(wchar_t), icon);
	surface()->DrawSetTextFont(m_hIconFont);
	surface()->DrawSetTextPos(barxpos - surface()->GetFontTall(m_hIconFont) / 2, barypos - surface()->GetFontTall(m_hIconFont) / 2);
	surface()->DrawSetTextColor(gHUD.GetDefaultColor());
	surface()->DrawUnicodeString(tempicon);
}

C_BaseCombatWeapon* CHudWeaponWheel::GetWeaponOnWheel(int iPos)
{
	C_BasePlayer* player = C_BasePlayer::GetLocalPlayer();
	if (!player)
		return NULL;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon* pWeapon = player->GetWeapon(i);

		if (pWeapon == NULL)
			continue;

		if (pWeapon->GetWheelPosition() == iPos)
			return pWeapon;
	}

	return NULL;
}

void CHudWeaponWheel::SetupButtons()
{
	for (int i = 0; i < MAX_WHEEL_SLOTS; i++)
	{
		weaponbox[i] = new WeaponWheelButton(this);
		char str[16] = "WheelButton";
		sprintf(str + strlen(str), "%i", i);
		weaponbox[i]->SetName(str);

		if (GetWeaponOnWheel(i))
		{
			weaponbox[i]->pWeapon = GetWeaponOnWheel(i);
			weaponbox[i]->SetVisible(true);
		}
	}

	label = new WeaponWheelLabel(this);
	label->SetName("WheelLabel");

	
}
void CHudWeaponWheel::UpdateButtons()
{

	double anglebetween = 2 * M_PI / 10;

	fAvailableAmmo = weaponbox[m_iSelectedSlot]->fAmmoCount;
	fMaxAmmo = weaponbox[m_iSelectedSlot]->fMaxAmmo;


	for (int i = 0; i < MAX_WHEEL_SLOTS; i++)
	{
		double radius = GetWide() * 0.35;

		double centerx = GetWide() / 2;
		double centery = GetTall() / 2;
		double angle = i * anglebetween;
		double x = centerx + radius * cos(angle);
		double y = centery + radius * sin(angle);

		if(GetWeaponOnWheel(i))
		{

			weaponbox[i]->pWeapon = GetWeaponOnWheel(i);
			weaponbox[i]->SetPos(x - weaponbox[i]->pWeapon->GetSpriteActive()->Width() * 0.5f, y - weaponbox[i]->pWeapon->GetSpriteActive()->Height() * 0.5f);
			weaponbox[i]->SetVisible(true);
			
		}
	}

	label->SetSize(GetWide() / 2, GetTall() / 2);

	int centerx, centery, x, y;

	centerx = GetWide() / 2;
	centery = GetTall() / 2;
	x = centerx - label->GetWide() / 2;
	y = centery - label->GetTall() / 2;
	label->SetPos(x, y);
}

void CHudWeaponWheel::GetAmmoIcon(const char* pAmmoName)
{
	icon = L"y";

	if (Q_strcmp(pAmmoName,"AR2") == 0)
		icon = L"u";
	if (Q_strcmp(pAmmoName,"SMG1") == 0)
		icon = L"p";
	if (Q_strcmp(pAmmoName,"Buckshot") == 0)
		icon = L"s";
	if (Q_strcmp(pAmmoName,"rpg_round") == 0)
		icon = L"x";
	if (Q_strcmp(pAmmoName,"AR2AltFire") == 0)
		icon = L"z";
}