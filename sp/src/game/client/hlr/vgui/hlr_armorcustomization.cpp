#include "cbase.h"
#include "cdll_client_int.h"
#include "ienginevgui.h"

#include "hlr/hlr_shareddefs.h"
#include "iclientmode.h"

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
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ComboBox.h>
#include "basemodelpanel.h"
#include "basemodel_panel.h"
#include "animation.h"
#include "bone_setup.h"
#include "tier0/memdbgon.h"
#include "view.h"
#include "viewrender.h"
#include "view_scene.h"
#include "studio.h"



using namespace vgui;



class CHLRArmorPanel : public CBaseModelPanel
{
	DECLARE_CLASS_SIMPLE(CHLRArmorPanel, CBaseModelPanel);
public:
	CHLRArmorPanel(vgui::Panel* parent);
	virtual void Paint();
	void SetArmorGroup(const char* szGroupName, int nValue);
	void Update();
	void InitModel();

	MDLHandle_t handle;
};

CHLRArmorPanel::CHLRArmorPanel(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	SetParent(parent);
	SetVisible(true);
	SetEnabled(true);
	m_bAllowRotation = true;
	SetProportional(true);
	SetPaintBackgroundEnabled(false);

}

void CHLRArmorPanel::InitModel()
{
	handle = mdlcache->FindMDL("models/player/mark6.mdl");
	mdlcache->PreloadModel(handle);
	SetMDL(handle);
	studiohdr_t* hdr = m_RootMDL.m_MDL.GetStudioHdr();
	CStudioHdr studiohdr(hdr, mdlcache);
	int anim = LookupSequence(&studiohdr, "armor_custom_idle");
	SetSequence(anim);
	SetModelAnglesAndPosition(QAngle(0, -180, 0), vec3_origin);
	SetCameraPositionAndAngles(Vector(-250, 0, 25), vec3_angle);
}

void CHLRArmorPanel::Paint()
{
	BaseClass::Paint();
}

void CHLRArmorPanel::SetArmorGroup(const char* szGroupName, int nValue)
{
	studiohdr_t* pStudioHDR = m_RootMDL.m_MDL.GetStudioHdr();

	CStudioHdr studiohdr(pStudioHDR, mdlcache);
	int bodygroup = ::FindBodygroupByName(&studiohdr, szGroupName);

	SetBodygroup(handle, bodygroup, nValue);
}

class CHLRArmorCustomization : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRArmorCustomization, PropertyPage);

public:
	CHLRArmorCustomization(Panel* parent);
	virtual void Paint();
	virtual void OnApplyChanges();
	virtual void OnDataChanged();
	virtual void OnResetData();
	void UpdateCombos();
	void SetupHelmets();
	void SetupChest();
	void SetupLegs();
	void SetupArms();
	void SetupCodpiece();
	void UpdateEditorBodygroups();
	void UpdatePlayerBodygroups();
	virtual void OnPageShow();
	CHLRArmorPanel* armorPanel;
	ComboBox* helmetCombo;
	ComboBox* chestcombo;
	ComboBox* bootscombo;
	ComboBox* upperarmscombo;
	ComboBox* codpiececombo;
	ComboBox* forearmscombo;
	ComboBox* thighcombo;

	Label* helmetlabel;
	Label* chestlabel;
	Label* bootslabel;
	Label* upperarmlabel;
	Label* forearmlabel;
	Label* thighlabel;
	Label* codpiecelabel;
};


CHLRArmorCustomization::CHLRArmorCustomization(Panel* parent) : BaseClass(parent, NULL)
{
	SetParent(parent);
	armorPanel = new CHLRArmorPanel(this);
	armorPanel->SetVisible(true);
	armorPanel->SetEnabled(true);
	armorPanel->SetSize(parent->GetTall()/1.5, parent->GetTall());
	armorPanel->SetPos((parent->GetWide() / 2) - (armorPanel->GetWide() / 2), 0);
	armorPanel->InitModel();
	

	SetupHelmets();
	SetupLegs();
	SetupChest();
	SetupCodpiece();
	SetupArms();

	LoadControlSettings("resource/ui/bonusoptionsarmor.res");
}
void CHLRArmorCustomization::SetupHelmets()
{
	helmetCombo = new ComboBox(this, "HelmetCombo", 3, false);
	helmetCombo->SetProportional(true);
	helmetCombo->AddItem("#HLR_Mark6", NULL);
	helmetCombo->AddItem("#HLR_CQB", NULL);
	helmetCombo->AddItem("#HLR_Cosmoneer", NULL);

	helmetlabel = new Label(this, "HelmetLabel", "#HLR_Helmet");
	helmetlabel->SetProportional(true);
}
void CHLRArmorCustomization::SetupChest()
{
	chestcombo = new ComboBox(this, "ChestCombo", 3, false);
	chestcombo->SetProportional(true);
	chestcombo->AddItem("#HLR_Mark6", NULL);
	chestcombo->AddItem("#HLR_CQB", NULL);
	chestcombo->AddItem("#HLR_Cosmoneer", NULL);

	chestlabel = new Label(this, "ChestLabel", "#HLR_Chest");
	chestlabel->SetProportional(true);
}
void CHLRArmorCustomization::SetupLegs()
{
	bootscombo = new ComboBox(this, "BootsCombo", 3, false);
	bootscombo->SetProportional(true);
	bootscombo->AddItem("#HLR_Mark6", NULL);
	bootscombo->AddItem("#HLR_CQB", NULL);
	bootscombo->AddItem("#HLR_Cosmoneer", NULL);

	bootslabel = new Label(this, "BootsLabel", "#HLR_Boots");
	bootslabel->SetProportional(true);

	thighcombo = new ComboBox(this, "ThighCombo", 3, false);
	thighcombo->SetProportional(true);
	thighcombo->AddItem("#HLR_Mark6", NULL);
	thighcombo->AddItem("#HLR_CQB", NULL);
	thighcombo->AddItem("#HLR_Cosmoneer", NULL);

	thighlabel = new Label(this, "ThighLabel", "#HLR_Thighs");
	thighlabel->SetProportional(true);
}
void CHLRArmorCustomization::SetupArms()
{
	upperarmscombo = new ComboBox(this, "UpperArmsCombo", 3, false);
	upperarmscombo->SetProportional(true);
	upperarmscombo->AddItem("#HLR_Mark6", NULL);
	upperarmscombo->AddItem("#HLR_CQB", NULL);
	upperarmscombo->AddItem("#HLR_Cosmoneer", NULL);

	upperarmlabel = new Label(this, "UpperarmLabel", "#HLR_Upperarm");
	upperarmlabel->SetProportional(true);

	forearmscombo = new ComboBox(this, "ForearmsCombo", 3, false);
	forearmscombo->SetProportional(true);
	forearmscombo->AddItem("#HLR_Mark6", NULL);
	forearmscombo->AddItem("#HLR_CQB", NULL);
	forearmscombo->AddItem("#HLR_Cosmoneer", NULL);

	forearmlabel = new Label(this, "ForearmLabel", "#HLR_Forearm");
	forearmlabel->SetProportional(true);
}
void CHLRArmorCustomization::SetupCodpiece()
{
	codpiececombo = new ComboBox(this, "CodpieceCombo", 3, false);
	codpiececombo->SetProportional(true);
	codpiececombo->AddItem("#HLR_Mark6", NULL);
	codpiececombo->AddItem("#HLR_CQB", NULL);
	codpiececombo->AddItem("#HLR_Cosmoneer", NULL);

	codpiecelabel = new Label(this, "CodpieceLabel", "#HLR_Codpiece");
	codpiecelabel->SetProportional(true);
}
void CHLRArmorCustomization::OnPageShow()
{
	UpdateEditorBodygroups();
}
void CHLRArmorCustomization::OnResetData()
{
	//C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	UpdateCombos();
}

void CHLRArmorCustomization::UpdateCombos()
{
	ConVarRef helmet("cl_armor_helmet");
	ConVarRef chest("cl_armor_chest");
	ConVarRef boots("cl_armor_boots");
	ConVarRef codpiece("cl_armor_codpiece");
	ConVarRef thighs("cl_armor_thighs");
	ConVarRef forearm("cl_armor_forearm");
	ConVarRef upperarm("cl_armor_upperarm");
	helmetCombo->ActivateItem(helmet.GetInt());
	chestcombo->ActivateItem(chest.GetInt());
	bootscombo->ActivateItem(boots.GetInt());
	codpiececombo->ActivateItem(codpiece.GetInt());
	thighcombo->ActivateItem(thighs.GetInt());
	forearmscombo->ActivateItem(thighs.GetInt());
	upperarmscombo->ActivateItem(thighs.GetInt());
}
void CHLRArmorCustomization::OnApplyChanges()
{
	UpdatePlayerBodygroups();
}

void CHLRArmorCustomization::UpdateEditorBodygroups()
{
	armorPanel->SetArmorGroup("helmet", helmetCombo->GetActiveItem());
	armorPanel->SetArmorGroup("chest", chestcombo->GetActiveItem());
	armorPanel->SetArmorGroup("codpiece", codpiececombo->GetActiveItem());
	armorPanel->SetArmorGroup("thighs", thighcombo->GetActiveItem());
	armorPanel->SetArmorGroup("boots", bootscombo->GetActiveItem());
	armorPanel->SetArmorGroup("upperarms", upperarmscombo->GetActiveItem());
	armorPanel->SetArmorGroup("forearms", forearmscombo->GetActiveItem());
}

void CHLRArmorCustomization::UpdatePlayerBodygroups()
{
	UpdateEditorBodygroups();
	ConVarRef helmet("cl_armor_helmet");
	ConVarRef chest("cl_armor_chest");
	ConVarRef boots("cl_armor_boots");
	ConVarRef codpiece("cl_armor_codpiece");
	ConVarRef thighs("cl_armor_thighs");
	ConVarRef forearm("cl_armor_forearm");
	ConVarRef upperarm("cl_armor_upperarm");
	helmet.SetValue(helmetCombo->GetActiveItem());
	chest.SetValue(chestcombo->GetActiveItem());
	boots.SetValue(bootscombo->GetActiveItem());
	codpiece.SetValue(codpiececombo->GetActiveItem());
	thighs.SetValue(thighcombo->GetActiveItem());
	forearm.SetValue(forearmscombo->GetActiveItem());
	upperarm.SetValue(upperarmscombo->GetActiveItem());
	C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	if (pPlayer)
		pPlayer->SetArmorPieces();
}
void CHLRArmorCustomization::OnDataChanged()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
	UpdateEditorBodygroups();
}

void CHLRArmorCustomization::Paint()
{
	BaseClass::Paint();
	UpdateEditorBodygroups();

	if (helmetCombo->IsDropdownVisible())
		OnDataChanged();
}

class CHLRArmorFrame : public PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CHLRArmorFrame, PropertyDialog);
public:
	CHLRArmorFrame(vgui::VPANEL parent);
	virtual void Activate();
	~CHLRArmorFrame();

};

CHLRArmorFrame::CHLRArmorFrame(vgui::VPANEL parent) : BaseClass(nullptr, "")
{
	SetParent(parent);
	SetDeleteSelfOnClose(true);
	SetApplyButtonVisible(true);
	SetBounds(0, 0, ScreenWidth()/2, ScreenHeight()/1.5);
	SetSizeable(false);
	SetMoveable(false);

	SetTitle("", true);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
	AddPage(new CHLRArmorCustomization(this), "#HLR_Armor_Customization");

}
void CHLRArmorFrame::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(true);
}
CHLRArmorFrame::~CHLRArmorFrame()
{
}


static vgui::DHANDLE<CHLRArmorFrame> g_hArmorCustomization;
CON_COMMAND(OpenArmorCustomization, "")
{
	if(!g_hArmorCustomization.Get())
	{
		vgui::VPANEL parent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
		if (!parent)
			return;

		g_hArmorCustomization.Set(new CHLRArmorFrame(parent));
	}

	auto* panel = g_hArmorCustomization.Get();

	int sw = ScreenWidth() / 2;
	int sh = ScreenHeight() / 2;

	int mw = panel->GetWide() / 2;
	int mh = panel->GetTall() / 2;
	panel->SetPos(sw - mw, sh - mh);
	panel->Activate();
}