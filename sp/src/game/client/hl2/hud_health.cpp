//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1


ConVar hud_override_healtharmor_color("hud_override_healtharmor_color", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudHealth, Panel );

public:
	CHudHealth( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
			void MsgFunc_Damage( bf_read &msg );
			void MsgFunc_Battery(bf_read &msg);
			void MsgFunc_SuitPickup(bf_read &msg);
	virtual void PaintValue(vgui::HFont font, int xpos, int ypos, int value, Color &color);


	void GetIcons();
	virtual void Paint(void);

private:
	// old variables
	int		m_iHealth;
	int		m_iArmor;
	int		iNewArmor;
	CHudTexture *m_iconHealth;
	CHudTexture *m_iconArmor;
	CHudTexture *m_background;
	int		m_bitsDamage;
	Color healthbarcolor;
	Color armorcolor;
	Color bgcolor;
	CHudNumericDisplay *nHealth;


	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HealthBarValues");
	CPanelAnimationVarAliasType(float, m_IconX, "icon1_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_IconY, "icon1_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_Icon2X, "icon2_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_Icon2Y, "icon2_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_textX, "digit_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_textY, "digit_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_barwidth, "bar_width", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_barheight, "bar_height", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_bar1xpos, "bar1_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_bar1ypos, "bar1_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_bar2xpos, "bar2_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_bar2ypos, "bar2_ypos", "0", "proportional_float");

};	

DECLARE_HUDELEMENT( CHudHealth );
DECLARE_HUD_MESSAGE( CHudHealth, Damage );
DECLARE_HUD_MESSAGE(CHudHealth, Battery);
DECLARE_HUD_MESSAGE(CHudHealth, SuitPickup);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth( const char *pElementName ) : CHudElement( pElementName ), BaseClass(NULL, "HudHealth")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_CINEMATIC_CAMERA);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	HOOK_HUD_MESSAGE( CHudHealth, Damage );
	HOOK_HUD_MESSAGE(CHudHealth, Battery);
	HOOK_HUD_MESSAGE(CHudHealth, SuitPickup);
	Reset();

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	m_iHealth		= INIT_HEALTH;
	m_bitsDamage	= 0;
	m_iArmor = INIT_HEALTH;;

	SetPaintBackgroundEnabled(false);
	GetIcons();
	/*wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_HEALTH");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"HEALTH");
	}*/
	//SetDisplayValue(m_iHealth);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
	Reset();
	SetPaintEnabled(true);

	SetVisible(true);
	

}


void CHudHealth::GetIcons()
{
	ConVarRef rainbow("hud_rainbow");
	m_iconArmor = gHUD.GetIcon("hud_healthbar_armor_icon");
	m_iconHealth = gHUD.GetIcon("hud_healthbar_health_icon");
	m_background = gHUD.GetIcon("hud_health_background");

	if (rainbow.GetBool() || hud_override_healtharmor_color.GetBool())
		m_background = gHUD.GetIcon("hud_health_background_mono");
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::OnThink()
{
	GetIcons();
	int newHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		// Never below zero
		newHealth = MAX(local->GetHealth(), 0);
	}


	ConVarRef rainbow("hud_rainbow");
	Color rainbowclr = gHUD.GetRainbowColor();
	Color defaultcolor = gHUD.GetDefaultColor();
	Color green = Color(120, 255, 0, 200);
	Color blue = Color(0, 210, 255, 200);

	Color armorIn, healthIn;
	
	if (rainbow.GetBool() || hud_override_healtharmor_color.GetBool())
	{
		bgcolor = rainbow.GetBool() ? rainbowclr : defaultcolor;
		armorIn = bgcolor;
		healthIn = bgcolor;
	}
	else
	{
		bgcolor = Color(255, 255, 255, 255);
		armorIn = blue;
		healthIn = green;
	}
	// Only update the fade if we've changed health
/*	if ( newHealth == m_iHealth && newArmor == m_iArmor)
	{
		return;
	}*/
	m_iHealth = newHealth;
	m_iArmor = iNewArmor;
	if (m_iHealth > 100)
	{
		healthbarcolor = Color(255, 220, 0, 200);
	}
	else if ( m_iHealth >= 30 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
		healthbarcolor = healthIn;
	}
	else if (m_iHealth > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedBelow20");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
		healthbarcolor = Color(150, 0, 0, 200);
		bgcolor = healthbarcolor;
	}

	if (m_iArmor > 100)
		armorcolor = Color(255, 220, 0, 200);
	else if (m_iHealth >= 30)
		armorcolor = armorIn;
	else
		armorcolor = healthbarcolor;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_Damage( bf_read &msg )
{

	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits
	bitsDamage; // variable still sent but not used

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	// Actually took damage?
	if ( damageTaken > 0 || armor > 0 )
	{
		if ( damageTaken > 0 )
		{
			// start the animation
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthDamageTaken");
		}
	}
}
void CHudHealth::MsgFunc_Battery(bf_read &msg)
{
	iNewArmor = msg.ReadShort();
}
void CHudHealth::Paint(void)
{
	BaseClass::Paint();

	float perc = (float)m_iHealth / 100;
	float armorperc = (float)m_iArmor / 100;
	float offs = ((m_IconY + (m_IconY / 2)) + (m_barheight / 2));

	m_background->DrawSelf(0, 0, GetWide(), GetTall(), bgcolor);
	gHUD.DrawProgressBar(m_bar1xpos, offs, m_barwidth, m_barheight, perc, healthbarcolor, CHud::HUDPB_HORIZONTAL_INV);
	PaintValue(m_hNumberFont, m_textX, m_textY, m_iHealth, healthbarcolor);
	gHUD.DrawProgressBar(m_bar2xpos, offs + (m_Icon2Y - (m_barheight/4)), m_barwidth, m_barheight, armorperc, armorcolor, CHud::HUDPB_HORIZONTAL_INV);
	PaintValue(m_hNumberFont, m_textX, offs + (m_Icon2Y - (m_barheight / 4)), m_iArmor,armorcolor);
	m_iconHealth->DrawSelf(m_IconX, m_IconY, healthbarcolor);
	m_iconArmor->DrawSelf(m_Icon2X, m_Icon2Y, armorcolor);
		
}

void CHudHealth::PaintValue(vgui::HFont font, int xpos, int ypos, int value, Color &color)
{
	surface()->DrawSetTextFont(font);
	wchar_t unicode[6];
	V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", value);
	int charWidth = surface()->GetCharacterWidth(font, '0');
	if (value < 100)
	{
		xpos += charWidth;
	}
	if (value < 10)
	{
		xpos += charWidth;
	}

	surface()->DrawSetTextPos(xpos, ypos);
	surface()->DrawSetTextColor(color);
	surface()->DrawUnicodeString(unicode);
}

void CHudHealth::MsgFunc_SuitPickup(bf_read& msg)
{
	int m_irecieved = msg.ReadByte();

	if (m_irecieved)
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HLRSuitPowerup");

}