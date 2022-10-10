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
};

CHLRSubBonusOptionsGraphics::CHLRSubBonusOptionsGraphics(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	m_pParallax = new CheckButton(this, "Parallax", "Parallax Occlusion Mapping");

	m_pRainSlider = new Slider(this, "RainDensity");
	m_pRainSlider->SetRange(0, 100);
	LoadControlSettings("resource/ui/bonusoptionsgraphics.res");
}

void CHLRSubBonusOptionsGraphics::OnResetData()
{
	ConVarRef parallax("mat_pbr_parallaxmap");
	ConVarRef raindensity("r_RainSplashPercentage");
	m_pRainSlider->SetValue(raindensity.GetFloat());
	if (parallax.GetBool())
		m_pParallax->SetSelected(true);
	else
		m_pParallax->SetSelected(false);
}

void CHLRSubBonusOptionsGraphics::OnApplyChanges()
{
	ConVarRef parallax("mat_pbr_parallaxmap");
	ConVarRef raindensity("r_RainSplashPercentage");
	raindensity.SetValue(m_pRainSlider->GetValue());
	if (m_pParallax->IsSelected())
		parallax.SetValue(1);
	else
		parallax.SetValue(0);
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
	vgui::CheckButton* m_pGibs;
	vgui::CheckButton* m_pClassic;
	vgui::CheckButton* m_pGuts;

};


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

	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked");

private:
	vgui::Slider* m_pautoaimscale;
	vgui::CheckButton* m_pautoaim;
	vgui::CheckButton* m_pthirdperson;
	vgui::CheckButton* m_pslowmo;
	int iautoaimscale;

};

CHLRSubBonusOptionsGameplay::CHLRSubBonusOptionsGameplay(vgui::Panel* parent) : BaseClass(parent, NULL)
{
	m_pautoaim = new CheckButton(this, "Autoaim", "Aim Assist");

	m_pslowmo = new CheckButton(this, "Slowmo", "Weapon Selection SlowMo");


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
	ConVarRef slowmo("hud_weapon_selection_slowmo");
	iautoaimscale = autoaimscale.GetFloat() * 10;

	m_pautoaimscale->SetValue(iautoaimscale);

	if (autoaim.GetBool())
		m_pautoaim->SetSelected(true);
	else
		m_pautoaim->SetSelected(false);

	if (thirdperson.GetBool())
		m_pthirdperson->SetSelected(true);
	else
		m_pthirdperson->SetSelected(false);

	if (slowmo.GetBool())
		m_pslowmo->SetSelected(true);
	else
		m_pslowmo->SetSelected(false);
}
void CHLRSubBonusOptionsGameplay::OnApplyChanges()
{
	ConVarRef autoaimscale("hud_autoaim_scale");
	ConVarRef autoaim("hud_draw_active_reticle");
	ConVarRef thirdperson("g_thirdperson");
	ConVarRef thirdpersoncrosshair("cl_thirdperson_crosshair");
	ConVarRef slowmo("hud_weapon_selection_slowmo");

	iautoaimscale = m_pautoaimscale->GetValue();

	float scale = iautoaimscale / 10.0f;

	autoaimscale.SetValue(scale);

	if (m_pslowmo->IsSelected())
		slowmo.SetValue(1);
	else
		slowmo.SetValue(0);

	if (m_pautoaim->IsSelected())
		autoaim.SetValue(1);
	else
		autoaim.SetValue(0);

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

	DevMsg("changing bonus option data\n");

}
void CHLRSubBonusOptionsGameplay::OnCheckButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
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

	AddPage(new CHLRSubBonusOptionsGameplay(this), "Gameplay");
	AddPage(new CHLRSubBonusOptionsGraphics(this), "Graphics");
	AddPage(new CHLRSubBonusOptions(this), "Extras");
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
