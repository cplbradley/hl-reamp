#include "cbase.h"
#include "ai_basenpc.h"

#include "tier0/memdbgon.h"


class CNPCMemoryManager : public CBaseEntity
{
	DECLARE_CLASS(CNPCMemoryManager, CBaseEntity);
	DECLARE_DATADESC();
public:
	void Spawn();
	void UpdateMemory();
	void UpdateThink();

	float m_fQueryRange;
	float m_fUpdateFrequency;

	bool m_bAmUpdating;

	virtual void OnRestore();
	void InputStartUpdate(inputdata_t& inputdata);
	void InputStopUpdate(inputdata_t& inputdata);
	void InputUpdateOnce(inputdata_t& inputdata);
	void InputSetQueryRange(inputdata_t& inputdata);
	void InputSetUpdateFrequency(inputdata_t& inputdata);

};

LINK_ENTITY_TO_CLASS(npc_memory_manager, CNPCMemoryManager);

BEGIN_DATADESC(CNPCMemoryManager)
DEFINE_INPUTFUNC(FIELD_VOID,"EnableAutoUpdate",InputStartUpdate),
DEFINE_INPUTFUNC(FIELD_VOID,"DisableAutoUpdate",InputStopUpdate),
DEFINE_INPUTFUNC(FIELD_VOID,"UpdateOnce", InputUpdateOnce),
DEFINE_INPUTFUNC(FIELD_FLOAT,"SetQueryRange", InputSetQueryRange),
DEFINE_INPUTFUNC(FIELD_FLOAT,"SetUpdateFrequency", InputSetUpdateFrequency),
DEFINE_KEYFIELD(m_fQueryRange,FIELD_FLOAT,"QueryRange"),
DEFINE_KEYFIELD(m_fUpdateFrequency,FIELD_FLOAT,"UpdateFrequency"),
DEFINE_FUNCTION(UpdateMemory),
DEFINE_FIELD(m_bAmUpdating,FIELD_BOOLEAN),
END_DATADESC()


void CNPCMemoryManager::Spawn()
{
	m_bAmUpdating = false;
}

void CNPCMemoryManager::UpdateMemory()
{
	if (!m_bAmUpdating)
		return;

	CAI_BaseNPC* pNPC;
	CBaseEntity* pSearch = nullptr;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	if (!pPlayer)
		return;

	float range = m_fQueryRange;

	if (range < 0.0f)
		range = 16384.0f;

	for (CEntitySphereQuery query(GetAbsOrigin(), range, FL_NPC); (pSearch = query.GetCurrentEntity()) != nullptr; query.NextEntity())
	{
		if (this == pSearch)
			continue;

		pNPC = pSearch->MyNPCPointer();
		if (!pNPC)
			continue;

		pNPC->UpdateEnemyMemory(pPlayer, pPlayer->GetAbsOrigin());
	}
}

void CNPCMemoryManager::UpdateThink()
{
	UpdateMemory();
	SetNextThink(gpGlobals->curtime + m_fUpdateFrequency);
}

void CNPCMemoryManager::InputStartUpdate(inputdata_t& inputdata)
{
	SetThink(&CNPCMemoryManager::UpdateThink);
	SetNextThink(gpGlobals->curtime);
	m_bAmUpdating = true;
}

void CNPCMemoryManager::InputStopUpdate(inputdata_t& inputdata)
{
	SetThink(NULL);
	SetNextThink(gpGlobals->curtime);
	m_bAmUpdating = false;
}

void CNPCMemoryManager::InputUpdateOnce(inputdata_t& inputdata)
{
	UpdateMemory();
}

void CNPCMemoryManager::InputSetQueryRange(inputdata_t& inputdata)
{
	m_fQueryRange = inputdata.value.Float();
}

void CNPCMemoryManager::InputSetUpdateFrequency(inputdata_t& inputdata)
{
	m_fUpdateFrequency = inputdata.value.Float();
}

void CNPCMemoryManager::OnRestore()
{
	BaseClass::OnRestore();

	if (m_bAmUpdating)
	{
		SetThink(&CNPCMemoryManager::UpdateThink);
		SetNextThink(gpGlobals->curtime);
	}
	else
	{
		SetThink(NULL);
		SetNextThink(gpGlobals->curtime);
	}
}