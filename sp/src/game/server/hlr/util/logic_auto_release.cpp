#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include "mathlib/mathlib.h"
#include "globalstate.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//This is a useful auto entity for firing different outputs based on a specified release state (i added it to skip intro cutscenes when working on levels without having remove anything)

class CLogicAutoDevelop : public CBaseEntity
{
public:
	DECLARE_CLASS(CLogicAutoDevelop, CBaseEntity);
	void Activate(void);
	void Think(void);

	int ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	DECLARE_DATADESC();
private:
	// fired no matter why the map loaded
	COutputEvent m_OnMapSpawnRelease;
	COutputEvent m_OnMapSpawnDevelop;

	// fired for specified types of map loads
	COutputEvent m_OnNewGameRelease;
	COutputEvent m_OnNewGameDevelop;
	COutputEvent m_OnLoadGameRelease;
	COutputEvent m_OnLoadGameDevelop;
	COutputEvent m_OnMapTransitionRelease;
	COutputEvent m_OnMapTransitionDevelop;
	COutputEvent m_OnBackgroundMapRelease;
	COutputEvent m_OnBackgroundMapDevelop;


	bool m_bRelease;
};

LINK_ENTITY_TO_CLASS(logic_auto_develop, CLogicAutoDevelop);
BEGIN_DATADESC(CLogicAutoDevelop)
	DEFINE_KEYFIELD(m_bRelease,FIELD_BOOLEAN,"ReleaseState"),
	DEFINE_OUTPUT(m_OnMapSpawnRelease,"OnMapSpawnRelease"),
	DEFINE_OUTPUT(m_OnMapSpawnDevelop, "OnMapSpawnDevelop"),
	DEFINE_OUTPUT(m_OnNewGameRelease, "OnNewGameRelease"),
	DEFINE_OUTPUT(m_OnNewGameDevelop, "OnNewGameDevelop"),
	DEFINE_OUTPUT(m_OnLoadGameRelease, "OnLoadGameRelease"),
	DEFINE_OUTPUT(m_OnLoadGameDevelop, "OnLoadGameDevelop"),
	DEFINE_OUTPUT(m_OnMapTransitionRelease, "OnMapTransitionRelease"),
	DEFINE_OUTPUT(m_OnMapTransitionDevelop, "OnMapTransitionDevelop"),
	DEFINE_OUTPUT(m_OnBackgroundMapRelease, "OnBackgroundMapRelease"),
	DEFINE_OUTPUT(m_OnBackgroundMapDevelop, "OnBackgroundMapDevelop"),
END_DATADESC()

void CLogicAutoDevelop::Activate()
{
	BaseClass::Activate();
	SetNextThink(gpGlobals->curtime + 0.2f);
}

void CLogicAutoDevelop::Think(void)
{
	if (m_bRelease)
	{
		if (gpGlobals->eLoadType == MapLoad_Transition)
		{
			m_OnMapTransitionRelease.FireOutput(NULL, this);
		}
		else if (gpGlobals->eLoadType == MapLoad_NewGame)
		{
			m_OnNewGameRelease.FireOutput(NULL, this);
		}
		else if (gpGlobals->eLoadType == MapLoad_LoadGame)
		{
			m_OnLoadGameRelease.FireOutput(NULL, this);
		}
		else if (gpGlobals->eLoadType == MapLoad_Background)
		{
			m_OnBackgroundMapRelease.FireOutput(NULL, this);
		}

		m_OnMapSpawnRelease.FireOutput(NULL, this);
	}
	else
	{
		if (gpGlobals->eLoadType == MapLoad_Transition)
		{
			m_OnMapTransitionDevelop.FireOutput(NULL, this);
		}
		else if (gpGlobals->eLoadType == MapLoad_NewGame)
		{
			m_OnNewGameDevelop.FireOutput(NULL, this);
		}
		else if (gpGlobals->eLoadType == MapLoad_LoadGame)
		{
			m_OnLoadGameDevelop.FireOutput(NULL, this);
		}
		else if (gpGlobals->eLoadType == MapLoad_Background)
		{
			m_OnBackgroundMapDevelop.FireOutput(NULL, this);
		}

		m_OnMapSpawnDevelop.FireOutput(NULL, this);
	}
}