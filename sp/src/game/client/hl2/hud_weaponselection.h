#ifndef HUD_WEAPONSELECTION_H
#define HUD_WEAPONSELECTION_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "weapon_selection.h"

class CHudWeaponSelection : public CBaseHudWeaponSelection, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudWeaponSelection, vgui::Panel);

public:
	CHudWeaponSelection(const char* pElementName);

	virtual bool ShouldDraw();
	virtual void OnWeaponPickup(C_BaseCombatWeapon* pWeapon);

	virtual void CycleToNextWeapon(void);
	virtual void CycleToPrevWeapon(void);

	virtual C_BaseCombatWeapon* GetWeaponInSlot(int iSlot, int iSlotPos);
	virtual void SelectWeaponSlot(int iSlot);

	virtual C_BaseCombatWeapon* GetSelectedWeapon(void)
	{
		return m_hSelectedWeapon;
	}

	virtual void OpenSelection(void);
	virtual void HideSelection(void);

	virtual void LevelInit();

protected:
	virtual void OnThink();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

	virtual bool IsWeaponSelectable()
	{
		if (IsInSelectionMode())
			return true;

		return false;
	}

	virtual void SetWeaponSelected()
	{
		CBaseHudWeaponSelection::SetWeaponSelected();

		switch (hud_fastswitch.GetInt())
		{
		case HUDTYPE_FASTSWITCH:
		case HUDTYPE_CAROUSEL:
			ActivateFastswitchWeaponDisplay(GetSelectedWeapon());
			break;
		case HUDTYPE_PLUS:
			ActivateWeaponHighlight(GetSelectedWeapon());
			break;
		default:
			// do nothing
			break;
		}
	}

private:
	C_BaseCombatWeapon* FindNextWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition);
	C_BaseCombatWeapon* FindPrevWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition);

	void DrawLargeWeaponBox(C_BaseCombatWeapon* pWeapon, bool bSelected, int x, int y, int wide, int tall, Color color, float alpha, int number);
	void ActivateFastswitchWeaponDisplay(C_BaseCombatWeapon* pWeapon);
	void ActivateWeaponHighlight(C_BaseCombatWeapon* pWeapon);
	float GetWeaponBoxAlpha(bool bSelected);
	int GetLastPosInSlot(int iSlot) const;

	void FastWeaponSwitch(int iWeaponSlot);
	void PlusTypeFastWeaponSwitch(int iWeaponSlot);

	virtual	void SetSelectedWeapon(C_BaseCombatWeapon* pWeapon)
	{
		m_hSelectedWeapon = pWeapon;
	}

	virtual	void SetSelectedSlot(int slot)
	{
		m_iSelectedSlot = slot;
	}

	void SetSelectedSlideDir(int dir)
	{
		m_iSelectedSlideDir = dir;
	}

	void DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha, int number);

	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HudSelectionNumbers");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudSelectionText");
	CPanelAnimationVar(float, m_flBlur, "Blur", "0");

	CPanelAnimationVarAliasType(float, m_flSmallBoxSize, "SmallBoxSize", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flLargeBoxWide, "LargeBoxWide", "108", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flLargeBoxTall, "LargeBoxTall", "72", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flMediumBoxWide, "MediumBoxWide", "75", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flMediumBoxTall, "MediumBoxTall", "50", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flBoxGap, "BoxGap", "12", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flSelectionNumberXPos, "SelectionNumberXPos", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flSelectionNumberYPos, "SelectionNumberYPos", "4", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flTextYPos, "TextYPos", "54", "proportional_float");

	CPanelAnimationVar(float, m_flAlphaOverride, "Alpha", "0");
	CPanelAnimationVar(float, m_flSelectionAlphaOverride, "SelectionAlpha", "0");

	//CPanelAnimationVar( Color, m_TextColor, "TextColor", "SelectionTextFg" );
	//CPanelAnimationVar( Color, m_NumberColor, "NumberColor", "SelectionNumberFg" );

	Color m_NumberColor;
	Color m_TextColor;
	CPanelAnimationVar(Color, m_EmptyBoxColor, "EmptyBoxColor", "SelectionEmptyBoxBg");
	CPanelAnimationVar(Color, m_BoxColor, "BoxColor", "SelectionBoxBg");
	CPanelAnimationVar(Color, m_SelectedBoxColor, "SelectedBoxColor", "SelectionSelectedBoxBg");
	CPanelAnimationVar(Color, m_SelectedFgColor, "SelectedFgColor", "FgColor");
	CPanelAnimationVar(Color, m_BrightBoxColor, "SelectedFgColor", "BgColor");

	CPanelAnimationVar(float, m_flWeaponPickupGrowTime, "SelectionGrowTime", "0.1");

	CPanelAnimationVar(float, m_flTextScan, "TextScan", "1.0");

	bool m_bFadingOut;

	// fastswitch weapon display
	struct WeaponBox_t
	{
		int m_iSlot;
		int m_iSlotPos;
	};
	CUtlVector<WeaponBox_t>	m_WeaponBoxes;
	int						m_iSelectedWeaponBox;
	int						m_iSelectedSlideDir;
	int						m_iSelectedBoxPosition;
	int						m_iSelectedSlot;
	C_BaseCombatWeapon* m_pLastWeapon;
	CPanelAnimationVar(float, m_flHorizWeaponSelectOffsetPoint, "WeaponBoxOffset", "0");
};

#endif
