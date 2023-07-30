//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//                                  VGUI WEAPON WHEEL                                   //
//                             BY JUST WAX @ VELOCITY PUNCH                             //
//                                                                                      //
//            Alright, listen up. To get the weapon wheel to work properly,             //
//                           you'll need to do a few things.                            //
//                                                                                      //
//  First, you'll need to add some keyvalues to the weapon script parser, which you'll  //
//                     also need to add to all the weapon scripts.                      //
//         You need icons for both the glow and main weapon icons. then, you'll         //
//                   need strings for the weapon names and subnames.			        //
//                   You're also gonna need to add functions to the                     //
//  shared baseweapon class that returns the keyvalues as usable variables/textures.    //
//                                                                                      //
//              Second, you're gonna need a background for the wheel that               //
//            has all the necessary slots. You're also gonna need to change             //
//                 MAX_WHEEL_SLOTS to the amount of slots you'll have.                  //
//                                                                                      //
//               Lastly, you're gonna need to make sure that you have an                //
//              entry in HudLayout.res that contains the variables for the              //
//              ammo count progress bar, as well as making sure to change               //
//                the character returns in SetAmmoIcon to the characters                //
//                           appropriate for your ammo types.                           //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////


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
#include "inputsystem/IInputSystem.h"

#include "vgui/ILocalize.h"

// you know the drill
#include "tier0/memdbgon.h"

#define MAX_WHEEL_SLOTS 10 //Total amount of wheel slots

using namespace vgui;

ConVar hud_weaponwheel_restrict_mouse("hud_weaponwheel_restrict_mouse", "1", FCVAR_CLIENTDLL);
ConVar debug_weapon_wheel("debug_weapon_wheel", "0");

///////////////////////////////////////////////////////////////////////////
///																		///
///                        Weapon Wheel Label							///
///	This displays the weapon information (name, ammo count, ammo type)	///
///																		///
///////////////////////////////////////////////////////////////////////////

class WeaponWheelLabel : public Panel
{
	DECLARE_CLASS_SIMPLE(WeaponWheelLabel, Panel);
public:
	WeaponWheelLabel(Panel* parent);
	char szWeaponName[80];
	char szWeaponSubName[80];

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

	//convert the const char* weapon name/sub name into a unicode format
	g_pVGuiLocalize->ConvertANSIToUnicode(szWeaponName, tempstring, sizeof(tempstring)); 
	g_pVGuiLocalize->ConvertANSIToUnicode(szWeaponSubName, tempstring2, sizeof(tempstring2));

	//place the text so that it's centered on the wheel
	int tx1, ty1, tx2, ty2;
	int inset = g_pMatSystemSurface->DrawTextLen(m_hTextFont, " "); //have to add this inset because localization adds a space to the front of strings, so offset it by the length of a space
	tx1 = (GetWide() / 2) - ((g_pMatSystemSurface->DrawTextLen(m_hTextFont, szWeaponName) / 2)) - inset; 
	ty1 = (GetTall() / 2) - (surface()->GetFontTall(m_hTextFont) * 2.5f);
	tx2 = (GetWide() / 2) - ((g_pMatSystemSurface->DrawTextLen(m_hTextFont, szWeaponSubName) / 2)) - inset;
	ty2 = ty1 + (surface()->GetFontTall(m_hTextFont) * 1.25f);
	
	//draw the text
	surface()->DrawSetTextColor(mTextColor);
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(tx1, ty1);
	surface()->DrawUnicodeString(tempstring);
	surface()->DrawSetTextPos(tx2, ty2);
	surface()->DrawUnicodeString(tempstring2);

}

///////////////////////////////////////////////////////////////////
///																///
///						Weapon Wheel Button	                    ///
///	This draws the weapon icon and handles the weapon switches	///
///																///
///////////////////////////////////////////////////////////////////


class WeaponWheelButton : public Panel
{
	DECLARE_CLASS_SIMPLE(WeaponWheelButton, Panel);
public:
	WeaponWheelButton(Panel* parent);


	struct WeaponBoxData_t
	{
		float fAmmoCount;
		float fMaxAmmo;
		CHandle<C_BaseCombatWeapon> hWeapon;
	};


	WeaponBoxData_t boxdata;
	void Paint();
	void OnThink();

	void OnMousePressed(MouseCode code);
	bool bSelected;
	bool bSoundEmitted;
	
	Color GetButtonDrawColor();

	C_BaseCombatWeapon* GetBoxWeapon() { return boxdata.hWeapon.Get(); }
	float GetBoxAmmoCount() { return boxdata.fAmmoCount; }
	float GetBoxMaxAmmo() { return boxdata.fMaxAmmo; }

	bool IsSelected() { return bSelected; }
	void SetWeapon(C_BaseCombatWeapon* pWeapon);
	char mAmmoIcon;

};

WeaponWheelButton::WeaponWheelButton(Panel* parent)
{
	SetParent(parent);
	SetVisible(false);
	SetSize(1, 1);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetMouseInputEnabled(true);
}

void WeaponWheelButton::OnThink()
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return;

	if (GetBoxWeapon())
	{
		bSelected = IsCursorOver();		
		SetSize(GetBoxWeapon()->GetWheelIconActive()->Width(), GetBoxWeapon()->GetWheelIconActive()->Height());
		//set the ammo counts to that of the weapon the player is hoving over
		boxdata.fAmmoCount = (float)pPlayer->GetAmmoCount(GetBoxWeapon()->m_iPrimaryAmmoType);
		boxdata.fMaxAmmo = (float)GetAmmoDef()->MaxCarry(GetBoxWeapon()->m_iPrimaryAmmoType);

		if (bSelected && !bSoundEmitted)
		{
			bSoundEmitted = true;
			pPlayer->EmitSound("Generic.Click");
		}
		else if (!bSelected && bSoundEmitted)
		{
			bSoundEmitted = false;
		}
	}
}
void WeaponWheelButton::SetWeapon(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return;
	boxdata.hWeapon.Set(pWeapon);

}
void WeaponWheelButton::OnMousePressed(MouseCode code) //switch to the weapon that the player clicks
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (bSelected)
	{
		if (GetBoxWeapon()->CanBeSelected())
		{
			::input->MakeWeaponSelection(GetBoxWeapon());
		}
	}
	if(debug_weapon_wheel.GetBool())
		Msg("Mouse Pressed");
}

void WeaponWheelButton::Paint()
{
	if (GetBoxWeapon())
	{
		if (GetBoxWeapon()->GetWheelIconActive())
		{
			//draw the weapon icons
			GetBoxWeapon()->GetWheelIconActive()->DrawSelf(0, 0, GetWide(), GetTall(), GetButtonDrawColor()); //Draw the background glow for the weapon icon
			GetBoxWeapon()->GetWheelIconInactive()->DrawSelf(0, 0, GetWide(), GetTall(), GetButtonDrawColor()); //Draw the actual weapon icon on top of the glow
		}
	}
}

Color WeaponWheelButton::GetButtonDrawColor()
{
	//get the correct color to use. if highligted, use FG color, if not, use BG color. if weapon has no ammo, override with red.
	Color clr;
	clr = bSelected ? gHUD.GetDefaultColor() : gHUD.GetDefaultBGColor();

	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (pPlayer && GetBoxWeapon() && !GetBoxWeapon()->CanBeSelected())
		clr = Color(190, 0, 0, 200);

	return clr;
}

///////////////////////////////////////////////////////////////////////
///																	///
///                          Weapon Wheel							///
///	This is the overall parent. draws the wheel, places the buttons	///
///      and relays info between the buttons and the label.			///
///																	///
///////////////////////////////////////////////////////////////////////


class CHudWeaponWheel : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudWeaponWheel, vgui::Panel);
public:
	CHudWeaponWheel(const char* pElementName);
	virtual void Paint();
	virtual void OnThink();
	void OpenWheel();
	void CloseWheel();
	virtual bool ShouldDraw();
	virtual void VidInit();
	virtual void Init(int x, int y, int wide, int tall);

	void HideWheel();

	void SetupButtons();
	void UpdateButtons();

	void SetAmmoIcon(const char* pAmmoName);

	
	virtual C_BaseCombatWeapon* GetWeaponOnWheel(int iPos);

private:
	CHudTexture*			pWheelBackground;
	int						m_iSelectedSlot;
	bool bShouldDraw = false;

	float fAvailableAmmo;
	float fMaxAmmo;

	int numWeapons;
	bool bHasBeenOpened;

	WeaponWheelButton* weaponbox[MAX_WHEEL_SLOTS];
	WeaponWheelLabel* label;

	wchar_t* icon;

	CPanelAnimationVarAliasType(float, m_barwidth, "bar_width", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_barheight, "bar_height", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_boxheight, "box_height", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_boxwide, "box_wide", "0", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hIconFont, "IconFont", "WeaponIcons")

};

DECLARE_HUDELEMENT(CHudWeaponWheel);

void IN_WeaponWheel_Pressed(const CCommand& args)
{
	CHudWeaponWheel* pwheel = GET_HUDELEMENT(CHudWeaponWheel);

	if (pwheel)
	{
		if(debug_weapon_wheel.GetBool())
			Msg("WeaponWheelPressed\n");
		pwheel->OpenWheel();
	}
}
void IN_WeaponWheel_Released(const CCommand& args)
{
	CHudWeaponWheel* pwheel = GET_HUDELEMENT(CHudWeaponWheel);

	if (pwheel)
	{
		if (debug_weapon_wheel.GetBool())
			Msg("WeaponWheelReleased\n");
		pwheel->CloseWheel();
	}
}

static ConCommand openweaponwheel("+weaponwheel", IN_WeaponWheel_Pressed);
static ConCommand closeweaponwheel("-weaponwheel", IN_WeaponWheel_Released);
ConVar joyvarx("joyvarx", "0");
ConVar joyvary("joyvary", "1");

CHudWeaponWheel::CHudWeaponWheel(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudWeaponWheel")
{
	Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetVisible(false);
	SetupButtons();
	SetAmmoIcon("");

	fAvailableAmmo = 1;
	fMaxAmmo = 1;
	bHasBeenOpened = false;
	m_iSelectedSlot = -1;
	numWeapons = 0;

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
	pWheelBackground = gHUD.GetIcon("weaponwheel_background");
}

void CHudWeaponWheel::VidInit()
{
	UpdateButtons(); // in event of resolution change, buttons need to be rescaled
}

void CHudWeaponWheel::Init(int x, int y, int wide, int tall)
{
	BaseClass::Init(x, y, wide, tall);
	UpdateButtons();
}

bool CHudWeaponWheel::ShouldDraw()
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return false;

	if (!pPlayer->m_Local.m_bWearingSuit)
		return false;

	if (!pPlayer->IsAlive())
	{
		CloseWheel(); //HACK: dying could cause the weapon wheel to enter a state of flux, resulting in a crash. if the player dies, make sure the wheel is fully closed.
		return false;
	}
	return bShouldDraw;
}
void CHudWeaponWheel::OnThink()
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	UpdateButtons();

	if (ShouldDraw())
	{
		
		if (!pWheelBackground)
			pWheelBackground = gHUD.GetIcon("weaponwheel_background"); // Wheel background load-in failsafe

		// Get ammo counts per-weapon and localization of weapon names

		for (int i = 0; i < MAX_WHEEL_SLOTS; i++) 
		{
			if (weaponbox[i] && weaponbox[i]->GetBoxWeapon() && weaponbox[i]->IsSelected())
			{
				fAvailableAmmo = weaponbox[i]->GetBoxAmmoCount();
				fMaxAmmo = weaponbox[i]->GetBoxMaxAmmo();

				if (debug_weapon_wheel.GetBool())
					engine->Con_NPrintf(1, "Current Ammo: %i Max Ammo: %i\n", (int)fAvailableAmmo, (int)fMaxAmmo);

				if (weaponbox[i]->GetBoxWeapon() == GetWeaponOnWheel(i))
				{
					const FileWeaponInfo_t& weaponInfo = GetWeaponOnWheel(i)->GetWpnData();
					//Get the localization of weapon name/subname
					V_strncpy(label->szWeaponName, g_pVGuiLocalize->FindAsUTF8(weaponInfo.szWheelName),sizeof(label->szWeaponName));
					V_strncpy(label->szWeaponSubName, g_pVGuiLocalize->FindAsUTF8(weaponInfo.szWheelSubName),sizeof(label->szWeaponSubName));
					//Get the ammo icon
					SetAmmoIcon(weaponbox[i]->GetBoxWeapon()->GetWpnData().szAmmo1);
					weaponbox[i]->GetBoxWeapon()->CanBeSelected() ? label->mTextColor = gHUD.GetDefaultColor() : label->mTextColor = Color(200, 0, 0, 200);
				}
				// color the text based on if the player has ammo or not
				
			}
		}
		if (hud_weaponwheel_restrict_mouse.GetBool()) // Handles cursor restriction and joystick control
		{
			int vx, vy, vw, vh;
			vgui::surface()->GetFullscreenViewport(vx, vy, vw, vh);
			float screenWidth = vw;
			float screenHeight = vh;

			float screenCenterX = (screenWidth / 2);
			float screenCenterY = (screenHeight / 2);

			// get the max distance the cursor can be from the center of the screen
			float maxdistance = (GetWide() * 0.35f); 

			int cursorX, cursorY;

			vgui::input()->GetCursorPosition(cursorX, cursorY);

			g_pInputSystem->EnableJoystickInput(0, true);

			// Get joystick x and y values
			int joystickin[2];
			joystickin[0] = g_pInputSystem->GetAnalogValue((AnalogCode_t)JOYSTICK_AXIS(0, joyvarx.GetInt()));
			joystickin[1] = g_pInputSystem->GetAnalogValue((AnalogCode_t)JOYSTICK_AXIS(0, joyvary.GetInt()));

			float joyx = ((float)joystickin[0] / 32768.0f);
			float joyy = ((float)joystickin[1] / 32768.0f);

			if (debug_weapon_wheel.GetBool())
				engine->Con_NPrintf(0, "JoyStick Values: %f %f\n", joyx, joyy);
			// get distance from center of screen by using pythagorean theorum 
			float distance = sqrt(pow(cursorX - screenCenterX, 2) + pow(cursorY - screenCenterY, 2)); 
			Vector joyvec = Vector(joyx, joyy, 0).Normalized();
			int newCursorX = cursorX;
			int newCursorY = cursorY;
			if (distance > maxdistance) //if cursor is too far
			{
				float angle = atan2(cursorY - screenCenterY, cursorX - screenCenterX); //constrain the distance
				newCursorX = screenCenterX + maxdistance * cos(angle);
				newCursorY = screenCenterY + maxdistance * sin(angle);
			}
			if (joyx != 0 || joyy != 0) //if the joystick values aren't 0 (is being used)
			{
				newCursorX = screenCenterX + joyvec[0] * maxdistance;
				newCursorY = screenCenterY + joyvec[1] * maxdistance;
			}

			vgui::input()->SetCursorPos(newCursorX, newCursorY); //update the new cursor position
		}
	}

	BaseClass::OnThink();
}
void CHudWeaponWheel::OpenWheel() //Opening the wheel
{
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
	if (!pPlayer->m_Local.m_bWearingSuit)
		return;

	if (!(pPlayer->m_Local.m_iHideHUD & HIDEHUD_WEAPON_WHEEL))
	{
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_WEAPON_WHEEL;
	}

	SetupButtons();
	UpdateButtons();
	bShouldDraw = true;
	SetVisible(true);
	RequestFocus();
	MakePopup(false,false,false);
	
	bHasBeenOpened = true;

	engine->ClientCmd("mat_blurdarken 1\n");
	engine->ClientCmd("sv_cheats 1\n");
	engine->ClientCmd("host_timescale 0.3\n");
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponWheelOpen");
}

void CHudWeaponWheel::CloseWheel()
{
	UpdateButtons(); //HACK: related to the dying issue above, force closing the wheel without updating the information first could cause the wheel to use incorrect weapon data, resulting in a crash.

	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
	if ((pPlayer) && (pPlayer->m_Local.m_iHideHUD & HIDEHUD_WEAPON_WHEEL))
	{
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_WEAPON_WHEEL;
	}
	if (pPlayer->IsAlive()) //only do this if the player is alive
	{
		for (int i = 0; i < MAX_WHEEL_SLOTS; i++)
		{
			if ((weaponbox[i] && weaponbox[i]->bSelected) && GetWeaponOnWheel(i)) //make extra sure we don't have any null pointers
			{
				if (weaponbox[i]->GetBoxWeapon()->CanBeSelected())
					::input->MakeWeaponSelection(weaponbox[i]->GetBoxWeapon()); //if the player can switch to the highlighted weapon, do it.
			}
		}
	}
	SetVisible(false);
	bShouldDraw = false;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponWheelClose");
	engine->ClientCmd("mat_blurdarken 0\n");
	engine->ClientCmd("host_timescale 1\n");
}

void CHudWeaponWheel::Paint()
{
	if (!ShouldDraw())
		return;

	Color color = gHUD.GetDefaultColor();
	color[3] = GetAlpha();
	//draw the wheel texture
	if (pWheelBackground)
		pWheelBackground->DrawSelf(0, 0, GetWide(), GetTall(), color);

	//get screen center
	int screenCenterX = (GetWide() / 2);
	int screenCenterY = (GetTall() / 2);

	//get cursor positioon
	int cursorx, cursory;
	vgui::input()->GetCursorPosition(cursorx, cursory);
	int endposx, endposy, xpos, ypos;
	GetPos(xpos, ypos);
	endposx = cursorx - xpos;
	endposy = cursory - ypos;

	surface()->DrawSetColor(gHUD.GetDefaultBGColor());
	surface()->DrawLine(screenCenterX, screenCenterY, endposx, endposy); //draw the line from center screen to cursor
	
	float ammopercentage = fAvailableAmmo / fMaxAmmo;
	Color barclr;

	ammopercentage <= 0 ? barclr = Color(200, 0, 0, 200) : barclr = color;

	int barxpos, barypos, barwide, bartall;
	barxpos = (GetWide() * 0.5) - (m_barwidth * 0.5);
	barypos = GetTall() * 0.5 + m_barheight * 2;
	barwide = m_barwidth;
	bartall = m_barheight;


	gHUD.DrawProgressBar(barxpos, barypos, barwide, bartall, ammopercentage, barclr, CHud::HUDPB_HORIZONTAL_INV); //draw the ammo count progress bar

	wchar_t tempicon[16];
	char charicon[16];
	
	_snwprintf(tempicon, sizeof(tempicon) / sizeof(wchar_t), icon);
	Q_UnicodeToUTF8(tempicon, charicon, sizeof(charicon));

	surface()->DrawSetTextFont(m_hIconFont);
	surface()->DrawSetTextPos(screenCenterX - g_pMatSystemSurface->DrawTextLen(m_hIconFont,charicon) * 0.5, barypos);
	surface()->DrawSetTextColor(color);
	surface()->DrawUnicodeString(tempicon); //draw the ammo icon next to the progress bar
	
}

C_BaseCombatWeapon* CHudWeaponWheel::GetWeaponOnWheel(int iPos) // Get the weapon associated with the specific slot position, if there is one. Will return NULL if there is no weapon, or the player doesn't have it. 
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
		{
			return pWeapon;
		}
	}

	return NULL;
}

void CHudWeaponWheel::SetupButtons() // Create the buttons. Placement is handled in the UpdateButtons() function.
{
	for (int i = 0; i < MAX_WHEEL_SLOTS; i++)
	{
		if (!weaponbox[i])
		{
			weaponbox[i] = new WeaponWheelButton(this);

			if (weaponbox[i])
			{
				char str[16] = "WheelButton";
				sprintf(str + strlen(str), "%i", i);
				weaponbox[i]->SetName(str);

				if (GetWeaponOnWheel(i))
				{
					weaponbox[i]->SetWeapon(GetWeaponOnWheel(i));
					weaponbox[i]->SetVisible(true);
					numWeapons++;
				}
			}
		}
	}

	if (!label)
	{
		label = new WeaponWheelLabel(this);
		label->SetName("WheelLabel");
	}
	
}
void CHudWeaponWheel::UpdateButtons()
{
	//So this one is a bit of a doozy. Basically, when the buttons are created, they're placed at a specified distance from the center of the screen, and each one's position is offset
	//based on rotation around the center of the screen, in this case 36 degrees because there's 10 buttons, but it could be anything, which is what below does.

	double dAngleBetween = 2 * M_PI / MAX_WHEEL_SLOTS; //Get the angle to rotate around the origin by.

	for (int i = 0; i < MAX_WHEEL_SLOTS; i++)
	{
		double radius = GetWide() * 0.35; //Distance from the center at which to place the buttons.

		double centerx = GetWide() / 2;
		double centery = GetTall() / 2;
		double angle = i * dAngleBetween; //total angle to rotate around the origin by (angle * button number)
		double x = centerx + radius * cos(angle); //final X placement
		double y = centery + radius * sin(angle); //final Y placement

		if(GetWeaponOnWheel(i) && weaponbox[i])
		{
			weaponbox[i]->SetWeapon(GetWeaponOnWheel(i)); //Verify that there's a weapon assigned to this slot, and if the player has it, add it to the weaponbox.

			//Offset the box position so that the center is on the calculated position
			int xPos = (int)x - weaponbox[i]->GetBoxWeapon()->GetWheelIconInactive()->Width() * 0.5f;
			int yPos = (int)y - weaponbox[i]->GetBoxWeapon()->GetWheelIconInactive()->Height() * 0.5f;

			weaponbox[i]->SetPos(xPos,yPos); //Place the box
			weaponbox[i]->SetVisible(true); //Make it visible
			
		}
	}

	label->SetSize(GetWide() / 2, GetTall() / 2); //Set label size

	int centerx, centery, x, y;

	centerx = GetWide() / 2;
	centery = GetTall() / 2;
	x = centerx - label->GetWide() / 2;
	y = centery - label->GetTall() / 2;
	label->SetPos(x, y); // Place label

	/*if ((weaponbox[m_iSelectedSlot]->IsSelected())) //Get the ammo count from the selected button, send it to the progress bar.
	{
		fAvailableAmmo = weaponbox[m_iSelectedSlot]->boxdata->fAmmoCount;
		fMaxAmmo = weaponbox[m_iSelectedSlot]->boxdata->fMaxAmmo;
	}*/

}

void CHudWeaponWheel::SetAmmoIcon(const char* pAmmoName) //returns the font icon for the weapon's ammo
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