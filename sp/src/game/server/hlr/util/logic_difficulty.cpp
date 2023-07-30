#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "gamerules.h"


extern ConVar skill;

class CLogicDifficultyRelay : public CLogicalEntity
{
public:
	DECLARE_CLASS(CLogicDifficultyRelay, CLogicalEntity);
	CLogicDifficultyRelay();

	void InputTrigger(inputdata_t &inputdata);

	COutputEvent m_OnEasy;
	COutputEvent m_OnNormal;
	COutputEvent m_OnHard;

	DECLARE_DATADESC();
};


LINK_ENTITY_TO_CLASS(logic_difficulty_relay, CLogicDifficultyRelay);

BEGIN_DATADESC(CLogicDifficultyRelay)
	DEFINE_INPUTFUNC(FIELD_VOID,"Trigger", InputTrigger),
	DEFINE_OUTPUT(m_OnEasy, "OnEasy"),
	DEFINE_OUTPUT(m_OnNormal, "OnNormal"),
	DEFINE_OUTPUT(m_OnHard, "OnHard"),
END_DATADESC()


CLogicDifficultyRelay::CLogicDifficultyRelay(void)
{
}


void CLogicDifficultyRelay::InputTrigger(inputdata_t &inputdata)
{
	int difficulty = g_pGameRules->GetSkillLevel();

	switch (difficulty){
	case SKILL_EASY:
	{
		m_OnEasy.FireOutput(inputdata.pActivator, this);
		break;
	}
	case SKILL_MEDIUM:
	{
		m_OnNormal.FireOutput(inputdata.pActivator, this);
		break;
	}
	case SKILL_HARD:
	{
		m_OnHard.FireOutput(inputdata.pActivator, this);
		break;
	}
	default:
	{
		Warning("i don't know what the fuck you did, but you managed to fire the relay without a difficulty. good job.\n");
		break;
	}
	}
}


class CDifficultyManager : public CLogicalEntity
{
	
public:
	DECLARE_CLASS(CDifficultyManager, CLogicalEntity);

	CDifficultyManager(){};

	void InputSetEasy(inputdata_t &inputdata);
	void InputSetNormal(inputdata_t &inputdata);
	void InputSetHard(inputdata_t &inputdata);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(difficulty_manager, CDifficultyManager);

BEGIN_DATADESC(CDifficultyManager)
DEFINE_INPUTFUNC(FIELD_VOID, "SetEasy", InputSetEasy),
DEFINE_INPUTFUNC(FIELD_VOID, "SetNormal", InputSetNormal),
DEFINE_INPUTFUNC(FIELD_VOID, "SetHard", InputSetHard),
END_DATADESC()


void CDifficultyManager::InputSetEasy(inputdata_t &inputdata)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	g_pGameRules->SetSkillLevel(SKILL_EASY);
	engine->ClientCommand(pPlayer->edict(), "skill 1");
}
void CDifficultyManager::InputSetNormal(inputdata_t &inputdata)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	g_pGameRules->SetSkillLevel(SKILL_MEDIUM);
	engine->ClientCommand(pPlayer->edict(), "skill 2");
}
void CDifficultyManager::InputSetHard(inputdata_t &inputdata)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	g_pGameRules->SetSkillLevel(SKILL_HARD);
	engine->ClientCommand(pPlayer->edict(), "skill 3");
}
