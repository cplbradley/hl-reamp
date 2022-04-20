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

	virtual void Paint(void);

private:
	// old variables
	int		m_iHealth;
	int		m_iArmor;
	int		iNewArmor;
	CHudTexture *m_iconHealth;
	CHudTexture *m_healthbar;
	CHudTexture *blank;
	CHudTexture *m_iconArmor;
	CHudTexture *m_backgroundframe;
	CHudTexture *m_background;
	CHudTexture *m_skull;
	int		m_bitsDamage;
	Color healthbarcolor;
	Color armorcolor;
	Color bgcolor;
	CHudNumericDisplay *nHealth;

	int m_iRes;
	char* tex[4];


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
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );

	tex[0] = "hud_healthbar_background4x3";
	tex[1] = "hud_healthbar_background16x9";
	tex[2] = "hud_healthbar_background16x10";
	tex[3] = "hud_healthbar_background5x4";
	
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

	m_iconHealth = gHUD.GetIcon("hud_healthbar_health_icon");
	m_healthbar = gHUD.GetIcon("hud_healthbar");
	m_iconArmor = gHUD.GetIcon("hud_healthbar_armor_icon");
	blank = gHUD.GetIcon("empty");
	SetPaintBackgroundEnabled(false);

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
	m_iconHealth = gHUD.GetIcon("hud_health_icon");
	m_healthbar = gHUD.GetIcon("hud_healthbar");
	m_background = gHUD.GetIcon("hud_health_background");
	SetVisible(true);

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::OnThink()
{
	int newHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		// Never below zero
		newHealth = MAX(local->GetHealth(), 0);
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
		bgcolor = Color(255, 255, 255, 255);
	}
	else if ( m_iHealth >= 30 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
		healthbarcolor = Color(120, 255, 0, 200);
		bgcolor = Color(255, 255, 255, 255);
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
		armorcolor = Color(0, 210, 255, 200);
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

	if (m_healthbar)
	{
		float perc = (float)m_iHealth / 100;
		float armorperc = (float)m_iArmor / 100;
		float offs = ((m_IconY + (m_IconY / 2)) + (m_barheight / 2));
		//gHUD.DrawIconProgressBarExt(m_IconX + 32, m_IconY, 96,32, m_healthbar, blank, 1.0f - perc, healthbarcolor, CHud::HUDPB_HORIZONTAL);

		//int xres = XRES(128);
		//int yres = YRES(64);

		/*vgui::Panel *pParent = g_pClientMode->GetViewport();
		int screenWide = pParent->GetWide();
		int screenTall = pParent->GetTall();

		float ratio = ((float)screenWide) / ((float)screenTall);
		if (ratio == 1.25)
			m_iRes = 3;
		else if (ratio >= 1.5 &&  ratio <= 1.7)
		{
			m_iRes = 2;
		}
		else if (ratio >= 1.75 && ratio <= 1.8)
		{
			m_iRes = 1;
		}
		else
		{
			m_iRes = 0;
		}

		m_backgroundframe = gHUD.GetIcon(tex[m_iRes]);*/

		//m_backgroundframe->DrawSelf(0, 0, xres, yres, bgcolor);
		m_background->DrawSelf(0, 0, GetWide(), GetTall(), bgcolor);
		gHUD.DrawProgressBar(m_bar1xpos, offs, m_barwidth, m_barheight, perc, healthbarcolor, CHud::HUDPB_HORIZONTAL_INV);
		PaintValue(m_hNumberFont, m_textX, m_textY, m_iHealth, healthbarcolor);
		gHUD.DrawProgressBar(m_bar2xpos, offs + (m_Icon2Y - (m_barheight/4)), m_barwidth, m_barheight, armorperc, armorcolor, CHud::HUDPB_HORIZONTAL_INV);
		PaintValue(m_hNumberFont, m_textX, offs + (m_Icon2Y - (m_barheight / 4)), m_iArmor,armorcolor);
		m_iconHealth->DrawSelf(m_IconX, m_IconY, healthbarcolor);
		m_iconArmor->DrawSelf(m_Icon2X, m_Icon2Y, armorcolor);


		
		//m_backgroundframe->DrawSelfCropped(0, 0, 0, 0, m_backgroundframe->Width(), m_backgroundframe->Height(), Color(255, 0, 0, 200));
		
	}
	else
		DevMsg("healthbar doesn't exist what the shit is happening\n");
	
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