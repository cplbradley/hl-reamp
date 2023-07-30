#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_healthbar.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "vgui\ILocalize.h"
#include <vgui/IVGui.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/AnimationController.h>

using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudHealthBar1);
DECLARE_HUD_MESSAGE(CHudHealthBar1, EnemyHealth1);

#define HULL_INIT 50 
#define MAX_INIT 1


ConVar debug_healthbar("debug_healthbar", "0");
//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudHealthBar1::CHudHealthBar1(const char * pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHealthBar1")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetProportional(true);

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
	bIsAlive = 0;
	drawanim = false;
	bStartAnimDrawn = false;
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudHealthBar1::Reset(void)
{
	m_nHullLow = -1;
	bStartAnimDrawn = false;
	SetBgColor(Color(0, 0, 0, 128));
}

void CHudHealthBar1::MsgFunc_EnemyHealth1(bf_read &msg)
{
	gHealth1 = msg.ReadFloat();
	gMaxHealth1 = msg.ReadFloat();
	bIsAlive = msg.ReadOneBit();
	char szString[256];
	msg.ReadString(szString, sizeof(szString));
	if (debug_healthbar.GetBool()) Msg("string received: %s\n", szString);
	SetBarLabelName(szString);

	animbar = 0.0f;

	if (bIsAlive && !bStartAnimDrawn)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowHealthBar1");
		bStartAnimDrawn = true;
	}
}

bool CHudHealthBar1::SetBarLabelName(const char* text)
{
	if (text == NULL || text[0] == L'\0')
		return false;

	if (text[0] == '#')
	{
		const char* local = g_pVGuiLocalize->FindAsUTF8(text);
		V_strncpy(chName, local, sizeof(chName));
		return true;
	}
	else
	{
		V_strncpy(chName, text, sizeof(chName));
		return true;
	}

	return false;
}
//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar1::OnThink(void)
{
	BaseClass::OnThink();

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
	
	if (animbar >= maxHull)
		animbar = maxHull;

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed heal

	if (newHull == maxHull)
	{
		m_flHull = animbar;
		drawanim = true;
	}
	else
	{
		m_flHull = newHull;
		drawanim = false;
	}

	m_flMaxHull = maxHull;


	Color color1 = Color(0, 255, 0, 255);
	Color color2 = Color(255, 0, 0, 255);
	Vector vecclr1 = Vector(color1[0], color1[1], color1[2]);
	Vector vecclr2 = Vector(color2[0], color2[1], color2[2]);

	float frac = gHealth1 / gMaxHealth1;

	InterpolateVector(frac, vecclr2, vecclr1, vecColor);
	animbar += gMaxHealth1 * 0.006f;
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudHealthBar1::Paint()
{
	// Get bar chunks

// Get bar chunks
	int wide, tall, baseX, baseY, correctX;
	GetPos(baseX, baseY);
	GetSize(wide, tall);
	correctX = GetCorrectPosition();

	SetPos(correctX, baseY);

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flHull / m_flMaxHull) + 0.5f );

	// Draw the suit power bar
	Color barColor = Color(vecColor[0], vecColor[1], vecColor[2], 255);
	surface()->DrawSetColor(barColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(barColor[0], barColor[1], barColor[2], m_iHullDisabledAlpha));
	if (!drawanim)
	{
		for (int i = enabledChunks; i < chunkCount; i++)
		{
			surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
			xpos += (m_flBarChunkWidth + m_flBarChunkGap);
		}
	}
	// Draw our name



	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(barColor);
	int centerX = GetWide() * 0.5;
	int xOffset = centerX - (g_pMatSystemSurface->DrawTextLen(m_hTextFont, chName) * 0.5);
	//int height = surface()->GetFontTall(m_hTextFont);
	int xInset = g_pMatSystemSurface->DrawTextLen(m_hTextFont, "_");
	surface()->DrawSetTextPos(xOffset - xInset, 0);

	wchar_t tempstring[256];
	g_pVGuiLocalize->ConvertANSIToUnicode(chName, tempstring, sizeof(tempstring));

	surface()->DrawUnicodeString(tempstring);
	//surface()->DrawPrintText(L"HULL", wcslen(L"HULL"));
}


int CHudHealthBar1::GetCorrectPosition()
{
	int screencenter = ScreenWidth() / 2;
	CHudHealthBar2* hb2 = (CHudHealthBar2*)GET_HUDELEMENT(CHudHealthBar2);
	CHudHealthBar3* hb3 = (CHudHealthBar3*)GET_HUDELEMENT(CHudHealthBar3);
	bool b1 = IsVisible();
	bool b2 = hb2->IsVisible();
	bool b3 = hb3->IsVisible();

	if (b1 && b2 && !b3)
		return screencenter - GetWide();

	if (b1 && b2 && b3)
		return screencenter - GetWide() - (hb2->GetWide()/2);
	if (b1 && !b2 && b3)
		return screencenter - hb3->GetWide();

	return screencenter - (GetWide() / 2);

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
	bIsAlive = 0;
	drawanim = false;
	bStartAnimDrawn = false;
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudHealthBar2::Reset(void)
{
	bStartAnimDrawn = false;
	m_nHullLow = -1;
	SetBgColor(Color(0, 0, 0, 128));
}

void CHudHealthBar2::MsgFunc_EnemyHealth2(bf_read &msg)
{
	gHealth2 = msg.ReadFloat();
	gMaxHealth2 = msg.ReadFloat();
	bIsAlive = msg.ReadOneBit();
	char szString[256];
	msg.ReadString(szString, sizeof(szString));
	if(debug_healthbar.GetBool()) Msg("string received: %s\n", szString);
	SetBarLabelName(szString);
	animbar = 0.0f;
	if (bIsAlive && !bStartAnimDrawn)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowHealthBar2");
		bStartAnimDrawn = true;
	}
}

bool CHudHealthBar2::SetBarLabelName(const char* text)
{
	if (text == NULL || text[0] == L'\0')
		return false;

	if (text[0] == '#')
	{
		const char* local = g_pVGuiLocalize->FindAsUTF8(text);
		V_strncpy(chName, local, sizeof(chName));
		return true;
	}
	else
	{
		V_strncpy(chName, text, sizeof(chName));
		return true;
	}

	return false;
}
//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar2::OnThink(void)
{
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

	if (animbar >= maxHull)
		animbar = maxHull;

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (newHull == maxHull)
	{
		m_flHull = animbar;
		drawanim = true;
	}
	else
	{
		m_flHull = newHull;
		drawanim = false;
	}

	m_flMaxHull = maxHull;

	Color color1 = Color(0, 255, 0, 255);
	Color color2 = Color(255, 0, 0, 255);
	Vector vecclr1 = Vector(color1[0], color1[1], color1[2]);
	Vector vecclr2 = Vector(color2[0], color2[1], color2[2]);

	float frac = gHealth2 / gMaxHealth2;

	InterpolateVector(frac, vecclr2, vecclr1, vecColor);
	animbar += gMaxHealth2 * 0.006f;

}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudHealthBar2::Paint()
{
	// Get bar chunks
	int wide, tall, baseX, baseY, correctX;
	GetPos(baseX, baseY);
	GetSize(wide, tall);
	correctX = GetCorrectPosition();

	SetPos(correctX, baseY);
	Color barColor = Color(vecColor[0], vecColor[1], vecColor[2], 255);
	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flHull / m_flMaxHull) + 0.5f);

	// Draw the suit power bar
	surface()->DrawSetColor(barColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(barColor[0], barColor[1], barColor[2], m_iHullDisabledAlpha));
	if (!drawanim)
	{
		for (int i = enabledChunks; i < chunkCount; i++)
		{
			surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
			xpos += (m_flBarChunkWidth + m_flBarChunkGap);
		}
	}
	// Draw our name

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(barColor);
	int centerX = GetWide() * 0.5;
	int xOffset = centerX - (g_pMatSystemSurface->DrawTextLen(m_hTextFont, chName) * 0.5);
	//int height = surface()->GetFontTall(m_hTextFont);
	int xInset = g_pMatSystemSurface->DrawTextLen(m_hTextFont, "_");
	surface()->DrawSetTextPos(xOffset - xInset, 0);

	wchar_t tempstring[256];
	g_pVGuiLocalize->ConvertANSIToUnicode(chName, tempstring, sizeof(tempstring));

	surface()->DrawUnicodeString(tempstring);
}

int CHudHealthBar2::GetCorrectPosition()
{
	int screencenter = ScreenWidth() / 2;
	CHudHealthBar1* hb1 = (CHudHealthBar1*)GET_HUDELEMENT(CHudHealthBar1);
	CHudHealthBar3* hb3 = (CHudHealthBar3*)GET_HUDELEMENT(CHudHealthBar3);
	bool b1 = hb1->IsVisible();
	bool b2 = IsVisible();
	bool b3 = hb3->IsVisible();

	if (b1 && b2 && !b3)
		return screencenter - ((GetWide()/2) - (hb1->GetWide()/2));

	if (b1 && b2 && b3)
		return screencenter - (GetWide() / 2);

	if (!b1 && b2 && !b3)
		return screencenter - (GetWide() / 2);

	if (!b1 && b2 && b3)
		return screencenter - hb3->GetWide();

	return screencenter;

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
	bIsAlive = false;
	bStartAnimDrawn = false;
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudHealthBar3::Reset(void)
{
	bStartAnimDrawn = false;
	m_nHullLow = -1;
	SetBgColor(Color(0, 0, 0, 128));
}

void CHudHealthBar3::MsgFunc_EnemyHealth3(bf_read &msg)
{
	gHealth3 = msg.ReadFloat();
	gMaxHealth3 = msg.ReadFloat();
	bIsAlive = msg.ReadOneBit();
	char szString[256];
	msg.ReadString(szString, sizeof(szString));
	if (debug_healthbar.GetBool()) Msg("string received: %s\n", szString);
	SetBarLabelName(szString);
	animbar = 0.0f;

	if (bIsAlive && !bStartAnimDrawn)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowHealthBar3");
		bStartAnimDrawn = true;
	}
}
bool CHudHealthBar3::SetBarLabelName(const char* text)
{
	if (text == NULL || text[0] == L'\0')
		return false;

	if (text[0] == '#')
	{
		const char* local = g_pVGuiLocalize->FindAsUTF8(text);
		V_strncpy(chName, local, sizeof(chName));
		return true;
	}
	else
	{
		V_strncpy(chName, text, sizeof(chName));
		DevMsg("name = %s\n", STRING(chName));
		return true;
	}

	return false;
}
//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthBar3::OnThink(void)
{
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

	if (animbar >= maxHull)
		animbar = maxHull;

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (newHull == maxHull)
	{
		m_flHull = animbar;
		drawanim = true;
	}
	else
	{
		m_flHull = newHull;
		drawanim = false;
	}

	m_flMaxHull = maxHull;
	Color color1 = Color(0, 255, 0, 255);
	Color color2 = Color(255, 0, 0, 255);
	Vector vecclr1 = Vector(color1[0], color1[1], color1[2]);
	Vector vecclr2 = Vector(color2[0], color2[1], color2[2]);

	float frac = gHealth3 / gMaxHealth3;

	InterpolateVector(frac, vecclr2, vecclr1, vecColor);
	animbar += gMaxHealth3 * 0.006f;
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudHealthBar3::Paint()
{
	int wide, tall, baseX, baseY, correctX;
	GetPos(baseX, baseY);
	GetSize(wide, tall);
	correctX = GetCorrectPosition();

	SetPos(correctX, baseY);
	// Get bar chunks

	// Get bar chunks

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flHull / m_flMaxHull) + 0.5f);
	Color barColor = Color(vecColor[0], vecColor[1], vecColor[2], 255);
	// Draw the suit power bar
	surface()->DrawSetColor(barColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(barColor[0], barColor[1], barColor[2], m_iHullDisabledAlpha));

	if (!drawanim)
	{
		for (int i = enabledChunks; i < chunkCount; i++)
		{
			surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
			xpos += (m_flBarChunkWidth + m_flBarChunkGap);
		}
	}
	// Draw our name
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(barColor);
	int centerX = GetWide() * 0.5;
	int xOffset = centerX - (g_pMatSystemSurface->DrawTextLen(m_hTextFont, chName) * 0.5);
	//int height = surface()->GetFontTall(m_hTextFont);
	int xInset = g_pMatSystemSurface->DrawTextLen(m_hTextFont, "_");
	surface()->DrawSetTextPos(xOffset - xInset, 0);

	wchar_t tempstring[256];
	g_pVGuiLocalize->ConvertANSIToUnicode(chName, tempstring, sizeof(tempstring));

	surface()->DrawUnicodeString(tempstring);
}

int CHudHealthBar3::GetCorrectPosition()
{
	int screencenter = ScreenWidth() / 2;
	CHudHealthBar1* hb1 = (CHudHealthBar1*)GET_HUDELEMENT(CHudHealthBar1);
	CHudHealthBar2* hb2 = (CHudHealthBar2*)GET_HUDELEMENT(CHudHealthBar2);
	bool b1 = hb1->IsVisible();
	bool b2 = hb2->IsVisible();
	bool b3 = IsVisible();	

	if (!b1 && !b2 && b3)
		return screencenter - (GetWide() / 2);

	if (!b1 && b2 && b3)
		return screencenter - ((GetWide()/2) - (hb2->GetWide()/2));
	if (b1 && !b2 && b3)
		return screencenter;

	return screencenter + (hb2->GetWide()/2);


}


int interpPos(int start, int end, float frac)
{
	return start + static_cast<int>((end - start) * frac);
}
int iNumActive()
{
	CHudHealthBar1* hb1 = (CHudHealthBar1*)GET_HUDELEMENT(CHudHealthBar1);
	CHudHealthBar2* hb2 = (CHudHealthBar2*)GET_HUDELEMENT(CHudHealthBar2);
	CHudHealthBar3* hb3 = (CHudHealthBar3*)GET_HUDELEMENT(CHudHealthBar3);
	return hb1->bIsAlive + hb2->bIsAlive + hb3->bIsAlive;
}