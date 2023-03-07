#ifndef HUD_ITEMPICKUP_H
#pragma once
#define HUD_ITEMPICKUP_H

#include "hudelement.h"
#include "ehandle.h"

#include <vgui_controls/Panel.h>

namespace vgui
{
	class IScheme;
}

class C_BaseCombatWeapon;


class CHudItemPickup : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudItemPickup, vgui::Panel);

private:
	struct ITEM_STRUCT
	{
		ITEM_STRUCT()
		{
			DrawTime = 0.0f;
		}
		char* iID;
		int iAmount = 0;
		float DrawTime;
		wchar_t *icon;

	};

	CUtlVector<ITEM_STRUCT> itemList;

	



public:
	CHudItemPickup(const char* pElementName);
	virtual void Init();
	virtual void Paint();
	virtual void Reset();
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

	void	MsgFunc_ItemPickup(bf_read& msg);
	void GetIcons();


private:
	CPanelAnimationVarAliasType(float, m_flIconInset, "icon_inset", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flTextInset, "text_inset", "32", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HealthBarValues");
	CPanelAnimationVar(vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsSmall");
};
#endif
