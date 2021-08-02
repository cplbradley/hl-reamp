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

	void MsgFunc_PurpleSkullKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nPurpleSkullKey;
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

	void MsgFunc_RedSkullKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nRedSkullKey;
};

class CHudBlueSkullKey : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudRedSkullKey, Panel);

public:
	CHudBlueSkullKey(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	virtual void Init(void);
	void		LevelInit(void);

	void MsgFunc_BlueSkullKey(bf_read &msg);

	bool bShowKey;
protected:
	virtual void Paint();
	int m_nBlueSkullKey;
};