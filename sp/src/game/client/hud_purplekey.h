#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudPurpleKey : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudPurpleKey, Panel);

public:
	CHudPurpleKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	virtual void Init(void);

	void MsgFunc_PurpleKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nPurpleKey;
};

class CHudKeyPanel : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudKeyPanel, Panel);

public:
	CHudKeyPanel(const char *pElementName);
	virtual void Init(void);
protected: 
	virtual void Paint();
};