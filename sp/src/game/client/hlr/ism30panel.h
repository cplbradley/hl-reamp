#if !defined(ISM30PANEL_H)
#define ISM30PANEL_H
#pragma once
#endif

#include <vgui/VGUI.h>

namespace vgui
{
	class Panel;
}
abstract_class ISM30ErrorPanel
{
public:
	virtual void		Create(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
};

extern ISM30ErrorPanel* sm30p;
