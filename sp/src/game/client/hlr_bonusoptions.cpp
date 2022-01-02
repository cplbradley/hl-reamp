#include "cbase.h"
#include "cdll_client_int.h"
#include "ienginevgui.h"
#include "IBonusOptions.h"

#include "hlr/hlr_shareddefs.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "vgui_controls/PropertyPage.h"
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/cvartogglecheckbutton.h>
#include <vgui_controls/Label.h>

#include "tier0/memdbgon.h"


using namespace vgui;

class CHLRSubBonusOptions : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRSubBonusOptions, vgui::PropertyPage);
public:
	CHLRSubBonusOptions(vgui::Panel *parent);

	virtual void OnResetData();
	virtual void OnApplyChanges();

	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked");
private:
	vgui::CheckButton* m_pGibs;
	vgui::CheckButton* m_pClassic;
	vgui::CheckButton* m_pGuts;
};

class CHLRBonusOptions : public vgui::PropertyDialog
{
public:
	DECLARE_CLASS_SIMPLE(CHLRBonusOptions, vgui::PropertyDialog);

	CHLRBonusOptions(vgui::VPANEL parent);
	~CHLRBonusOptions();
	virtual void Activate();
private:
};

static vgui::DHANDLE<CHLRBonusOptions> g_hBonusOptions;

CON_COMMAND(OpenBonusOptions, "")
{
	if (!g_hBonusOptions.Get())
	{
		vgui::VPANEL parent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
		if (parent == NULL)
		{
			Assert(0);
			return;
		}

		g_hBonusOptions.Set(new CHLRBonusOptions(parent));
	}

	auto* pPanel = g_hBonusOptions.Get();


	int x, y, w, h;
	vgui::surface()->GetWorkspaceBounds(x, y, w, h);

	int mw = pPanel->GetWide();
	int mh = pPanel->GetTall();
	pPanel->SetPos(x + w / 2 - mw / 2, y + h / 2 - mh / 2);

	pPanel->Activate();
}


CHLRBonusOptions::CHLRBonusOptions(VPANEL parent) : BaseClass(nullptr, "BonusOptions")
{
	SetParent(parent);
	SetBounds(0, 0, 420, 350);
	SetDeleteSelfOnClose(true);
	SetSizeable(false);
	SetApplyButtonVisible(true);

	SetTitle("", true);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
	


	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	AddPage(new CHLRSubBonusOptions(this), "Bonus Options");
}
CHLRBonusOptions::~CHLRBonusOptions()
{

}

void CHLRBonusOptions::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(true);
}






CHLRSubBonusOptions::CHLRSubBonusOptions(vgui::Panel *parent) : BaseClass(parent, NULL)
{
	m_pGibs = new CheckButton(this, "GibsButton", "UltraGibs");
	m_pClassic = new CheckButton(this, "ClassicButton", "Retro Mode");
	m_pGuts = new CheckButton(this, "GutsButton", "Guts and Glory");
	LoadControlSettings("resource/ui/bonusoptions.res");
}

void CHLRSubBonusOptions::OnResetData()
{
	ConVarRef gibs("g_ultragibs");
	ConVarRef classic("mat_classic_render");
	ConVarRef guts("g_guts_and_glory");
	if (gibs.GetBool() == true)
		m_pGibs->SetSelected(true);
	else
		m_pGibs->SetSelected(false);

	if (classic.GetBool() == true)
		m_pClassic->SetSelected(true);
	else
		m_pClassic->SetSelected(false);

	if (guts.GetBool() == true)
		m_pGuts->SetSelected(true);
	else
		m_pGuts->SetSelected(false);
}
void CHLRSubBonusOptions::OnApplyChanges()
{
	ConVarRef gibs("g_ultragibs");
	ConVarRef classic("mat_classic_render");
	ConVarRef guts("g_guts_and_glory");
	if (m_pGibs->IsSelected())
		gibs.SetValue(1);
	else
		gibs.SetValue(0);

	if (m_pClassic->IsSelected())
		classic.SetValue(1);
	else
		classic.SetValue(0);

	if (m_pGuts->IsSelected())
		guts.SetValue(1);
	else
		guts.SetValue(0);

	
	DevMsg("changing data\n");

}
void CHLRSubBonusOptions::OnCheckButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}
