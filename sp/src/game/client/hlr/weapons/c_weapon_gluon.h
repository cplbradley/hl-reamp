#ifndef C_GLUON_H
#define C_GLUON_H

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "vgui_controls/Panel.h"

#ifdef _WIN32
#pragma once
#endif

class C_GluonHudDot : public CHudElement, vgui::Panel
{
	DECLARE_CLASS_SIMPLE(C_GluonHudDot, vgui::Panel);
public:
	C_GluonHudDot(const char* ElementName);
	virtual bool ShouldDraw();
	virtual void Paint();
	virtual void Init();
	virtual void OnThink();
	bool bUseHoverPoint;
	bool bDraw;
	Vector m_vecHoverPoint;

	float fLerp;

private:
	CHudTexture* pDot;
	Vector m_vecDrawPos;
	
};


#endif
