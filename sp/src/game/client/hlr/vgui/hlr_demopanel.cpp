#include "cbase.h"
#include "ienginevgui.h"
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
#include "cdll_client_int.h"
#include "vgui_basepanel.h"
#include <vgui_controls/QueryBox.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/MessageDialog.h>
#include <vgui_controls/Button.h>


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class CVGUIDemoPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CVGUIDemoPanel, Frame);
public:
	CVGUIDemoPanel(Panel* parent);

	Button* pEasyButton;
	Button* pNormalButton;
	Button* pHardButton;

	virtual void OnCommand(const char* command);

	void StartDemo(int difficulty);


};

CVGUIDemoPanel::CVGUIDemoPanel(Panel* parent) : BaseClass(parent,"DemoPanel")
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
	SetSize(384, 128);
	SetSizeable(false);
	SetMoveable(false);
	
	MoveToCenterOfScreen();

	pEasyButton = new Button(this, "EasyButton", "#GameUI_SkillEasy", this, "StartEasy");
	pEasyButton->SetButtonActivationType(Button::ACTIVATE_ONPRESSEDANDRELEASED);

	pNormalButton = new Button(this, "MediumButton", "#GameUI_SkillNormal",this,"StartMedium");
	pNormalButton->SetButtonActivationType(Button::ACTIVATE_ONPRESSEDANDRELEASED);

	pHardButton = new Button(this, "HardButton", "#GameUI_SkillHard",this,"StartHard");
	pHardButton->SetButtonActivationType(Button::ACTIVATE_ONPRESSEDANDRELEASED);

	LoadControlSettings("resource/ui/DemoDialogue.res");
}

void CVGUIDemoPanel::OnCommand(const char* command)
{
	if (!stricmp(command, "StartEasy"))
	{
		StartDemo(1);
	}
	else if (!stricmp(command, "StartMedium"))
	{
		StartDemo(2);
	}
	else if (!stricmp(command, "StartHard"))
	{
		StartDemo(3);
	}
	else BaseClass::OnCommand(command);
}

void CVGUIDemoPanel::StartDemo(int difficulty)
{
	const char* cmd = "skill ";
	char txt[32];
	Q_strncpy(txt, cmd, sizeof(txt));
	sprintf(txt + strlen(txt), "%i", difficulty);
	//Msg(txt);
	engine->ClientCmd(txt);
	engine->ClientCmd("progress_enable\nmap demo_no_cubicles");
	BaseClass::OnClose();
}


static DHANDLE<CVGUIDemoPanel> g_hDemoPanel;

CON_COMMAND(OpenDemoStartPanel, "")
{
	if (!g_hDemoPanel.Get())
	{
		vgui::VPANEL parent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
		if(parent == NULL)
		{
			return;
		}

		g_hDemoPanel.Set(new CVGUIDemoPanel(NULL));
	}

	auto* pPanel = g_hDemoPanel.Get();

	int x, y, w, h;
	vgui::surface()->GetWorkspaceBounds(x, y, w, h);

	int mw = pPanel->GetWide();
	int mh = pPanel->GetTall();
	pPanel->SetPos(x + w / 2 - mw / 2, y + h / 2 - mh / 2);

	pPanel->Activate();

}