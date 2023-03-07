#include "cbase.h"
#include "ism30panel.h"
#include <vgui_controls/Panel.h>
#include "view.h"
#include <vgui/IVGui.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IPanel.h>
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include <vgui/ILocalize.h>
#include "filesystem.h"
#include "../common/xbox/xboxstubs.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar g_hide_sm30error("g_hide_sm30error", "1", FCVAR_ARCHIVE);
static ConVar g_hide_dxerror("g_hide_dxerror", "1", FCVAR_ARCHIVE);

class CSM30Error : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CSM30Error, vgui::Panel);
public:
	CSM30Error(vgui::VPANEL parent);
	virtual ~CSM30Error(void);
	virtual void	Paint();
	virtual bool	ShouldDraw(void);
	virtual void	ApplySchemeSettings(vgui::IScheme* pScheme);
	virtual void OnTick(void);
	virtual void ComputeSize();
	int GetInsetPos(const char* string);

private:
	vgui::HFont		m_hFont;

};

CSM30Error::CSM30Error(vgui::VPANEL parent) : BaseClass(NULL, "CSM30Error")
{
	SetParent(parent);	
	SetVisible(true);
	SetPaintBackgroundEnabled(true);

	vgui::ivgui()->AddTickSignal(GetVPanel(), 250);
	
}

CSM30Error::~CSM30Error(void)
{

}
void CSM30Error::OnTick(void)
{
	bool bVisible = ShouldDraw();
	if (IsVisible() != bVisible)
	{
		SetVisible(bVisible);
	}
}
void CSM30Error::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("Default");
	Assert(m_hFont);
}
const char* smdxerror1()
{
	if (!g_hide_dxerror.GetBool())
	{
		return g_pVGuiLocalize->FindAsUTF8("#HLR_DX9_ERROR1");
	}
	else
	{
		return g_pVGuiLocalize->FindAsUTF8("#HLR_SM30_ERROR1");
	}

}

const char* smdxerror2()
{
	if (!g_hide_dxerror.GetBool())
	{
		return g_pVGuiLocalize->FindAsUTF8("#HLR_DX9_ERROR1");
	}
	else
	{
		return g_pVGuiLocalize->FindAsUTF8("#HLR_SM30_ERROR2");
	}
}
bool CSM30Error::ShouldDraw(void)
{
	if (!g_hide_sm30error.GetBool() || !g_hide_dxerror.GetBool())
	{
		if (!IsVisible())
			SetVisible(true);
		return true;
	}
	else
	{
		if (IsVisible())
			SetVisible(false);
		return false;
	}
}

void CSM30Error::ComputeSize(void)
{
	int wide, tall, xpos, ypos;

	int nWidth = 64 + g_pMatSystemSurface->DrawTextLen(m_hFont, smdxerror1());
	int halfWidth = (nWidth * 0.5f);
	xpos = (ScreenWidth() * 0.5f) - halfWidth;
	ypos = 10;
	wide = nWidth;
	tall = 4 + (vgui::surface()->GetFontTall(m_hFont) * 2);

	SetSize(wide, tall);
	SetPos(xpos, ypos);

}
int CSM30Error::GetInsetPos(const char* string)
{
	int length = g_pMatSystemSurface->DrawTextLen(m_hFont, string);
	int gap = GetWide() - length;
	int pos = gap * 0.5f;
	return pos;
}
void CSM30Error::Paint()
{
	if (!ShouldDraw())
		return;

	ComputeSize();

	
	g_pMatSystemSurface->DrawColoredText(m_hFont, GetInsetPos(smdxerror1()), 0, 255, 60, 0, 255, smdxerror1());
	g_pMatSystemSurface->DrawColoredText(m_hFont, GetInsetPos(smdxerror2()), 2 + vgui::surface()->GetFontTall(m_hFont), 255, 60, 0, 255, smdxerror2());

}



class CSM30ErrorPanel : public ISM30ErrorPanel
{
private:
	CSM30Error* fpsPanel;
public:
	CSM30ErrorPanel(void)
	{
		fpsPanel = NULL;
	}

	void Create(vgui::VPANEL parent)
	{
		fpsPanel = new CSM30Error(parent);
	}

	void Destroy(void)
	{
		if (fpsPanel)
		{
			fpsPanel->SetParent((vgui::Panel*)NULL);
			delete fpsPanel;
			fpsPanel = NULL;
		}
	}
};

static CSM30ErrorPanel g_sm30panel;
ISM30ErrorPanel* sm30p = (ISM30ErrorPanel*)&g_sm30panel;