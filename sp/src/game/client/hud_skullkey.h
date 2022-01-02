#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudPurpleSkullKey : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudPurpleSkullKey, Panel);

public:
	CHudPurpleSkullKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	virtual void Init(void);
	void		LevelInit(void);
	void VidInit();

	void MsgFunc_PurpleSkullKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nPurpleSkullKey;
	CHudTexture *icon_skullkey;
};

class CHudRedSkullKey : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudRedSkullKey, Panel);

public:
	CHudRedSkullKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	virtual void Init(void);
	void		LevelInit(void);
	void VidInit();

	void MsgFunc_RedSkullKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nRedSkullKey;
	CHudTexture *icon_skullkey;
};

class CHudBlueSkullKey : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudRedSkullKey, Panel);

public:
	CHudBlueSkullKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	void VidInit();
	virtual void Init(void);
	void		LevelInit(void);

	void MsgFunc_BlueSkullKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nBlueSkullKey;
	CHudTexture *icon_skullkey;
};