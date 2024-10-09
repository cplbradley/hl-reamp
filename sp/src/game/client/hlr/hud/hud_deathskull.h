#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;


class CHudDeathScreen : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudDeathScreen, Panel);
public:
	CHudDeathScreen(const char* pElementName);
	virtual void OnThink();
	virtual void Init();
	virtual void VidInit();
	virtual void Paint();
	void MsgFunc_DeathScreen(bf_read& msg);
	bool SetKiller(const char* name);
private:
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudHugeText");
	CPanelAnimationVar(vgui::HFont, m_hGlowFont, "GlowFont", "HudHugeTextGlow");
	char szKiller[256];
	int alpha;
	int panelwidth;
	float fDrawTime;
};
class CHudDeathSkull : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudDeathSkull, Panel);

public:
	CHudDeathSkull(const char *pElementName);
	virtual void OnThink();
	virtual void VidInit(void);
	
protected:
	int m_health;
	virtual void Paint();
	CHudTexture *m_icon;
	
};