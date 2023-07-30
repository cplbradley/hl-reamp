#if !defined HUD_HEALTHBAR_H
#define HUD_HEALTHBAR_H 

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"

//-----------------------------------------------------------------------------
// Purpose: Shows the hull bar
//-----------------------------------------------------------------------------


class CHudHealthBar1 : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudHealthBar1, vgui::Panel);

public:
	CHudHealthBar1(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	virtual bool ShouldDraw(void) { return bIsAlive; }
	bool SetBarLabelName(const char* text);


	int GetCorrectPosition();

	void MsgFunc_EnemyHealth1(bf_read &msg);
	bool bIsAlive;


protected:
	virtual void Paint();

private:
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "255 0 0 255");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "1", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudHealthbarText");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");

	char chName[256];
	float m_flHull;
	float m_flMaxHull;
	float gHealth1;
	float gMaxHealth1;

	bool bStartAnimDrawn;

	Vector vecColor;

	int m_nHullLow;
	bool drawanim;
	float animbar;
	float interpfrac;

};
class CHudHealthBar2 : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudHealthBar2, vgui::Panel);

public:
	CHudHealthBar2(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	virtual bool ShouldDraw(void) { return bIsAlive; }

	bool bIsAlive;
	void MsgFunc_EnemyHealth2(bf_read &msg);
	bool SetBarLabelName(const char* text);
	int GetCorrectPosition();


protected:
	virtual void Paint();

private:
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "255 0 0 255");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "1", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudHealthbarText");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");
	
	float m_flHull;
	float m_flMaxHull;
	float gHealth2;
	float gMaxHealth2;

	bool bStartAnimDrawn;

	int m_nHullLow;
	bool drawanim;
	float animbar;
	Vector vecColor;
	char chName[256];
	float interpfrac;

};
class CHudHealthBar3 : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudHealthBar3, vgui::Panel);

public:
	CHudHealthBar3(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	virtual bool ShouldDraw(void) { return bIsAlive; }
	bool bIsAlive;
	
	void MsgFunc_EnemyHealth3(bf_read &msg);
	bool SetBarLabelName(const char* text);
	int GetCorrectPosition();

protected:
	virtual void Paint();

private:
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "255 0 0 255");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "13", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "1", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudHealthbarText");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");
	float m_flHull;
	float m_flMaxHull;
	float gHealth3;
	float gMaxHealth3;

	bool bStartAnimDrawn;
	

	Vector vecColor;
	int m_nHullLow;
	bool drawanim;
	float animbar;
	char chName[256];
	float interpfrac;
};
inline int interpPos(int start, int end, float frac);
inline int iNumActive();

#endif // HUD_SUITPOWER_H