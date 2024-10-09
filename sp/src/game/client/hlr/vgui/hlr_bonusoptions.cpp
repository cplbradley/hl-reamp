#include "cbase.h"
#include "cdll_client_int.h"
#include "ienginevgui.h"

#include "hlr/hlr_shareddefs.h"
#include "basecombatweapon_shared.h"

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


enum {
	VQ_LOW,
	VQ_MEDIUM,
	VQ_HIGH,
	VQ_MAX
};

ConVar r_videoquality("r_videoquality", "2", FCVAR_CLIENTDLL | FCVAR_HIDDEN | FCVAR_ARCHIVE);


//////////////////////////////////////////////////////////////
////////// BONUS OPTIONS GRAPHICS PAGE //////////////////
//////////////////////////////////////////////////////
///////////////////////////////////////////////

/*const char* slotnames[10] =
{
	"Pistol",
	"PumpShotgun",
	"SuperShotgun",
	"PlasmaRifle",
	"Chaingun",
	"Railgun",
	"Rifle",
	"GrenadeLauncher",
	"RocketLauncher",
	"BFG"
};
const char* slotweapons[10] =
{
	"weapon_pistol",
	"weapon_pumpshotgun",
	"weapon_shotgun",
	"weapon_plasmarifle",
	"weapon_chaingun",
	"weapon_railgun",
	"weapon_rifle",
	"weapon_frag",
	"weapon_rpg",
	"weapon_bfg"
};

class CHLRSubBonusOptionsWeaponWheel : public PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRSubBonusOptionsWeaponWheel, PropertyPage)
public:
	CHLRSubBonusOptionsWeaponWheel(Panel* parent);
	virtual void OnResetData();
	virtual void OnThink();
	virtual void OnDataChanged();
	virtual void OnApplyChanges();
	virtual void OnPageShow();

	void ApplyWeaponSlots();
	ComboBox* wheelslot[10];

};

CHLRSubBonusOptionsWeaponWheel::CHLRSubBonusOptionsWeaponWheel(Panel* parent) : BaseClass(parent, NULL)
{
	for (int i = 0; i < 10; i++)
	{
		char str[16] = "wheelslot";
		sprintf(str + strlen(str), "%i", i);
		wheelslot[i] = new ComboBox(parent, str, 10, false);
		if (wheelslot[i])
		{
			for (int j = 0; j < 10; j++)
			{
				wheelslot[i]->AddItem(slotnames[j], NULL);
			}
		}
	}
}


void CHLRSubBonusOptionsWeaponWheel::ApplyWeaponSlots()
{
	for (int i = 0; i < 10; i++)
	{
		CBaseCombatWeapon* weapon = 
	}
}*/
class CHLRSubBonusOptionsGraphics : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CHLRSubBonusOptionsGraphics, vgui::PropertyPage);
public:
	CHLRSubBonusOptionsGraphics(vgui::Panel* parent);

	virtual void OnResetData();
	virtual void OnThink();
	virtual void OnApplyChanges();
	virtual void OnPageShow();
	void ApplyVideoQuality();
	virtual void OnDataChanged();
	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked");
	MESSAGE_FUNC(OnMenuItemSelected, "MenuItemSelected");
private:
	Slider* m_pRainSlider;
	CheckButton* m_pParallax;
	CheckButton* m_pFuryEffects;
	ComboBox* m_pVideoQuality;
	CheckButton* m_pVSync;
	CheckButton* m_pMulticore;
	CheckButton* m_pMuzzleLights;
	CheckButton* m_pEfficientParticles;
	Label* m_pRainValue;

};

CHLRSubBonusOptionsGraphics::CHLRSubBonusOptionsGraphics(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	m_pParallax = new CheckButton(this, "Parallax", "Parallax Occlusion Mapping");
	m_pFuryEffects = new CheckButton(this, "FuryFX", "Fury Effects");
	m_pVSync = new CheckButton(this, "VSync", "VSync");
	m_pMulticore = new CheckButton(this, "Multicore", "Multicore");
	m_pRainSlider = new Slider(this, "RainDensity");
	m_pRainSlider->SetRange(0, 100);
	m_pRainValue = new Label(this, "RainDensityValue", "NULL");

	m_pVideoQuality = new ComboBox(this, "VideoQuality",4,false);
	m_pVideoQuality->AddItem("Low", NULL);
	m_pVideoQuality->AddItem("Medium", NULL);
	m_pVideoQuality->AddItem("High", NULL);
	m_pVideoQuality->AddItem("Max", NULL);

	m_pMuzzleLights = new CheckButton(this, "MuzzleLights", "Muzzle Flash Lights");
	m_pEfficientParticles = new CheckButton(this, "EfficientParticles", "Efficient Particles");
	

	LoadControlSettings("resource/ui/bonusoptionsgraphics.res");
}

void CHLRSubBonusOptionsGraphics::OnResetData()
{
	ConVarRef parallax("mat_pbr_parallaxmap");
	ConVarRef raindensity("r_RainSplashPercentage");
	ConVarRef furyFX("g_draw_fury_effects");
	ConVarRef vsync("mat_vsync");
	ConVarRef multicore("mat_queue_mode");
	ConVarRef muzzlelights("r_muzzleflash_lights");


	m_pMuzzleLights->SetSelected(muzzlelights.GetBool());
	m_pRainSlider->SetValue(raindensity.GetFloat());

	m_pParallax->SetSelected(parallax.GetBool());
	m_pVSync->SetSelected(vsync.GetBool());

	m_pFuryEffects->SetSelected(furyFX.GetBool());
	m_pVideoQuality->ActivateItem(r_videoquality.GetInt());
	m_pMulticore->SetSelected(multicore.GetInt() == 2);

	m_pEfficientParticles->SetSelected(r_efficient_particles.GetBool());

	wchar_t tempstring[128];
	char str[128];
	sprintf(str, "%i", m_pRainSlider->GetValue());
	g_pVGuiLocalize->ConvertANSIToUnicode(str, tempstring, sizeof(tempstring));
	m_pRainValue->SetText(tempstring);
}


void CHLRSubBonusOptionsGraphics::OnPageShow()
{
	OnResetData();
}
void CHLRSubBonusOptionsGraphics::OnThink()
{
	BaseClass::OnThink();

	wchar_t tempstring[128];
	char str[128];
	sprintf(str, "%i", m_pRainSlider->GetValue());
	g_pVGuiLocalize->ConvertANSIToUnicode(str, tempstring, sizeof(tempstring));
	m_pRainValue->SetText(tempstring);

}
void CHLRSubBonusOptionsGraphics::ApplyVideoQuality()
{
	ConVarRef r_rootlod("r_rootlod");
	ConVarRef mat_picmip("mat_picmip");
	ConVarRef mat_trilinear("mat_trilinear");
	ConVarRef mat_forceaniso("mat_forceaniso");
	ConVarRef mat_antialias("mat_antialias");
	ConVarRef mat_aaquality("mat_aaquality");
	ConVarRef r_flashlightdepthtexture("r_flashlightdepthtexture");
	ConVarRef r_shadowrendertotexture("r_shadowrendertotexture");
	ConVarRef r_waterforceexpensive("r_waterforceexpensive");
	ConVarRef r_waterforcereflectentities("r_waterforcereflectentities");
	ConVarRef parallax("mat_pbr_parallaxmap");

	engine->ClientCmd("mat_reloadallmaterials\n");

	r_videoquality.SetValue(m_pVideoQuality->GetActiveItem());

	switch (m_pVideoQuality->GetActiveItem())
	{
	case VQ_LOW:
		r_rootlod.SetValue(2);
		mat_trilinear.SetValue(1);
		mat_forceaniso.SetValue(0);
		mat_antialias.SetValue(0);
		mat_aaquality.SetValue(0);
		mat_picmip.SetValue(2);
		r_flashlightdepthtexture.SetValue(0);
		r_shadowrendertotexture.SetValue(0);
		parallax.SetValue(0);
		r_efficient_particles.SetValue(1);
		break;
	case VQ_MEDIUM:
		r_rootlod.SetValue(1);
		mat_trilinear.SetValue(0);
		mat_forceaniso.SetValue(2);
		mat_antialias.SetValue(2);
		mat_aaquality.SetValue(2);
		mat_picmip.SetValue(1);
		r_flashlightdepthtexture.SetValue(0);
		r_shadowrendertotexture.SetValue(1);
		parallax.SetValue(0);
		r_efficient_particles.SetValue(1);
		break;
	case VQ_HIGH:
		r_rootlod.SetValue(0);
		mat_trilinear.SetValue(0);
		mat_forceaniso.SetValue(8);
		mat_antialias.SetValue(4);
		mat_aaquality.SetValue(4);
		mat_picmip.SetValue(0);
		r_flashlightdepthtexture.SetValue(1);
		r_shadowrendertotexture.SetValue(1);
		parallax.SetValue(1);
		r_efficient_particles.SetValue(0);
		break;
	case VQ_MAX:
		r_rootlod.SetValue(0);
		mat_trilinear.SetValue(0);
		mat_forceaniso.SetValue(16);
		mat_antialias.SetValue(8);
		mat_aaquality.SetValue(8);
		mat_picmip.SetValue(-1);
		r_flashlightdepthtexture.SetValue(1);
		r_shadowrendertotexture.SetValue(1);
		parallax.SetValue(1);
		r_efficient_particles.SetValue(0);
		break;
	default:
		break;
	}
}
void CHLRSubBonusOptionsGraphics::OnApplyChanges()
{
	ConVarRef parallax("mat_pbr_parallaxmap");
	ConVarRef raindensity("r_RainSplashPercentage");
	ConVarRef furyFX("g_draw_fury_effects");
	ConVarRef vsync("mat_vsync");
	ConVarRef muzzlelights("r_muzzleflash_lights");
	vsync.SetValue(m_pVSync->IsSelected());
	raindensity.SetValue(m_pRainSlider->GetValue());
	parallax.SetValue(m_pParallax->IsSelected());
	furyFX.SetValue(m_pFuryEffects->IsSelected());
	muzzlelights.SetValue(m_pMuzzleLights->IsSelected());
	r_efficient_particles.SetValue(m_pEfficientParticles->IsSelected());


	m_pMulticore->IsSelected() ? engine->ClientCmd_Unrestricted("mat_queue_mode 2") : engine->ClientCmd_Unrestricted("mat_queue_mode 0");

	if(m_pVideoQuality->GetActiveItem() != r_videoquality.GetInt())
		ApplyVideoQuality();


	OnResetData();
}

void CHLRSubBonusOptionsGraphics::OnCheckButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

void CHLRSubBonusOptionsGraphics::OnDataChanged()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

void CHLRSubBonusOptionsGraphics::OnMenuItemSelected()
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
	CheckButton* m_classicpos;


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

	m_classicpos = new CheckButton(this, "ClassicPos","ClassicPos");

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
	ConVarRef classicpos("r_classic_weapon_pos");
	m_pGuts->SetSelected(guts.GetBool());
	m_pGibs->SetSelected(gibs.GetBool());
	m_pClassic->SetSelected(classic.GetBool());
	m_pRocket->SetSelected(rocket.GetBool());
	m_prainbow->SetSelected(rainbow.GetBool());
	m_pRainbowRate->SetValue(rainbowrate.GetFloat());
	m_classicpos->SetSelected(classicpos.GetBool());
}
void CHLRSubBonusOptions::OnApplyChanges()
{
	ConVarRef gibs("g_ultragibs");
	ConVarRef classic("mat_classic_render");
	ConVarRef guts("g_guts_and_glory");
	ConVarRef rocket("g_rocket_jump_mania");
	ConVarRef rainbow("hud_rainbow");
	ConVarRef rainbowrate("hud_rainbow_rate");
	ConVarRef classicpos("r_classic_weapon_pos");

	gibs.SetValue(m_pGibs->IsSelected());
	classic.SetValue(m_pClassic->IsSelected());
	guts.SetValue(m_pGuts->IsSelected());
	rocket.SetValue(m_pRocket->IsSelected());
	rainbow.SetValue(m_prainbow->IsSelected());
	classicpos.SetValue(m_classicpos->IsSelected());

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
	vgui::CheckButton* m_pcustomnv;
	vgui::Slider* m_sHudR;
	vgui::Slider* m_sHudG;
	vgui::Slider* m_sHudB;
	vgui::Panel* m_pCrosshair;
	vgui::CheckButton* m_poverride;
	vgui::ComboBox* m_movement;
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
	m_pcustomnv = new CheckButton(this, "customNV", "custom nightvision");

	m_movement = new ComboBox(this, "AirMovementType", 2, false);
	m_movement->AddItem("#HLR_MovementType_Source", NULL);
	m_movement->AddItem("#HLR_MovementType_Accel", NULL);


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
	ConVarRef customnv("g_custom_nightvision");
	ConVarRef movementtype("g_airmovement_type");

	iautoaimscale = autoaimscale.GetFloat() * 10;

	m_pautoaimscale->SetValue(iautoaimscale);

	m_pautoaim->SetSelected(autoaim.GetBool());
	m_pthirdperson->SetSelected(thirdperson.GetBool());
	m_pslowmo->SetSelected(slowmo.GetBool());

	m_sHudR->SetValue(hudr.GetFloat());
	m_sHudG->SetValue(hudg.GetFloat());
	m_sHudB->SetValue(hudb.GetFloat());

	m_poverride->SetSelected(haoverride.GetBool());

	m_pcustomnv->SetSelected(customnv.GetBool());
	m_movement->ActivateItem(movementtype.GetInt());
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
	ConVarRef customnv("g_custom_nightvision");
	ConVarRef movementtype("g_airmovement_type");

	iautoaimscale = m_pautoaimscale->GetValue();

	float scale = iautoaimscale / 10.0f;

	autoaimscale.SetValue(scale);

	haoverride.SetValue(m_poverride->IsSelected());
	slowmo.SetValue(m_pslowmo->IsSelected());
	autoaim.SetValue(m_pautoaim->IsSelected());

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

	customnv.SetValue(m_pcustomnv->IsSelected());
	movementtype.SetValue(m_movement->GetActiveItem());

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
#ifndef DEMO_DLL

#endif
///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
////////// BONUS OPTIONS DIALOG //////////////////////////
//////////////////////////////////////////////////////
///////////////////////////////////////////////

/*class CHLRWeaponWheelCustomization : public PropertyPage
{
public:
	DECLARE_CLASS_SIMPLE(CHLRWeaponWheelCustomization, PropertyPage);

};*/

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



void ReadGameDirectory(const CCommand& args)
{
	char gamedir[MAX_PATH];
	char video_service_path[MAX_PATH];
	Q_snprintf(video_service_path, sizeof(video_service_path), "%s\\bin\\video_services.dll", engine->GetGameDirectory());
	Q_snprintf(gamedir, sizeof(gamedir), engine->GetGameDirectory());
	Msg("GameDir: %s", gamedir);
	Msg("TestDir: %s\n", video_service_path);
}
static ConCommand readgamedir("readgamedir", ReadGameDirectory);