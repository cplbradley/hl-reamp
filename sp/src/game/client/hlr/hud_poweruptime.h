#include "hudelement.h"
#include <vgui_controls/Panel.h>



using namespace vgui;


//	____________ _____ ________  ___  ___   _   _  _____  ______ _   _________   __
//	|  ___| ___ \  ___|  ___|  \/  | / _ \ | \ | |/  ___| |  ___| | | | ___ \ \ / /
//	| |_  | |_/ / |__ | |__ | .  . |/ /_\ \|  \| |\ `--.  | |_  | | | | |_/ /\ V / 
//	|  _| |    /|  __||  __|| |\/| ||  _  || . ` | `--. \ |  _| | | | |    /  \ /  
//	| |   | |\ \| |___| |___| |  | || | | || |\  |/\__/ / | |   | |_| | |\ \  | |  
//	\_|   \_| \_\____/\____/\_|  |_/\_| |_/\_| \_/\____/  \_|    \___/\_| \_| \_/  
                                                                               
                                                                               

class CHudProgressFF : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudProgressFF, Panel);

public: 
	CHudProgressFF(const char *pElementName);

	void TogglePrint(void);
	void OnThink(void);
	void LevelInit(void);
	void VidInit(void);
	void Init(void);
	bool bShowIcon;
	int m_iTimeLeft;

protected:
	virtual void Paint();
	CHudTexture *m_icon_powerup;
	CHudTexture *m_icon_background;
	CHudTexture *m_icon_glow;
	CHudTexture *m_empty;

private:
	CPanelAnimationVar(vgui::HFont, m_hFont, "Font", "WeaponIconsSmall");
	CPanelAnimationVarAliasType(float, m_IconX, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_IconY, "icon_ypos", "0", "proportional_float");
};

//-------------------------------------------------------------------------------------------------//
//  ______   ____    ____  _______ .______       _______  .______       __  ____    ____  _______  //
// /  __  \  \   \  /   / |   ____||   _  \     |       \ |   _  \     |  | \   \  /   / |   ____| //
//|  |  |  |  \   \/   /  |  |__   |  |_)  |    |  .--.  ||  |_)  |    |  |  \   \/   /  |  |__    //
//|  |  |  |   \      /   |   __|  |      /     |  |  |  ||      /     |  |   \      /   |   __|   //
//|  `--'  |    \    /    |  |____ |  |\  \----.|  '--'  ||  |\  \----.|  |    \    /    |  |____  //
// \______/      \__/     |_______|| _| `._____||_______/ | _| `._____||__|     \__/     |_______| //
//-------------------------------------------------------------------------------------------------//


class CHudProgressOD : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudProgressOD, Panel);

public:
	CHudProgressOD(const char *pElementName);

	void TogglePrint(void);
	void OnThink(void);
	void LevelInit(void);
	void VidInit(void);
	void Init(void);
	bool bShowIcon;
	int m_iTimeLeft;

protected:
	virtual void Paint();
	CHudTexture *m_icon_powerup;
	CHudTexture *m_icon_background;
	CHudTexture *m_icon_glow;
	CHudTexture *m_empty;

private:

	CPanelAnimationVarAliasType(float, m_IconX, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_IconY, "icon_ypos", "0", "proportional_float");
};

class CHudProgressQJ : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudProgressQJ, Panel);

public:
	CHudProgressQJ(const char* pElementName);

	void TogglePrint(void);
	void OnThink(void);
	void LevelInit(void);
	void VidInit(void);
	void Init(void);
	bool bShowIcon;
	int m_iTimeLeft;

protected:
	virtual void Paint();
	CHudTexture* m_icon_powerup;
	CHudTexture* m_icon_background;
	CHudTexture* m_icon_glow;
	CHudTexture* m_empty;

private:

	CPanelAnimationVarAliasType(float, m_IconX, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_IconY, "icon_ypos", "0", "proportional_float");
};