#include "cbase.h"
#include "cdll_client_int.h"
#include "ienginevgui.h"

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
#include <vgui_controls/Slider.h>
#include <vgui_controls/QueryBox.h>

#include "tier0/memdbgon.h"

using namespace vgui;

class CMasochistModeQBox : public vgui::QueryBox
{
public:
	DECLARE_CLASS_SIMPLE(CMasochistModeQBox, vgui::QueryBox);
	

	CMasochistModeQBox(vgui::Panel* parent);
	virtual void OnCommand(const char* command);
	
};

CMasochistModeQBox::CMasochistModeQBox(vgui::Panel* parent) : BaseClass("MASOCHIST MODE", "The game is locked on the hardest difficulty. If you die, your save folder is deleted. Are you sure?", parent)
{
	
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
	LoadControlSettings("resource/ui/MasochistModeDialog.res");
	SetOKButtonText("Let's Go!");
	SetCancelButtonText("No Way!");
	SetBounds(0, 0, 256, 128);
}
/*void CMasochistModeQBox::OnCancelButtonPressed()
{
	//Close();
	Warning("fuck me this sucks\n");
}
void CMasochistModeQBox::OnOkButtonPressed()
{
	Warning("fuck me this sucks\n");
	engine->ClientCmd_Unrestricted("map introtest_new\n");
	engine->ClientCmd_Unrestricted("startmasochistmode\n");
}*/

void CMasochistModeQBox::OnCommand(const char* command)
{
	if (!stricmp(command, "Cancel"))
	{
		Close();
	}
	else if (!stricmp(command, "OK"))
	{
		Close();
		engine->ClientCmd_Unrestricted("map introtest_new\n");
		engine->ClientCmd_Unrestricted("g_masochist_mode 1\n");
	}
	else
		return;
}

static vgui::DHANDLE<CMasochistModeQBox> g_hMasochistQBox;
CON_COMMAND(OpenMasochistModeDialog, "")
{
	if (!g_hMasochistQBox.Get())
	{
		vgui::VPANEL parent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
		if (parent == NULL)
		{
			Assert(0);
			return;
		}

		g_hMasochistQBox.Set(new CMasochistModeQBox(NULL));
	}

	auto* pPanel = g_hMasochistQBox.Get();


	int x, y, w, h;
	vgui::surface()->GetWorkspaceBounds(x, y, w, h);

	int mw = pPanel->GetWide();
	int mh = pPanel->GetTall();
	pPanel->SetPos(x + w / 2 - mw / 2, y + h / 2 - mh / 2);

	pPanel->Activate();
}