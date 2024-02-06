#include "cbase.h"
#include "hud_itempickup.h"
#include "hud_macros.h"
#include <vgui_controls/Controls.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


extern ConVar hud_drawhistory_time;

DECLARE_HUDELEMENT(CHudItemPickup);
DECLARE_HUD_MESSAGE(CHudItemPickup, ItemPickup);


CHudItemPickup::CHudItemPickup(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudItemPickup")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
}

void CHudItemPickup::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}

void CHudItemPickup::Init()
{
	HOOK_HUD_MESSAGE(CHudItemPickup, ItemPickup);
	Reset();
	GetIcons();
}

void CHudItemPickup::Reset()
{
	for (int i = 0; i < itemList.Count(); i++)
	{
			itemList[i].iAmount = 0;
	}
}

void CHudItemPickup::Paint()
{
	int wide, tall;
	wide = GetWide();
	tall = GetTall();

	int talldiv = tall / 7;

	ConVarRef rainbow("hud_rainbow");
	ConVarRef haoverride("hud_override_healtharmor_color");

	for (int i = 0; i < itemList.Count(); i++)
	{
		Color hudclr, armor, health;
		hudclr = rainbow.GetBool() ? gHUD.GetRainbowColor() : gHUD.GetDefaultColor();
		

		armor = haoverride.GetBool() ? hudclr : Color(0, 190, 255, 200);
		health = haoverride.GetBool() ? hudclr : Color(0, 255, 0, 200);

		float elapsed = itemList[i].DrawTime - gpGlobals->curtime;
		float ratio = elapsed / hud_drawhistory_time.GetFloat();
		float scale = ratio;
		if (itemList[i].iAmount == 0)
			scale = 0;
		Color clr;
		if (Q_strcmp(itemList[i].iID, "Armor") == 0)
			clr = armor;
		else if (Q_strcmp(itemList[i].iID, "Health") == 0)
			clr = health;
		else
			clr = hudclr;
		if (scale <= 0.0f)
			scale = 0.0f;
		clr[3] = 200 * scale;

		if (elapsed <= 0.1f)
			itemList[i].iAmount = 0;

		int ypos = 0 + (talldiv * i) + vgui::surface()->GetFontTall(m_hNumberFont);

		wchar_t icon[16];
		_snwprintf(icon, sizeof(icon) / sizeof(wchar_t), itemList[i].icon);
		vgui::surface()->DrawSetTextFont(m_hIconFont);
		vgui::surface()->DrawSetTextColor(clr);
		vgui::surface()->DrawSetTextPos(0, ypos - (surface()->GetFontTall(m_hIconFont)*0.5f));
		vgui::surface()->DrawUnicodeString(icon);

		wchar_t text[16];
		_snwprintf(text, sizeof(text) / sizeof(wchar_t), L"+ %i", itemList[i].iAmount);
		vgui::surface()->DrawSetTextFont(m_hNumberFont);
		vgui::surface()->DrawSetTextColor(clr);
		vgui::surface()->DrawSetTextPos(m_flTextInset, ypos);
		vgui::surface()->DrawUnicodeString(text);
		
	}
}

void CHudItemPickup::GetIcons()
{
	ITEM_STRUCT pStruct[6];

	for (int i = 0; i < 6; i++)
	{
		switch (i)
		{
		case 0:
			pStruct[i].iID = "Health";
			pStruct[i].icon = L"+";
			break;
		case 1:
			pStruct[i].iID = "Armor";
			pStruct[i].icon = L"*";
			break;
		case 2:
			pStruct[i].iID = "Buckshot";
			pStruct[i].icon = L"s";
			break;
		case 3:
			pStruct[i].iID = "SMG1";
			pStruct[i].icon = L"p";
			break;
		case 4:
			
			pStruct[i].iID = "AR2";
			pStruct[i].icon = L"u";
			break;
		case 5:
			pStruct[i].iID = "RPG_Round";
			pStruct[i].icon = L"x";
			break;
		default:
			break;
		}
		itemList.AddToTail(pStruct[i]);
	}
	
}


void CHudItemPickup::MsgFunc_ItemPickup(bf_read& msg)
{
	char szItemName[256];

	msg.ReadString(szItemName, sizeof(szItemName));
	int iValue = msg.ReadShort();

	DevMsg("Picked up %i of %s\n", iValue, szItemName);

	for (int i = 0; i < itemList.Count(); i++)
	{
		if (!Q_strcmp(szItemName,itemList[i].iID))
		{
			itemList[i].iAmount += iValue;
			itemList[i].DrawTime = gpGlobals->curtime + hud_drawhistory_time.GetFloat();
		}
	}
}