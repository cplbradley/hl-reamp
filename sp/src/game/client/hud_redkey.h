#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudRedKey : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE(CHudRedKey, CHudNumericDisplay);

public:
	CHudRedKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	virtual void Init(void);
	void	VidInit(void);
	void LevelInit(void);
	void MsgFunc_RedKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	//int m_nRedKey;
	CHudTexture *m_nRedKey;
};

DECLARE_HUDELEMENT(CHudRedKey);