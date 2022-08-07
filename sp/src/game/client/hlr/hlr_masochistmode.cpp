#include "cbase.h"
/*#include "cdll_client_int.h"
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
#include "vgui_controls/QueryBox.h"

#include "tier0/memdbgon.h"

using namespace vgui;

class CMasochistModePrompt : public vgui::QueryBox
{
public:
	DECLARE_CLASS_SIMPLE(CMasochistModePrompt, vgui::QueryBox);
	CMasochistModePrompt(const char *title, const char *info, Panel *parent) : BaseClass(title, info, parent)
	{
	}
	~CMasochistModePrompt();
	virtual void Activate();
	void DoModal(Frame* pFrameOver)
	{
		BaseClass::DoModal(pFrameOver);
		vgui::surface()->RestrictPaintToSinglePanel(GetVPanel());
	}
	void OnKeyCodePressed(KeyCode code)
	{
		// ESC cancels
		if (code == KEY_ESCAPE)
		{
			Close();
		}
		else
		{
			BaseClass::OnKeyCodePressed(code);
		}
	}
	virtual void OnClose()
	{
		BaseClass::OnClose();
		vgui::surface()->RestrictPaintToSinglePanel(NULL);
	}
};
static vgui::DHANDLE<CMasochistModePrompt> g_hMasochistPrompt;
CON_COMMAND(OpenMasochistModePrompt, "OpenMasochistModePrompt")
{
	/*if (!g_hMasochistPrompt.Get())
	{

		if (parent == NULL)
		{
			Assert(0);
			return;
		}

		g_hMasochistPrompt.Set(new CMasochistModePrompt(parent));
	}

	auto* pPanel = g_hMasochistPrompt.Get();


	int x, y, w, h;
	vgui::surface()->GetWorkspaceBounds(x, y, w, h);

	int mw = pPanel->GetWide();
	int mh = pPanel->GetTall();
	pPanel->SetPos(x + w / 2 - mw / 2, y + h / 2 - mh / 2);

	pPanel->Activate();*/
	/*vgui::VPANEL parent = enginevgui->GetPanel(PANEL_GAMEDLL);
	if (!g_hMasochistPrompt.Get())
	{

		if (parent == NULL)
		{
			Assert(0);
			return;
		}
	}
	QueryBox *box = new CMasochistModePrompt("Start Masochist Mode", "#GameUI_QuitConfirmationText", box);
	box->SetOKButtonText("Start");
	box->SetOKCommand(new KeyValues("Command", "command", "StartMasochistMode"));
	box->SetCancelCommand(new KeyValues("Command", "command", "ReleaseModalWindow"));
	box->AddActionSignalTarget(box);
	box->DoModal();
}

CON_COMMAND(StartMasochistMode, "StartMasochistMode")
{
	engine->ClientCmd("g_masochist_mode 1\n");
	engine->ClientCmd("map introtest_tex\n");
}
/*CMasochistModePrompt::CMasochistModePrompt(VPANEL parent) : BaseClass(nullptr, "MasochistModePrompt")
{
	SetParent(parent);
	SetBounds(0, 0, 420, 350);
	SetDeleteSelfOnClose(true);
	SetSizeable(false);

	SetTitle("idk", false);
	SetOKButtonVisible(true);
	SetCancelButtonVisible(true);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));



	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

}
CMasochistModePrompt::~CMasochistModePrompt()
{

}

void CMasochistModePrompt::Activate()
{
	BaseClass::Activate();
}*/