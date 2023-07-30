#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;


class CHudEnvironmentResistToxic : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudEnvironmentResistToxic, Panel);

public:
	CHudEnvironmentResistToxic(const char *pElementName);

	void TogglePrint(void);
	void OnThink(void);
	void LevelInit(void);
	void VidInit(void);
	void Init(void);
	bool bShowIcon;
	int m_iDmgType;
	int m_iDamageLeft;
	int m_iMaxPoints;

	void InitAnimation(void);
	void DamageAnimation(void);

protected:
	virtual void Paint();
	CHudTexture *m_icon1;
	CHudTexture *m_icon2;
	CHudTexture *m_empty;
	int alpha;


private:
	CPanelAnimationVarAliasType(float, m_IconX, "icon_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_IconY, "icon_ypos", "2", "proportional_float");
};
class CHudEnvironmentResistFire : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudEnvironmentResistFire, Panel);

public:
	CHudEnvironmentResistFire(const char *pElementName);

	void TogglePrint(void);
	void OnThink(void);
	void LevelInit(void);
	void VidInit(void);
	void Init(void);
	bool bShowIcon;
	int m_iDmgType;
	int m_iDamageLeft;
	int m_iMaxPoints;

protected:
	virtual void Paint();
	CHudTexture *m_icon1;
	CHudTexture *m_icon2;
	CHudTexture *m_empty;
	int alpha;
	Color clrBurn;
	Color clrBurnBG;
	Color clrBurnGlow;

private:
	CPanelAnimationVarAliasType(float, m_IconX, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_IconY, "icon_ypos", "0", "proportional_float");
};
class CHudEnvironmentResistElectric : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudEnvironmentResistElectric, Panel);

public:
	CHudEnvironmentResistElectric(const char *pElementName);

	void TogglePrint(void);
	void OnThink(void);
	void LevelInit(void);
	void VidInit(void);
	void Init(void);
	bool bShowIcon;
	int m_iDmgType;
	int m_iDamageLeft;
	int m_iMaxPoints;

protected:
	virtual void Paint();
	CHudTexture *m_icon1;
	CHudTexture *m_icon2;
	CHudTexture *m_empty;
	int alpha;



private:
	CPanelAnimationVarAliasType(float, m_IconX, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_IconY, "icon_ypos", "0", "proportional_float");
};