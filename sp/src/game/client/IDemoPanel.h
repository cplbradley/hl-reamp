#if !defined(IDEMOPANEL_H)
#define IDEMOPANEL_H
#pragma once
#endif

#include <vgui/VGUI.h>

namespace vgui
{
	class Panel;
}

abstract_class IDemoPanel
{
public:
	virtual void		Create(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
};

extern IDemoPanel* demopanel;