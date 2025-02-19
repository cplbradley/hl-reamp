//======= Maestra Fenix, 2017 ==================================================//
//
// Purpose: Map load background panel
//
//==============================================================================//

#include "cbase.h"
#include "mapload_background.h"
#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>
#include "ienginevgui.h"
#include "GameUI/IGameUI.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define NUM_OF_TIPS 17

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMapLoadBG::CMapLoadBG(char const *panelName) : EditablePanel(NULL, panelName)
{
	VPANEL toolParent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
	SetParent(toolParent);
	int iWide, iTall;
	surface()->GetScreenSize(iWide, iTall);
	SetSize(iWide, iTall);

	//Fenix: We load a RES file rather than create the element here for taking advantage of the "F" parameter for wide and tall
	//Is the sole thing that makes fill the background to the entire screen regardless of the texture size
	//Congratulations to Valve for once again give options to only one side and not both
	LoadControlSettings("resource/loadingdialogbackground.res");

	m_pBackground = FindControl<ImagePanel>("LoadingImage", true);
	m_pBackground->SetShouldScaleImage(true);

	m_pLoadingTip = new vgui::Label(this, "LoadTips", "");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMapLoadBG::~CMapLoadBG()
{
	// None
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMapLoadBG::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	IScheme* pClientScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetScheme("ClientScheme"));

	if (m_pLoadingTip)
	{
		m_pLoadingTip->SetFont(pClientScheme->GetFont("Default", true));
		m_pLoadingTip->SetContentAlignment(vgui::Label::a_center);
		m_pLoadingTip->SizeToContents();
		m_pLoadingTip->SetPos(ScreenWidth() / 2 - m_pLoadingTip->GetWide() / 2, ScreenHeight() * 0.75f);
	}

	
}

void CMapLoadBG::OnMessage(const KeyValues* params, VPANEL fromPanel)
{
	if (m_pLoadingTip)
	{
		char tipText[256];

		Q_snprintf(tipText, sizeof(tipText), "#HLR_Mapload_Tips_%i", RandomInt(1, NUM_OF_TIPS));

		m_pLoadingTip->SetText(g_pVGuiLocalize->FindAsUTF8(tipText));

		m_pLoadingTip->SizeToContents();
		m_pLoadingTip->SetPos(ScreenWidth() / 2 - m_pLoadingTip->GetWide() / 2, ScreenHeight() * 0.75f);

	}

	SetSize(ScreenWidth(), ScreenHeight());

	BaseClass::OnMessage(params, fromPanel);
}

//-----------------------------------------------------------------------------
// Purpose: Sets a new background on demand
//-----------------------------------------------------------------------------
void CMapLoadBG::SetNewBackgroundImage(char const *imageName)
{
	m_pBackground->SetImage(imageName);
}