#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_healthbar.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "vgui\ILocalize.h"
#include <vgui/IVGui.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/AnimationController.h>

using namespace vgui;

class C_HudEnemyCounter : public CHudElement, Panel
{
	DECLARE_CLASS_SIMPLE(C_HudEnemyCounter, Panel);
public:
	C_HudEnemyCounter(const char* pElementName);

	virtual void Init();
	virtual void Reset();
	virtual void Paint();
	virtual bool ShouldDraw();
	virtual void OnThink();

	void MsgFunc_EnemyCount(bf_read& msg);
	void Show();
	void Hide();

	const char* GetDisplayText();

	void Blip();
	int iCount;
	bool m_bShouldDraw;
	int iPreviousCount;
	int iGlowAlpha;

	int iThreshold;

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudWeaponWheelText");
	CPanelAnimationVar(vgui::HFont, m_hGlowFont, "GlowFont", "HudWeaponWheelTextGlow");
};

DECLARE_HUDELEMENT(C_HudEnemyCounter);
DECLARE_HUD_MESSAGE(C_HudEnemyCounter, EnemyCount);


C_HudEnemyCounter::C_HudEnemyCounter(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudEnemyCounter")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(true);
	SetEnabled(true);
	SetActive(true);
	SetPaintBackgroundEnabled(true);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
	Init();
}

void C_HudEnemyCounter::Reset()
{
	Init();
}

void C_HudEnemyCounter::Init()
{
	SetAlpha(0);
	m_bShouldDraw = false;
	iCount = 0;
	iPreviousCount = 0;
	iGlowAlpha = 0;
	HOOK_HUD_MESSAGE(C_HudEnemyCounter, EnemyCount);
}
const char* C_HudEnemyCounter::GetDisplayText()
{
	return "";
}

void C_HudEnemyCounter::OnThink()
{
	if (iPreviousCount != iCount)
	{
		Blip();
	}

	iGlowAlpha--;

	if (iGlowAlpha < 0)
		iGlowAlpha = 0;

	if (iCount <= 0 && m_bShouldDraw)
		Hide();

	if (iCount > 0 && iCount <= iThreshold && !m_bShouldDraw)
		Show();

}
void C_HudEnemyCounter::Paint()
{
	Color glow = Color(gHUD.GetDefaultColor().r(), gHUD.GetDefaultColor().g(), gHUD.GetDefaultColor().b(), iGlowAlpha);

	const char* string = g_pVGuiLocalize->FindAsUTF8("#HLR_Enemies_Left");
	char txt[64];
	Q_strncpy(txt, string, sizeof(txt));
	wchar_t wchar[64];
	sprintf(txt + strlen(txt), "%i", iCount);

	g_pVGuiLocalize->ConvertANSIToUnicode(txt, wchar, sizeof(wchar));

	surface()->DrawSetTextColor(gHUD.GetDefaultColor());
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(1, 1);
	surface()->DrawUnicodeString(wchar);

	surface()->DrawSetTextColor(glow);
	surface()->DrawSetTextFont(m_hGlowFont);
	surface()->DrawSetTextPos(1, 1);
	surface()->DrawUnicodeString(wchar);
}

bool C_HudEnemyCounter::ShouldDraw()
{
	return true;
}

void C_HudEnemyCounter::Blip()
{
	iGlowAlpha = 255;
	iPreviousCount = iCount;
}

void C_HudEnemyCounter::Show()
{
	m_bShouldDraw = true;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowEnemyCounter");
}
void C_HudEnemyCounter::Hide()
{
	m_bShouldDraw = false;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HideEnemyCounter");
}

void C_HudEnemyCounter::MsgFunc_EnemyCount(bf_read& msg)
{
	iCount = msg.ReadByte();
	iThreshold = msg.ReadByte();
	Msg("Data Msg Recieved, Count %i Threshold %i\n", iCount, iThreshold);
}

///////////////////////////
//////////////////////////
/////////////////////////


/*class C_EnemyCounter : public C_BaseEntity
{
	DECLARE_CLASS(C_EnemyCounter, C_BaseEntity);
	DECLARE_CLIENTCLASS();

public:
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink();
	void UpdateData();
	C_EnemyCounter() {};

private:
	bool m_bShouldDraw;
	int m_iEnemyCount;
};

IMPLEMENT_CLIENTCLASS_DT(C_EnemyCounter, DT_EnemyCounter, CEnemyCounter)
RecvPropInt(RECVINFO(m_iEnemyCount)),
RecvPropBool(RECVINFO(m_bShouldDraw)),
END_RECV_TABLE();


void C_EnemyCounter::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
		SetNextClientThink(CLIENT_THINK_ALWAYS);

	BaseClass::OnDataChanged(type);
}

void C_EnemyCounter::UpdateData()
{
	C_HudEnemyCounter* hud = GET_HUDELEMENT(C_HudEnemyCounter);

	if (!hud)
		return;

	hud->iCount = m_iEnemyCount;
	
	if (m_bShouldDraw && !hud->m_bShouldDraw)
		hud->Show();
	else if (!m_bShouldDraw && hud->m_bShouldDraw)
		hud->Hide();

	Msg("Client Data Update\n");
}

void C_EnemyCounter::ClientThink()
{
	BaseClass::ClientThink();

	UpdateData();
}

*/
