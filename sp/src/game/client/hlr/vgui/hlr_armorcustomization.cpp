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
	m_bStartFramed = true;
	SetProportional(true);
	SetPaintBackgroundEnabled(false);
	m_bRenderToTexture = false;

}


ConVar armorpanel_camx("armorpanel_camx", "-240");
ConVar armorpanel_camy("armorpanel_camy", "0");
ConVar armorpanel_camz("armorpanel_camy", "25");

void CHLRArmorPanel::InitModel()
{
	handle = mdlcache->FindMDL("models/player/mark6.mdl");
	mdlcache->PreloadModel(handle);
	SetMDL(handle);
	studiohdr_t* hdr = m_RootMDL.m_MDL.GetStudioHdr();
	CStudioHdr studiohdr(hdr, mdlcache);
	int anim = LookupSequence(&studiohdr, "armor_custom_idle");
	SetSequence(anim);
	Vector campos = Vector(armorpanel_camx.GetFloat(), armorpanel_camy.GetFloat(), armorpanel_camz.GetFloat());
	SetModelAnglesAndPosition(QAngle(0, -180, 0), vec3_origin);
	SetCameraPositionAndAngles(campos, vec3_angle);
}

void CHLRArmorPanel::Paint()
{
	BaseClass::Paint();
	Vector campos = Vector(armorpanel_camx.GetFloat(), armorpanel_camy.GetFloat(), armorpanel_camz.GetFloat());
	SetCameraPositionAndAngles(campos, vec3_angle);
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
	void SetupArmorPanel();
	void UpdateCombos();
	void SetupHelmets();
	void SetupChest();
	void SetupLegs();
	void SetupArms();
	void SetupCodpiece();
	void SetupTextBoxes();
	void UpdateEditorBodygroups();
	void UpdatePlayerBodygroups();
	virtual void OnPageShow();
	bool EnteringText();
	void RetrieveColors();
	void SetupColors();
	void UpdateColors();
	virtual void OnKeyCodeTyped(KeyCode code) { OnKeyCodePressed(code); }
	virtual void OnKeyCodePressed(KeyCode code);

	MESSAGE_FUNC(UpdateColorsWithTextEntries, "TextKillFocus");

	void UpdateTextBoxesFromSliders();

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

	Slider* primaryR;
	Slider* primaryG;
	Slider* primaryB;

	Slider* secondaryR;
	Slider* secondaryG;
	Slider* secondaryB;

	Slider* tertiaryR;
	Slider* tertiaryG;
	Slider* tertiaryB;

	Panel* primaryBox;
	Panel* secondaryBox;
	Panel* tertiaryBox;

	TextEntry* textTextEntryPR;
	TextEntry* textTextEntryPG;
	TextEntry* textTextEntryPB;
	TextEntry* textTextEntrySR;
	TextEntry* textTextEntrySG;
	TextEntry* textTextEntrySB;
	TextEntry* textTextEntryTR;
	TextEntry* textTextEntryTG;
	TextEntry* textTextEntryTB;


	CheckButton* UseCustomColors;
};


CHLRArmorCustomization::CHLRArmorCustomization(Panel* parent) : BaseClass(parent, NULL)
{
	SetParent(parent);
	SetupArmorPanel();
	SetupHelmets();
	SetupLegs();
	SetupChest();
	SetupCodpiece();
	SetupArms();
	SetupColors();
	SetupTextBoxes();

	LoadControlSettings("resource/ui/bonusoptionsarmor.res");
} 

void CHLRArmorCustomization::SetupTextBoxes()
{
	textTextEntryPR = new TextEntry(this, "textPrimaryR");
	textTextEntryPR->SetAllowNumericInputOnly(true);
	textTextEntryPG = new TextEntry(this, "textPrimaryG");
	textTextEntryPG->SetAllowNumericInputOnly(true);
	textTextEntryPB = new TextEntry(this, "textPrimaryB");
	textTextEntryPB->SetAllowNumericInputOnly(true);

	textTextEntrySR = new TextEntry(this, "textSecondaryR");
	textTextEntrySR->SetAllowNumericInputOnly(true);
	textTextEntrySG = new TextEntry(this, "textSecondaryG");
	textTextEntrySG->SetAllowNumericInputOnly(true);
	textTextEntrySB = new TextEntry(this, "textSecondaryB");
	textTextEntrySB->SetAllowNumericInputOnly(true);

	textTextEntryTR = new TextEntry(this, "textTertiaryR");
	textTextEntryTR->SetAllowNumericInputOnly(true);
	textTextEntryTG = new TextEntry(this, "textTertiaryG");
	textTextEntryTG->SetAllowNumericInputOnly(true);
	textTextEntryTB = new TextEntry(this, "textTertiaryB");
	textTextEntryTB->SetAllowNumericInputOnly(true);
}

bool CHLRArmorCustomization::EnteringText()
{
	if (textTextEntryPR->HasFocus() || textTextEntryPG->HasFocus() || textTextEntryPB->HasFocus())
		return true;

	if (textTextEntrySR->HasFocus() || textTextEntrySG->HasFocus() || textTextEntrySB->HasFocus())
		return true;

	if (textTextEntryTR->HasFocus() || textTextEntryTG->HasFocus() || textTextEntryTB->HasFocus())
		return true;

	return false;
}
void CHLRArmorCustomization::OnKeyCodePressed(KeyCode code)
{
	switch (code)
	{
	case KEY_ENTER:
	case KEY_PAD_ENTER:
		for (int i = 0; i < 9; i++)
		{
			if (EnteringText())
			{
				PostActionSignal(new KeyValues("TextKillFocus"));
				UpdateColorsWithTextEntries();
				GetParent()->RequestFocus();
			}
		}
		break;
	default:
		BaseClass::OnKeyCodePressed(code);
		break;
	}
}
void CHLRArmorCustomization::SetupArmorPanel()
{
	armorPanel = new CHLRArmorPanel(this);
	armorPanel->SetVisible(true);
	armorPanel->SetEnabled(true);
	armorPanel->SetSize(GetParent()->GetTall() / 1.5, GetParent()->GetTall());
	armorPanel->SetPos((GetParent()->GetWide() / 2) - (armorPanel->GetWide() / 2), 0);
	armorPanel->InitModel();
}
void CHLRArmorCustomization::SetupColors()
{
	UseCustomColors = new CheckButton(this, "CustomColorsCheckBox", "Use Custom Armor Colors");

	primaryR = new Slider(this, "primaryR");
	primaryR->SetRange(0, 255);
	primaryG = new Slider(this, "primaryG");
	primaryG->SetRange(0, 255);
	primaryB = new Slider(this, "primaryB");
	primaryB->SetRange(0, 255);
	primaryBox = new Panel(this, "PrimaryBox");
	primaryBox->SetSize(64, 64);
	primaryBox->SetPaintBackgroundEnabled(false);

	secondaryR = new Slider(this, "secondaryR");
	secondaryR->SetRange(0, 255);
	secondaryG = new Slider(this, "secondaryG");
	secondaryG->SetRange(0, 255);
	secondaryB = new Slider(this, "secondaryB");
	secondaryB->SetRange(0, 255);
	secondaryBox = new Panel(this, "secondaryBox");
	secondaryBox->SetSize(64, 64);
	secondaryBox->SetPaintBackgroundEnabled(false);

	tertiaryR = new Slider(this, "tertiaryR");
	tertiaryR->SetRange(0, 255);
	tertiaryG = new Slider(this, "tertiaryG");
	tertiaryG->SetRange(0, 255);
	tertiaryB = new Slider(this, "tertiaryB");
	tertiaryB->SetRange(0, 255);
	tertiaryBox = new Panel(this, "tertiaryBox");
	tertiaryBox->SetSize(64, 64);
	tertiaryBox->SetPaintBackgroundEnabled(false);
}
void CHLRArmorCustomization::UpdateColorsWithTextEntries()
{
	char text[128];
	textTextEntryPR->GetText(text, sizeof(text));
	primaryR->SetValue(atoi(text));
	textTextEntryPG->GetText(text, sizeof(text));
	primaryG->SetValue(atoi(text));
	textTextEntryPB->GetText(text, sizeof(text));
	primaryB->SetValue(atoi(text));

	textTextEntrySR->GetText(text, sizeof(text));
	secondaryR->SetValue(atoi(text));
	textTextEntrySG->GetText(text, sizeof(text));
	secondaryG->SetValue(atoi(text));
	textTextEntrySB->GetText(text, sizeof(text));
	secondaryB->SetValue(atoi(text));

	textTextEntryTR->GetText(text, sizeof(text));
	tertiaryR->SetValue(atoi(text));
	textTextEntryTG->GetText(text, sizeof(text));
	tertiaryG->SetValue(atoi(text));
	textTextEntryTB->GetText(text, sizeof(text));
	tertiaryB->SetValue(atoi(text));
}
void CHLRArmorCustomization::UpdateColors()
{
	ConVarRef primR("mat_player_primary_r");
	ConVarRef primG("mat_player_primary_g");
	ConVarRef primB("mat_player_primary_b");

	ConVarRef secR("mat_player_secondary_r");
	ConVarRef secG("mat_player_secondary_g");
	ConVarRef secB("mat_player_secondary_b");

	ConVarRef tertR("mat_player_tertiary_r");
	ConVarRef tertG("mat_player_tertiary_g");
	ConVarRef tertB("mat_player_tertiary_b");

	primR.SetValue(primaryR->GetValue());
	primG.SetValue(primaryG->GetValue());
	primB.SetValue(primaryB->GetValue());

	secR.SetValue(secondaryR->GetValue());
	secG.SetValue(secondaryG->GetValue());
	secB.SetValue(secondaryB->GetValue());

	tertR.SetValue(tertiaryR->GetValue());
	tertG.SetValue(tertiaryG->GetValue());
	tertB.SetValue(tertiaryB->GetValue());

	UpdateTextBoxesFromSliders();
}

void CHLRArmorCustomization::UpdateTextBoxesFromSliders()
{
	if (!EnteringText())
	{
		char text[128];
		sprintf(text, "%i", primaryR->GetValue());
		textTextEntryPR->SetText(text);
		sprintf(text, "%i", primaryG->GetValue());
		textTextEntryPG->SetText(text);
		sprintf(text, "%i", primaryB->GetValue());
		textTextEntryPB->SetText(text);

		sprintf(text, "%i", secondaryR->GetValue());
		textTextEntrySR->SetText(text);
		sprintf(text, "%i", secondaryG->GetValue());
		textTextEntrySG->SetText(text);
		sprintf(text, "%i", secondaryB->GetValue());
		textTextEntrySB->SetText(text);

		sprintf(text, "%i", tertiaryR->GetValue());
		textTextEntryTR->SetText(text);
		sprintf(text, "%i", tertiaryG->GetValue());
		textTextEntryTG->SetText(text);
		sprintf(text, "%i", tertiaryB->GetValue());
		textTextEntryTB->SetText(text);
	}
}
void CHLRArmorCustomization::Paint()
{
	BaseClass::Paint();
	int xpos, ypos, wide, tall;

	primaryBox->GetPos(xpos, ypos);
	primaryBox->GetSize(wide, tall);
	surface()->DrawSetColor(primaryR->GetValue(), primaryG->GetValue(), primaryB->GetValue(), 255);
	surface()->DrawFilledRect(xpos, ypos, xpos + wide, ypos + tall);

	secondaryBox->GetPos(xpos, ypos);
	secondaryBox->GetSize(wide, tall);
	surface()->DrawSetColor(secondaryR->GetValue(), secondaryG->GetValue(), secondaryB->GetValue(), 255);
	surface()->DrawFilledRect(xpos, ypos, xpos + wide, ypos + tall);

	tertiaryBox->GetPos(xpos, ypos);
	tertiaryBox->GetSize(wide, tall);
	surface()->DrawSetColor(tertiaryR->GetValue(), tertiaryG->GetValue(), tertiaryB->GetValue(), 255);
	surface()->DrawFilledRect(xpos, ypos, xpos + wide, ypos + tall);

	UpdateEditorBodygroups();
	UpdateColors();

	ConVarRef usecustomcolors("mat_player_use_custom_colors");
	if (UseCustomColors->IsSelected() != usecustomcolors.GetBool())
		OnDataChanged();

	if (helmetCombo->IsDropdownVisible() || bootscombo->IsDropdownVisible() || thighcombo->IsDropdownVisible() || codpiececombo->IsDropdownVisible() || upperarmscombo->IsDropdownVisible() ||
		forearmscombo->IsDropdownVisible() || chestcombo->IsDropdownVisible())
	{
		OnDataChanged();
	}
}

void CHLRArmorCustomization::RetrieveColors()
{
	ConVarRef primR("mat_player_primary_r");
	ConVarRef primG("mat_player_primary_g");
	ConVarRef primB("mat_player_primary_b");

	ConVarRef secR("mat_player_secondary_r");
	ConVarRef secG("mat_player_secondary_g");
	ConVarRef secB("mat_player_secondary_b");

	ConVarRef tertR("mat_player_tertiary_r");
	ConVarRef tertG("mat_player_tertiary_g");
	ConVarRef tertB("mat_player_tertiary_b");

	ConVarRef usecustomcolors("mat_player_use_custom_colors");

	UseCustomColors->SetSelected(usecustomcolors.GetBool());
	primaryR->SetValue(primR.GetInt());
	primaryG->SetValue(primG.GetInt());
	primaryB->SetValue(primB.GetInt());

	secondaryR->SetValue(secR.GetInt());
	secondaryG->SetValue(secG.GetInt());
	secondaryB->SetValue(secB.GetInt());

	tertiaryR->SetValue(tertR.GetInt());
	tertiaryG->SetValue(tertG.GetInt());
	tertiaryB->SetValue(tertB.GetInt());

	UpdateTextBoxesFromSliders();
}
void CHLRArmorCustomization::SetupHelmets()
{
	helmetCombo = new ComboBox(this, "HelmetCombo", 3, false);
	helmetCombo->SetProportional(false);
	helmetCombo->AddItem("#HLR_Mark6", NULL);
	helmetCombo->AddItem("#HLR_CQB", NULL);
	helmetCombo->AddItem("#HLR_Cosmoneer", NULL);

	helmetlabel = new Label(this, "HelmetLabel", "#HLR_Helmet");
	helmetlabel->SetProportional(false);
}
void CHLRArmorCustomization::SetupChest()
{
	chestcombo = new ComboBox(this, "ChestCombo", 3, false);
	chestcombo->SetProportional(false);
	chestcombo->AddItem("#HLR_Mark6", NULL);
	chestcombo->AddItem("#HLR_CQB", NULL);
	chestcombo->AddItem("#HLR_Cosmoneer", NULL);

	chestlabel = new Label(this, "ChestLabel", "#HLR_Chest");
	chestlabel->SetProportional(false);
}
void CHLRArmorCustomization::SetupLegs()
{
	bootscombo = new ComboBox(this, "BootsCombo", 3, false);
	bootscombo->SetProportional(false);
	bootscombo->AddItem("#HLR_Mark6", NULL);
	bootscombo->AddItem("#HLR_CQB", NULL);
	bootscombo->AddItem("#HLR_Cosmoneer", NULL);

	bootslabel = new Label(this, "BootsLabel", "#HLR_Boots");
	bootslabel->SetProportional(false);

	thighcombo = new ComboBox(this, "ThighCombo", 3, false);
	thighcombo->SetProportional(false);
	thighcombo->AddItem("#HLR_Mark6", NULL);
	thighcombo->AddItem("#HLR_CQB", NULL);
	thighcombo->AddItem("#HLR_Cosmoneer", NULL);

	thighlabel = new Label(this, "ThighLabel", "#HLR_Thighs");
	thighlabel->SetProportional(false);
}
void CHLRArmorCustomization::SetupArms()
{
	upperarmscombo = new ComboBox(this, "UpperArmsCombo", 3, false);
	upperarmscombo->SetProportional(false);
	upperarmscombo->AddItem("#HLR_Mark6", NULL);
	upperarmscombo->AddItem("#HLR_CQB", NULL);
	upperarmscombo->AddItem("#HLR_Cosmoneer", NULL);

	upperarmlabel = new Label(this, "UpperarmLabel", "#HLR_Upperarm");
	upperarmlabel->SetProportional(false);

	forearmscombo = new ComboBox(this, "ForearmsCombo", 3, false);
	forearmscombo->SetProportional(false);
	forearmscombo->AddItem("#HLR_Mark6", NULL);
	forearmscombo->AddItem("#HLR_CQB", NULL);
	forearmscombo->AddItem("#HLR_Cosmoneer", NULL);

	forearmlabel = new Label(this, "ForearmLabel", "#HLR_Forearm");
	forearmlabel->SetProportional(false);
}
void CHLRArmorCustomization::SetupCodpiece()
{
	codpiececombo = new ComboBox(this, "CodpieceCombo", 3, false);
	codpiececombo->SetProportional(false);
	codpiececombo->AddItem("#HLR_Mark6", NULL);
	codpiececombo->AddItem("#HLR_CQB", NULL);
	codpiececombo->AddItem("#HLR_Cosmoneer", NULL);

	codpiecelabel = new Label(this, "CodpieceLabel", "#HLR_Codpiece");
	codpiecelabel->SetProportional(false);
}
void CHLRArmorCustomization::OnPageShow()
{
	RetrieveColors();
	UpdateEditorBodygroups();
}
void CHLRArmorCustomization::OnResetData()
{
	UpdateCombos();
	UpdateColors();
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
	ConVarRef usecustomcolors("mat_player_use_custom_colors");
	usecustomcolors.SetValue(UseCustomColors->IsSelected());
	UpdatePlayerBodygroups();
	UpdateColors();
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

	engine->ServerCmd("updatearmor");
}
void CHLRArmorCustomization::OnDataChanged()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
	UpdateEditorBodygroups();
}
class CHLRArmorFrame : public PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CHLRArmorFrame, PropertyDialog);
public:
	CHLRArmorFrame(vgui::VPANEL parent);
	virtual void Activate();
	~CHLRArmorFrame();

	virtual void OnKeyCodePressed(KeyCode code);

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

void CHLRArmorFrame::OnKeyCodePressed(KeyCode code)
{
	return;
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

	int sw = 960;
	int sh = 540;

	int mw = panel->GetWide() / 2;
	int mh = panel->GetTall() / 2;
	panel->SetPos(sw - mw, sh - mh);
	panel->Activate();
}