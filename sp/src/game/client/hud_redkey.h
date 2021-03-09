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
	virtual void Init(void);

	void MsgFunc_RedKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nRedKey;
};

DECLARE_HUDELEMENT(CHudRedKey);