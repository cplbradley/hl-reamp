#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudRedKey : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudRedKey, Panel);

public:
	CHudRedKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();

protected:
	virtual void Paint();
	int m_nRedKey;
};