#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include "string_t.h"
#include "mathlib/mathlib.h"
#include "globalstate.h"
#include "ndebugoverlay.h"
#include "saverestore_utlvector.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "gameinterface.h"

#include "tier0/memdbgon.h"


class CUtilHudBlocker : public CLogicalEntity
{
	DECLARE_CLASS(CUtilHudBlocker, CLogicalEntity);
	DECLARE_SERVERCLASS();
public:
	void Spawn(void);
	void InputEnable(inputdata_t &inputdata);
	void InputDisable(inputdata_t &inputdata);

	CNetworkVar(bool, bEnabled);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(util_hudblocker, CUtilHudBlocker);

IMPLEMENT_SERVERCLASS_ST(CUtilHudBlocker, DT_HudBlocker)
SendPropBool(SENDINFO(bEnabled)),
END_SEND_TABLE();

BEGIN_DATADESC(CUtilHudBlocker)
DEFINE_INPUTFUNC(FIELD_VOID, "Enable",InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_FIELD(bEnabled,FIELD_BOOLEAN),
END_DATADESC()

void CUtilHudBlocker::Spawn(void)
{
	bEnabled = false;
}

void CUtilHudBlocker::InputEnable(inputdata_t &inputdata)
{
	bEnabled = true;
}
void CUtilHudBlocker::InputDisable(inputdata_t &inputdata)
{
	bEnabled = false;
}
