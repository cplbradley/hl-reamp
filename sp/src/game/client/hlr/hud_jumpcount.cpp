#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_jumpcount.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"

using namespace vgui;

#include "tier0/memdbgon.h" 
//-----------------------------------------------------------------------------
// Purpose: Shows the hull bar
//-----------------------------------------------------------------------------


DECLARE_HUDELEMENT(CHudJumpCount);
DECLARE_HUD_MESSAGE(CHudJumpCount, JumpCount);

#define HULL_INIT 50 
#define MAX_INIT 1

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudJumpCount::CHudJumpCount(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudJumpCount")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudJumpCount::Init()
{
	HOOK_HUD_MESSAGE(CHudJumpCount, JumpCount);
	m_iJumpCount = 0;
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudJumpCount::Reset(void)
{

	SetBgColor(Color(0, 0, 0, 128));
}

void CHudJumpCount::MsgFunc_JumpCount(bf_read &msg)
{
	m_iJumpCount = msg.ReadShort();
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudJumpCount::OnThink(void)
{
	m_fJumps = (2 - m_iJumpCount);
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudJumpCount::Paint()
{
	// Get bar chunks

	// Get bar chunks

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_fJumps / 2) + 0.5f);

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