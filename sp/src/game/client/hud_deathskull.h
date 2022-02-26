#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

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