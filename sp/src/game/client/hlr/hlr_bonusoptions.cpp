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


using namespace vgui;


//////////////////////////////////////////////////////////////
////////// BONUS OPTIONS GRAPHICS PAGE //////////////////
//////////////////////////////////////////////////////
///////////////////////////////////////////////

class CHLRSubBonusOptionsGraphics : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRSubBonusOptionsGraphics, vgui::PropertyPage);
public:
	CHLRSubBonusOptionsGraphics(vgui::Panel* parent);

	virtual void OnResetData();
	virtual void OnApplyChanges();

	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked");
private:
	vgui::Slider* m_pRainSlider;
	vgui::CheckButton* m_pParallax;
	vgui::CheckButton* m_pFuryEffects;
};

CHLRSubBonusOptionsGraphics::CHLRSubBonusOptionsGraphics(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	m_pParallax = new CheckButton(this, "Parallax", "Parallax Occlusion Mapping");
	m_pFuryEffects = new CheckButton(this, "FuryFX", "Fury Effects");
	m_pRainSlider = new Slider(this, "RainDensity");
	m_pRainSlider->SetRange(0, 100);
	LoadControlSettings("resource/ui/bonusoptionsgraphics.res");
}

void CHLRSubBonusOptionsGraphics::OnResetData()
{
	ConVarRef parallax("mat_pbr_parallaxmap");
	ConVarRef raindensity("r_RainSplashPercentage");
	ConVarRef furyFX("g_draw_fury_effects");
	m_pRainSlider->SetValue(raindensity.GetFloat());

	m_pParallax->SetSelected(parallax.GetBool());

	m_pFuryEffects->SetSelected(furyFX.GetBool());
}

void CHLRSubBonusOptionsGraphics::OnApplyChanges()
{
	ConVarRef parallax("mat_pbr_parallaxmap");
	ConVarRef raindensity("r_RainSplashPercentage");
	ConVarRef furyFX("g_draw_fury_effects");
	raindensity.SetValue(m_pRainSlider->GetValue());
	m_pParallax->IsSelected() ? parallax.SetValue(1) : parallax.SetValue(0);
	m_pFuryEffects->IsSelected() ? furyFX.SetValue(1) : furyFX.SetValue(0);
}

void CHLRSubBonusOptionsGraphics::OnCheckButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
////////// BONUS OPTIONS EXTRAS PAGE ////////////////////
//////////////////////////////////////////////////////
/////////////////////////////////////////////////

class CHLRSubBonusOptions : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRSubBonusOptions, vgui::PropertyPage);
public:
	CHLRSubBonusOptions(vgui::Panel* parent);

	virtual void OnResetData();
	virtual void OnApplyChanges();

	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked");
private:
	CheckButton* m_pGibs;
	CheckButton* m_pClassic;
	CheckButton* m_pGuts;
	CheckButton* m_pRocket;
	CheckButton* m_prainbow;


	Slider* m_pRainbowRate;

};


CHLRSubBonusOptions::CHLRSubBonusOptions(vgui::Panel *parent) : BaseClass(parent, NULL)
{
	m_pGibs = new CheckButton(this, "GibsButton", "UltraGibs");

	m_pClassic = new CheckButton(this, "ClassicButton", "Retro Mode");

	m_pGuts = new CheckButton(this, "GutsButton", "Guts and Glory");

	m_pRocket = new CheckButton(this, "RocketButton", "Rocket Jump Mania");

	m_prainbow = new CheckButton(this, "RainbowHudButton", "Rainbow HUD");

	m_pRainbowRate = new Slider(this, "RainbowRateSlider");
	m_pRainbowRate->SetRange(1, 10);

	LoadControlSettings("resource/ui/bonusoptions.res");
}

void CHLRSubBonusOptions::OnResetData()
{
	ConVarRef gibs("g_ultragibs");
	ConVarRef classic("mat_classic_render");
	ConVarRef guts("g_guts_and_glory");
	ConVarRef rocket("g_rocket_jump_mania");
	ConVarRef rainbow("hud_rainbow");
	ConVarRef rainbowrate("hud_rainbow_rate");

	m_pGuts->SetSelected(guts.GetBool());
	m_pGibs->SetSelected(gibs.GetBool());
	m_pClassic->SetSelected(classic.GetBool());
	m_pRocket->SetSelected(rocket.GetBool());
	m_prainbow->SetSelected(rainbow.GetBool());
	m_pRainbowRate->SetValue(rainbowrate.GetFloat());
}
void CHLRSubBonusOptions::OnApplyChanges()
{
	ConVarRef gibs("g_ultragibs");
	ConVarRef classic("mat_classic_render");
	ConVarRef guts("g_guts_and_glory");
	ConVarRef rocket("g_rocket_jump_mania");
	ConVarRef rainbow("hud_rainbow");
	ConVarRef rainbowrate("hud_rainbow_rate");

	m_pGibs->IsSelected() ? gibs.SetValue(1) : gibs.SetValue(0);
	m_pClassic->IsSelected() ? classic.SetValue(1) : classic.SetValue(0);
	m_pGuts->IsSelected() ? guts.SetValue(1) : guts.SetValue(0);
	m_pRocket->IsSelected() ? rocket.SetValue(1) : rocket.SetValue(0);
	m_prainbow->IsSelected() ? rainbow.SetValue(1) : rainbow.SetValue(0);

	rainbowrate.SetValue(m_pRainbowRate->GetValue());

	DevMsg("changing bonus option data\n");

}
void CHLRSubBonusOptions::OnCheckButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
////////// BONUS OPTIONS GAMEPLAY PAGE ///////////////////
//////////////////////////////////////////////////////
/////////////////////////////////////////////////

class CHLRSubBonusOptionsGameplay : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRSubBonusOptionsGameplay, vgui::PropertyPage);
public:
	CHLRSubBonusOptionsGameplay(vgui::Panel* parent);

	virtual void OnResetData();
	virtual void OnApplyChanges();
	virtual void Paint();

	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked");

private:
	vgui::Slider* m_pautoaimscale;
	vgui::CheckButton* m_pautoaim;
	vgui::CheckButton* m_pthirdperson;
	vgui::CheckButton* m_pslowmo;
	vgui::Slider* m_sHudR;
	vgui::Slider* m_sHudG;
	vgui::Slider* m_sHudB;
	vgui::Panel* m_pCrosshair;
	vgui::CheckButton* m_poverride;
	int iautoaimscale;

};


CHLRSubBonusOptionsGameplay::CHLRSubBonusOptionsGameplay(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	m_pautoaim = new CheckButton(this, "Autoaim", "Aim Assist");

	m_pslowmo = new CheckButton(this, "Slowmo", "Weapon Selection SlowMo");
	
	m_sHudR = new Slider(this, "HudRed");
	m_sHudG = new Slider(this, "HudGreen");
	m_sHudB = new Slider(this, "HudBlue");
	m_pCrosshair = new Panel(this, "CrosshairPanel");
	m_pCrosshair->SetPaintBackgroundEnabled(false);

	m_sHudR->SetRange(0, 255);
	m_sHudG->SetRange(0, 255);
	m_sHudB->SetRange(0, 255);

	m_poverride = new CheckButton(this, "haoverride", "Override Health/Armor Colors");


	m_pautoaimscale = new Slider(this, "AutoaimScale");
	m_pautoaimscale->SetRange(0, 10);
	m_pautoaimscale->SetNumTicks(10);

	m_pthirdperson = new CheckButton(this, "ThirddPerson", "Third-Person");


	LoadControlSettings("resource/ui/bonusoptionsgameplay.res");
}

void CHLRSubBonusOptionsGameplay::OnResetData()
{
	ConVarRef autoaimscale("hud_autoaim_scale");
	ConVarRef autoaim("hud_draw_active_reticle");
	ConVarRef thirdperson("g_thirdperson");
	ConVarRef thirdpersoncrosshair("cl_thirdperson_crosshair");
	ConVarRef slowmo("hud_weapon_selection_slowmo");
	ConVarRef hudr("hud_color_r");
	ConVarRef hudg("hud_color_g");
	ConVarRef hudb("hud_color_b");
	ConVarRef haoverride("hud_override_healtharmor_color");

	iautoaimscale = autoaimscale.GetFloat() * 10;

	m_pautoaimscale->SetValue(iautoaimscale);

	m_pautoaim->SetSelected(autoaim.GetBool());
	m_pthirdperson->SetSelected(thirdperson.GetBool());
	m_pslowmo->SetSelected(slowmo.GetBool());

	m_sHudR->SetValue(hudr.GetFloat());
	m_sHudG->SetValue(hudg.GetFloat());
	m_sHudB->SetValue(hudb.GetFloat());

	m_poverride->SetSelected(haoverride.GetBool());
}
void CHLRSubBonusOptionsGameplay::OnApplyChanges()
{
	ConVarRef autoaimscale("hud_autoaim_scale");
	ConVarRef autoaim("hud_draw_active_reticle");
	ConVarRef thirdperson("g_thirdperson");
	ConVarRef thirdpersoncrosshair("cl_thirdperson_crosshair");
	ConVarRef slowmo("hud_weapon_selection_slowmo");
	ConVarRef hudr("hud_color_r");
	ConVarRef hudg("hud_color_g");
	ConVarRef hudb("hud_color_b");
	ConVarRef haoverride("hud_override_healtharmor_color");

	iautoaimscale = m_pautoaimscale->GetValue();

	float scale = iautoaimscale / 10.0f;

	autoaimscale.SetValue(scale);

	m_poverride->IsSelected() ? haoverride.SetValue(1) : haoverride.SetValue(0);

	m_pslowmo->IsSelected() ? slowmo.SetValue(1) : slowmo.SetValue(0);
	m_pautoaim->IsSelected() ? autoaim.SetValue(1) : autoaim.SetValue(0);

	int thirdpersonsetting;
	if (m_pthirdperson->IsSelected())
	{
		thirdpersonsetting = 1;
		engine->ClientCmd("thirdperson\n");
	}
	else
	{
		thirdpersonsetting = 0;
		engine->ClientCmd("firstperson\n");
	}

	thirdperson.SetValue(thirdpersonsetting);
	thirdpersoncrosshair.SetValue(thirdpersonsetting);

	hudr.SetValue(m_sHudR->GetValue());
	hudg.SetValue(m_sHudG->GetValue());
	hudb.SetValue(m_sHudB->GetValue());

	DevMsg("changing bonus option data\n");

}
void CHLRSubBonusOptionsGameplay::OnCheckButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

void CHLRSubBonusOptionsGameplay::Paint()
{
	BaseClass::Paint();

	Color clr;
	clr[0] = m_sHudR->GetValue();
	clr[1] = m_sHudG->GetValue();
	clr[2] = m_sHudB->GetValue();
	clr[3] = 255;
	int xpos, ypos;
	m_pCrosshair->GetPos(xpos, ypos);
	vgui::surface()->DrawSetColor(clr);
	vgui::surface()->DrawFilledRect(xpos, ypos, xpos + m_pCrosshair->GetWide(), ypos + m_pCrosshair->GetTall());


}

class CHLRArmorPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHLRArmorPanel, Panel);
public:
	CHLRArmorPanel(vgui::Panel* parent);
	virtual void Paint();
	void SetArmorGroup(const char* szGroupName, int nValue);

	MDLHandle_t handle;
	CStudioHdr* studio;

	int m_nRotation;
	CBaseModelPanel* armorPanel;


private:
	
	
};

CHLRArmorPanel::CHLRArmorPanel(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	SetWide(512);
	SetTall(512);
	SetVisible(true);
	SetEnabled(true);
	armorPanel = new CBaseModelPanel(this, "ArmorPanel");
	studio = new CStudioHdr;

	m_nRotation = 0;
	handle = mdlcache->FindMDL("models/player/mark6.mdl");

	armorPanel->SetMDL(handle);
	armorPanel->SetMouseInputEnabled(true);
	armorPanel->SetEnabled(true);
	armorPanel->SetVisible(true);
	armorPanel->SetWide(GetWide());
	armorPanel->SetTall(GetTall());

	

}

void CHLRArmorPanel::Paint()
{
	BaseClass::Paint();
	studio->Init(armorPanel->GetStudioHdr(), mdlcache);
	int anim = LookupSequence(studio, "armor_custom_idle");
	armorPanel->SetSequence(anim);
	//armorPanel->SetCameraPositionAndAngles(vecPos, vec3_angle);
	armorPanel->SetModelAnglesAndPosition(QAngle(0,m_nRotation - 180,0), vec3_origin);
	armorPanel->SetCameraPositionAndAngles(Vector(-350, 0, 8), vec3_angle);
//	armorPanel->SetCameraPositionAndAngles(vec3_origin, vec3_angle);
	//armorPanel->SetLookAtCamera(true);


}

void CHLRArmorPanel::SetArmorGroup(const char* szGroupName, int nValue)
{
	if (studio->IsValid())
	{
		int bodygroup = ::FindBodygroupByName(studio, szGroupName);
		if (handle)
			armorPanel->SetBodygroup(handle, bodygroup, nValue);
	}
}

class CHLRArmorCustomization : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRArmorCustomization, PropertyPage);

public:
	CHLRArmorCustomization(vgui::Panel* parent);
	virtual void Paint();
	virtual void OnApplyChanges();
	virtual void OnDataChanged();
	virtual void OnResetData();
	void SetupHelmets();
	void SetupChest();
	void SetupBoots();
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
	Slider* modelSlider;
};


CHLRArmorCustomization::CHLRArmorCustomization(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	armorPanel = new CHLRArmorPanel(this);
	armorPanel->SetVisible(true);
	armorPanel->SetEnabled(true);

	SetupHelmets();
	SetupBoots();
	SetupChest();
	SetupCodpiece();
	SetupArms();


	modelSlider = new Slider(this, "ModelRotationSlider");
	modelSlider->SetRange(0, 360);

	LoadControlSettings("resource/ui/bonusoptionsarmor.res");
}

void CHLRArmorCustomization::SetupHelmets()
{
	helmetCombo = new ComboBox(this, "HelmetCombo", 3, false);
	helmetCombo->AddItem("#HLR_Mark6", NULL);
	helmetCombo->AddItem("#HLR_CQB", NULL);
	helmetCombo->AddItem("#HLR_Cosmoneer", NULL);
}
void CHLRArmorCustomization::SetupChest()
{
	chestcombo = new ComboBox(this, "ChestCombo", 3, false);
	chestcombo->AddItem("#HLR_Mark6", NULL);
	chestcombo->AddItem("#HLR_CQB", NULL);
	chestcombo->AddItem("#HLR_Cosmoneer", NULL);
}
void CHLRArmorCustomization::SetupBoots()
{
	bootscombo = new ComboBox(this, "BootsCombo", 3, false);
	bootscombo->AddItem("#HLR_Mark6", NULL);
	bootscombo->AddItem("#HLR_CQB", NULL);
	bootscombo->AddItem("#HLR_Cosmoneer", NULL);

	thighcombo = new ComboBox(this, "ThighCombo", 3, false);
	thighcombo->AddItem("#HLR_Mark6", NULL);
	thighcombo->AddItem("#HLR_CQB", NULL);
	thighcombo->AddItem("#HLR_Cosmoneer", NULL);
}
void CHLRArmorCustomization::SetupArms()
{
	upperarmscombo = new ComboBox(this, "UpperArmsCombo", 3, false);
	upperarmscombo->AddItem("#HLR_Mark6", NULL);
	upperarmscombo->AddItem("#HLR_CQB", NULL);
	upperarmscombo->AddItem("#HLR_Cosmoneer", NULL);

	forearmscombo = new ComboBox(this, "ForearmsCombo", 3, false);
	forearmscombo->AddItem("#HLR_Mark6", NULL);
	forearmscombo->AddItem("#HLR_CQB", NULL);
	forearmscombo->AddItem("#HLR_Cosmoneer", NULL);
}
void CHLRArmorCustomization::SetupCodpiece()
{
	codpiececombo = new ComboBox(this, "CodpieceCombo", 3, false);
	codpiececombo->AddItem("#HLR_Mark6", NULL);
	codpiececombo->AddItem("#HLR_CQB", NULL);
	codpiececombo->AddItem("#HLR_Cosmoneer", NULL);
}
void CHLRArmorCustomization::OnPageShow()
{
	UpdateEditorBodygroups();
}
void CHLRArmorCustomization::OnResetData()
{
	//C_BasePlayer* pPlayer = CBasePlayer::GetLocalPlayer();
	ConVarRef helmet("cl_armor_helmet");
	helmetCombo->ActivateItem(helmet.GetInt());
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
	armorPanel->SetArmorGroup("upperarm", upperarmscombo->GetActiveItem());
	armorPanel->SetArmorGroup("forearm", forearmscombo->GetActiveItem());
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
	armorPanel->m_nRotation = modelSlider->GetValue();

	if (helmetCombo->IsDropdownVisible())
		OnDataChanged();
	


	////// DOESN'T WORK DON'T EVEN TRY TO MAKE IT WORK UGUUUGUGGGHGHGHGHGHG

	/*if (armorPanel->studio->IsValid())
	{
		int bone = Studio_BoneIndexByName(armorPanel->studio, "ValveBiped.Bip01_Head1");
		QAngle ang, camang;
		Vector vec, camvec, position;
		int x, y, x2, y2, x3, y3;

		helmetCombo->GetPos(x, y);
		GetParent()->GetPos(x2, y2);
		Camera_t cam;
		Vector2D vec2d;
		position = armorPanel->studio->pBone(bone)->pos;
		matrix3x4_t *matrix = armorPanel->studio->pAttachment(1).
		armorPanel->armorPanel->GetCameraPositionAndAngles(camvec, camang);
		GetVectorInScreenSpace(position, x3, y3);

		
		cam.m_origin = view->GetViewSetup()->origin;
		cam.m_angles = view->GetViewSetup()->angles;
		cam.m_flFOV = view->GetViewSetup()->fov;
		cam.m_flZNear = view->GetViewSetup()->zNear;
		cam.m_flZFar = view->GetViewSetup()->zFar;
		DevMsg("attachment x = %f\n attachment y = %f\n attachment z = %f\n", position.x, position.y, position.z);
		DevMsg("Transform x = %f\n Transform y = %f\n, Transform z = %f\n", camvec.x, camvec.y, camvec.z);
		DevMsg("screenspace x = %f\n screenspace y = %f\n",x3,y3);
		//DevMsg("screenspace x = %f\n screenspace y = %f\n", vec2d.x, vec2d.y);
		surface()->DrawSetColor(Color(255, 255, 255, 255));
		surface()->DrawLine(position.x, position.z, x + helmetCombo->GetWide(), y + (helmetCombo->GetTall() * 0.5f));

	}
*/	
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
////////// BONUS OPTIONS DIALOG //////////////////////////
//////////////////////////////////////////////////////
///////////////////////////////////////////////



class CHLRBonusOptions : public vgui::PropertyDialog
{
public:
	DECLARE_CLASS_SIMPLE(CHLRBonusOptions, vgui::PropertyDialog);

	CHLRBonusOptions(vgui::VPANEL parent);
	~CHLRBonusOptions();
	virtual void Activate();
private:
};

CHLRBonusOptions::CHLRBonusOptions(VPANEL parent) : BaseClass(nullptr, "BonusOptions")
{
	SetParent(parent);
	SetBounds(0, 0, 512, 420);
	SetDeleteSelfOnClose(true);
	SetSizeable(false);
	SetApplyButtonVisible(true);

	SetTitle("", true);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));



	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	AddPage(new CHLRSubBonusOptionsGameplay(this), "#HLR_BonusOptions_Gameplay");
	AddPage(new CHLRSubBonusOptionsGraphics(this), "#HLR_BonusOptions_Graphics");
	AddPage(new CHLRSubBonusOptions(this), "#HLR_BonusOptions_Extras");
	AddPage(new CHLRArmorCustomization(this), "#HLR_BonusOptions_Armor");
}
CHLRBonusOptions::~CHLRBonusOptions()
{

}

void CHLRBonusOptions::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(true);
}


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
