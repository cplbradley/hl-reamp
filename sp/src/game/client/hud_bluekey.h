#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudBlueKey : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudBlueKey, Panel);

public:
	CHudBlueKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	virtual void Init(void);
	void VidInit(void);

	void MsgFunc_BlueKey(bf_read &msg);

	bool bShowKey;

	void LevelInit(void);
protected:
	virtual void Paint();
	int m_nBlueKey;
	CHudTexture *m_icon;
};