#include "cbase.h" 
#include "hud.h" 
#include "hudelement.h"
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "vgui\ILocalize.h"
#include <vgui/IVGui.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/AnimationController.h>
#include <KeyValues.h>

#include "tier0/memdbgon.h" 

using namespace vgui;

class CHudSpecialMessage : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudSpecialMessage, Panel);
public:

	CHudSpecialMessage(const char* pElementName);

	void MsgFunc_SpecialMessage(bf_read& msg);

	virtual void LevelInit();

	virtual void Init();
	virtual void Paint();
	virtual void Reset();
	virtual void OnThink();

	virtual bool ShouldDraw() { return true; }

	bool SetText(const char* text);

	void Display();
	void Hide();

private:
	Color cTextClr;
	Color cGlowClr;

	int iWidth;
	int iHeight;

	int iTextXPos;
	int iTextYPos;

	bool m_bShouldDraw;

	wchar_t wchText[64];
	char chText[64];

	float flVerticalPos;

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudHugeText");
	CPanelAnimationVar(vgui::HFont, m_hGlowFont, "GlowFont", "HudHugeTextGlow");
	CPanelAnimationVarAliasType(float, flLineGap, "LineGap", "4", "proportional_float");
	CPanelAnimationVar(float, m_flSizeLerp, "WidthLerp", "0");
	CPanelAnimationVar(float, m_flGlowAlpha, "GlowAlpha", "0");
	CPanelAnimationVar(float, m_flTextAlpha, "TextAlpha", "0");

};

DECLARE_HUDELEMENT(CHudSpecialMessage);
DECLARE_HUD_MESSAGE(CHudSpecialMessage, SpecialMessage);

CHudSpecialMessage::CHudSpecialMessage(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudSpecialMessage")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(true);
	SetEnabled(true);
	SetActive(true);
	SetPaintBackgroundEnabled(true);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
	Init();
}

void CHudSpecialMessage::Init()
{
	HOOK_HUD_MESSAGE(CHudSpecialMessage, SpecialMessage);
	SetSize(0, 0);
	m_flSizeLerp = 0;
	m_flGlowAlpha = 0;
	m_flTextAlpha = 0;
}

void CHudSpecialMessage::LevelInit()
{
	CHudElement::LevelInit();
	Reset();
}
void CHudSpecialMessage::Reset()
{
	Init();
}

void CHudSpecialMessage::MsgFunc_SpecialMessage(bf_read& msg)
{
	char szString[256];
	msg.ReadString(szString, sizeof(szString));
	flVerticalPos = msg.ReadFloat();
	SetText(szString);
	Display();
}

void CHudSpecialMessage::Display()
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowWaveCounter");
}

void CHudSpecialMessage::OnThink()
{
	int textwidth = g_pMatSystemSurface->DrawTextLen(m_hTextFont, chText);
	iWidth = (textwidth * 2) * m_flSizeLerp;
	int textheight = surface()->GetFontTall(m_hTextFont);
	iHeight = textheight * 1.5;
	SetSize(iWidth, iHeight);
	int xPos = (ScreenWidth() * 0.5) - (GetWide() * 0.5);
	int yPos = (ScreenHeight() * flVerticalPos) - (GetTall() * 0.5);
	SetPos(xPos, yPos);
	cGlowClr = gHUD.GetDefaultColor();
	cTextClr = gHUD.GetDefaultColor();
	cGlowClr[3] = m_flGlowAlpha;
	cTextClr[3] = m_flTextAlpha;

	iTextXPos = (GetWide() * 0.5) - (textwidth * 0.5);
	iTextYPos = (GetTall() * 0.5) - (textheight * 0.5);
}

void CHudSpecialMessage::Paint()
{
	wchar_t wchText[64];

	g_pVGuiLocalize->ConvertANSIToUnicode(chText, wchText, sizeof(wchText));

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(cTextClr);
	surface()->DrawSetTextPos(iTextXPos, iTextYPos);
	surface()->DrawUnicodeString(wchText);

	surface()->DrawSetTextFont(m_hGlowFont);
	surface()->DrawSetTextColor(cGlowClr);
	surface()->DrawSetTextPos(iTextXPos, iTextYPos);
	surface()->DrawUnicodeString(wchText);

	int textlen = g_pMatSystemSurface->DrawTextLen(m_hTextFont, chText);
	int lineY = GetTall() * 0.5;
	surface()->DrawSetColor(cTextClr);
	surface()->DrawLine(flLineGap, lineY, iTextXPos - flLineGap, lineY);
	surface()->DrawLine(iTextXPos + textlen + flLineGap, lineY, GetWide() - flLineGap, lineY);
}

bool CHudSpecialMessage::SetText(const char* text)
{
	if (text == NULL || text[0] == L'\0')
		return false;

	if (text[0] == '#')
	{
		const char* local = g_pVGuiLocalize->FindAsUTF8(text);
		V_strncpy(chText, local, sizeof(chText));
		return true;
	}
	else
	{
		V_strncpy(chText, text, sizeof(chText));
		return true;
	}

	return false;
}