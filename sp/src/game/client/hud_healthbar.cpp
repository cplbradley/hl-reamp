#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_healthbar.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"

using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudHealthBar1);
DECLARE_HUD_MESSAGE(CHudHealthBar1, EnemyHealth1);

#define HULL_INIT 50 
#define MAX_INIT 1

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudHealthBar1::CHudHealthBar1(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHealthBar1")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar1::Init()
{
	SetVisible(false);
	HOOK_HUD_MESSAGE(CHudHealthBar1, EnemyHealth1);
	m_flHull = HULL_INIT;
	m_flMaxHull = MAX_INIT;
	bIsAlive1 = 0;

	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudHealthBar1::Reset(void)
{

	m_nHullLow = -1;
	SetBgColor(Color(0, 0, 0, 128));
}

void CHudHealthBar1::MsgFunc_EnemyHealth1(bf_read &msg)
{
	gHealth1 = msg.ReadFloat();
	gMaxHealth1 = msg.ReadFloat();
	bIsAlive1 = msg.ReadByte();
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar1::OnThink(void)
{
	if (bIsAlive1)
	{
		SetVisible(true);
	}
	if (!bIsAlive1)
	{
		SetVisible(false);
	}
	float newHull = 0;
	float maxHull = 0;



	// Never below zero 
	newHull = max(gHealth1, 1);
	maxHull = max(gMaxHealth1, MAX_INIT);

	if (newHull <= 0)
	{
		SetActive(false);
		SetVisible(false);
	}
	
	float colormult = 255 * (gHealth1 / gMaxHealth1);
	m_HullColor[0] = 255 - colormult;
	m_HullColor[1] = colormult;

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (newHull == m_flHull)
		return;

	m_flHull = newHull;
	m_flMaxHull = maxHull;
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudHealthBar1::Paint()
{
	// Get bar chunks

// Get bar chunks

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flHull / m_flMaxHull) + 0.5f );

	// Draw the suit power bar
	surface()->DrawSetColor (m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor [0], m_HullColor [1], m_HullColor [2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw our name

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);

	//wchar_t *tempString = vgui::localize()->Find("#Valve_Hud_AUX_POWER");

	//surface()->DrawPrintText(L"HULL", wcslen(L"HULL"));
}
/////////////////////////////////////////////////
//////////////////////////////////////////
///////////////////////////////
//////////////////////
/////////////
///////
//
DECLARE_HUDELEMENT(CHudHealthBar2);
DECLARE_HUD_MESSAGE(CHudHealthBar2, EnemyHealth2);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudHealthBar2::CHudHealthBar2(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHealthBar2")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar2::Init()
{
	SetVisible(false);
	HOOK_HUD_MESSAGE(CHudHealthBar2, EnemyHealth2);
	m_flHull = HULL_INIT;
	m_flMaxHull = MAX_INIT;
	bIsAlive2 = 0;

	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudHealthBar2::Reset(void)
{

	m_nHullLow = -1;
	SetBgColor(Color(0, 0, 0, 128));
}

void CHudHealthBar2::MsgFunc_EnemyHealth2(bf_read &msg)
{
	gHealth2 = msg.ReadFloat();
	gMaxHealth2 = msg.ReadFloat();
	bIsAlive2 = msg.ReadByte();
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar2::OnThink(void)
{
	if (bIsAlive2)
	{
		SetVisible(true);
	}
	if (!bIsAlive2)
	{
		SetVisible(false);
	}
	float newHull = 0;
	float maxHull = 0;



	// Never below zero 
	newHull = max(gHealth2, 1);
	maxHull = max(gMaxHealth2, MAX_INIT);

	if (newHull <= 0)
	{
		SetActive(false);
		SetVisible(false);
	}

	float colormult = 255 * (gHealth2 / gMaxHealth2);
	m_HullColor[0] = 255 - colormult;
	m_HullColor[1] = colormult;

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (newHull == m_flHull)
		return;

	m_flHull = newHull;
	m_flMaxHull = maxHull;
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudHealthBar2::Paint()
{
	// Get bar chunks

	// Get bar chunks

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flHull / m_flMaxHull) + 0.5f);

	// Draw the suit power bar
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw our name

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);

	//wchar_t *tempString = vgui::localize()->Find("#Valve_Hud_AUX_POWER");

	//surface()->DrawPrintText(L"HULL", wcslen(L"HULL"));
}

/////////////////////////////////////////////////
//////////////////////////////////////////
///////////////////////////////
//////////////////////
/////////////
///////
//

DECLARE_HUDELEMENT(CHudHealthBar3);
DECLARE_HUD_MESSAGE(CHudHealthBar3, EnemyHealth3);

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudHealthBar3::CHudHealthBar3(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHealthBar3")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar3::Init()
{
	SetVisible(false);
	HOOK_HUD_MESSAGE(CHudHealthBar3, EnemyHealth3);
	m_flHull = HULL_INIT;
	m_flMaxHull = MAX_INIT;
	bIsAlive3 = 0;

	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudHealthBar3::Reset(void)
{

	m_nHullLow = -1;
	SetBgColor(Color(0, 0, 0, 128));
}

void CHudHealthBar3::MsgFunc_EnemyHealth3(bf_read &msg)
{
	gHealth3 = msg.ReadFloat();
	gMaxHealth3 = msg.ReadFloat();
	bIsAlive3 = msg.ReadByte();
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar3::OnThink(void)
{
	if (bIsAlive3)
	{
		SetVisible(true);
	}
	if (!bIsAlive3)
	{
		SetVisible(false);
	}
	float newHull = 0;
	float maxHull = 0;



	// Never below zero 
	newHull = max(gHealth3, 1);
	maxHull = max(gMaxHealth3, MAX_INIT);

	if (newHull <= 0)
	{
		SetActive(false);
		SetVisible(false);
	}

	float colormult = 255 * (gHealth3 / gMaxHealth3);
	m_HullColor[0] = 255 - colormult;
	m_HullColor[1] = colormult;

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (newHull == m_flHull)
		return;

	m_flHull = newHull;
	m_flMaxHull = maxHull;
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudHealthBar3::Paint()
{
	// Get bar chunks

	// Get bar chunks

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flHull / m_flMaxHull) + 0.5f);

	// Draw the suit power bar
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw our name

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);

	//wchar_t *tempString = vgui::localize()->Find("#Valve_Hud_AUX_POWER");

	//surface()->DrawPrintText(L"HULL", wcslen(L"HULL"));
}