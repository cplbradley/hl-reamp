#include "cbase.h"
#include "triggers.h"


#include "tier0/memdbgon.h"

#define SF_TRIGGERDESTROY_STARTENABLED 0x0001
#define SF_TRIGGERDESTROY_KILL_NPC 0x0002
#define SF_TRIGGERDESTROY_DELETE_NPC 0x0004
#define SF_TRIGGERDESTROY_KILL_PLAYER 0x0008
#define SF_TRIGGERDESTROY_DELETE_OBJECTS 0x0010
#define SF_TRIGGERDESTROY_BREAK_OBJECTS 0x0020


class CTriggerDestroy : public CBaseTrigger
{
	DECLARE_CLASS(CTriggerDestroy, CBaseTrigger);
public:
	void Spawn();
	void Touch(CBaseEntity* pOther);
	void Enable(inputdata_t& data);
	void Disable(inputdata_t& data);

private:
	bool m_bEnabled;
	DECLARE_DATADESC();

};

LINK_ENTITY_TO_CLASS(trigger_destroy, CTriggerDestroy);

BEGIN_DATADESC(CTriggerDestroy)
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", Enable),
DEFINE_INPUTFUNC(FIELD_VOID,"Disable",Disable),
DEFINE_FIELD(m_bEnabled,FIELD_BOOLEAN)

END_DATADESC()

void CTriggerDestroy::Spawn()
{
	BaseClass::Spawn();
	BaseClass::InitTrigger();
	m_bEnabled = HasSpawnFlags(SF_TRIGGERDESTROY_STARTENABLED);
	SetTouch(&CTriggerDestroy::Touch);
}

void CTriggerDestroy::Touch(CBaseEntity* pOther)
{
	if (!m_bEnabled)
		return;

	if (!pOther)
		return;

	if (pOther->IsWorld())
		return;

	if (pOther->IsNPC())
	{
		if (HasSpawnFlags(SF_TRIGGERDESTROY_KILL_NPC))
			pOther->SetHealth(0);
		else if (HasSpawnFlags(SF_TRIGGERDESTROY_DELETE_NPC))
			UTIL_Remove(pOther);

		return;
	}

	if (pOther->IsPlayer())
	{
		if(HasSpawnFlags(SF_TRIGGERDESTROY_KILL_PLAYER))
			pOther->SetHealth(0);
		return;
	}

	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_USE_TRIGGER_BOUNDS))
		return;


	if (HasSpawnFlags(SF_TRIGGERDESTROY_BREAK_OBJECTS))
		pOther->SetHealth(0);
	else if (HasSpawnFlags(SF_TRIGGERDESTROY_DELETE_OBJECTS))
		UTIL_Remove(pOther);

}

void CTriggerDestroy::Enable(inputdata_t& data)
{
	m_bEnabled = true;
}
void CTriggerDestroy::Disable(inputdata_t& data)
{
	m_bEnabled = false;
}