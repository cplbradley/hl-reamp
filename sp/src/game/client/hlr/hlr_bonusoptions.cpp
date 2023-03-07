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

#include "tier0/memdbgon.h"


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


private:
	CBaseModelPanel* armorPanel;
	
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
	if (!studio->IsValid())
		return;
	int bodygroup = ::FindBodygroupByName(studio, szGroupName);
	if (!handle)
		return;
	armorPanel->SetBodygroup(handle, bodygroup, nValue);
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
	void UpdateEditorBodygroups();
	void UpdatePlayerBodygroups();
	
	virtual void OnPageShow();

	CHLRArmorPanel* armorPanel;
	ComboBox* helmetCombo;
	Slider* modelSlider;
};


CHLRArmorCustomization::CHLRArmorCustomization(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	armorPanel = new CHLRArmorPanel(this);
	armorPanel->SetVisible(true);
	armorPanel->SetEnabled(true);

	helmetCombo = new ComboBox(this, "HelmetCombo", 3, false);
	helmetCombo->AddItem("#HLR_Mark6", NULL);

	helmetCombo->AddItem("#HLR_CQB", NULL);
	helmetCombo->AddItem("#HLR_Cosmoneer", NULL);


	modelSlider = new Slider(this, "ModelRotationSlider");
	modelSlider->SetRange(0, 360);

	LoadControlSettings("resource/ui/bonusoptionsarmor.res");
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
}

void CHLRArmorCustomization::UpdatePlayerBodygroups()
{
	armorPanel->SetArmorGroup("helmet", helmetCombo->GetActiveItem());
	ConVarRef helmet("cl_armor_helmet");
	helmet.SetValue(helmetCombo->GetActiveItem());
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
