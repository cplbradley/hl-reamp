#include "cbase.h"
#include <vgui_controls/Panel.h>
#include "view.h"
#include <vgui/IVGui.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "IDemoPanel.h"

//YOU KNOW THE RULES

#include "tier0/memdbgon.h"


using namespace vgui;

class CVguiDemoPanel : public Panel
{
	DECLARE_CLASS_SIMPLE(CVguiDemoPanel, Panel);
public:
	CVguiDemoPanel(VPANEL parent);
	virtual ~CVguiDemoPanel() {};
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);
	virtual bool ShouldDraw() { return true; }

private:
	vgui::HFont		m_hFont;
};

CVguiDemoPanel::CVguiDemoPanel(VPANEL parent) : BaseClass(NULL, "VguiDemoPanel")
{
	SetParent(parent);
	SetVisible(true);
	SetPaintBackgroundEnabled(true);
	SetEnabled(true);

}
void CVguiDemoPanel::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("Default");
	Assert(m_hFont);
}
void CVguiDemoPanel::Paint()
{
	const char* szTextToDraw = "Enter Your Text Here :)";


	int textH = surface()->GetFontTall(m_hFont);
	int textW = g_pMatSystemSurface->DrawTextLen(m_hFont, szTextToDraw);

	int widthBorder = textW * 0.1;
	SetWide(textW + (widthBorder * 2));
	SetTall(textH * 3);

	SetPos(ScreenWidth() - GetWide(), GetTall());

	g_pMatSystemSurface->DrawColoredText(m_hFont, widthBorder, textH * 2, 255, 255, 255, 255, szTextToDraw);
}



class CVguiDemoParent : public IDemoPanel
{
private:
	CVguiDemoPanel* demo;
public:
	CVguiDemoParent(void)
	{
		demo = NULL;
	}

	void Create(vgui::VPANEL parent)
	{
		demo = new CVguiDemoPanel(parent);
	}

	void Destroy(void)
	{
		if (demo)
		{
			demo->SetParent((vgui::Panel*)NULL);
			delete demo;
			demo = NULL;
		}
	}
};

static CVguiDemoParent g_demopanel;
IDemoPanel* demopanel = (IDemoPanel*)&g_demopanel;