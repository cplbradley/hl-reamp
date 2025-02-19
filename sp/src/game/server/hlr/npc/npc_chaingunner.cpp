#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_behavior_follow.h"
#include "ai_moveprobe.h"
#include "ai_senses.h"
#include "ai_speech.h"
#include "ai_task.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_baseactor.h"
#include "ai_waypoint.h"
#include "ai_link.h"
#include "ai_hint.h"
#include "ai_network.h"
#include "ai_networkmanager.h"
#include "ai_squadslot.h"
#include "ai_squad.h"
#include "ai_tacticalservices.h"
#include "ai_pathfinder.h"
#include "ammodef.h"
#include "soundent.h"
#include "ai_baseactor.h"
#include "game.h"
#include "npcevent.h"
#include "hlr/util/hlr_projectile.h"
#include "entitylist.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "hlr/weapons/actual_bullet.h"


/*#define CHAINGUNNER_MODEL ""

ConVar sk_chaingunner_health("sk_chaingunner_health", "400");
ConVar sk_chaingunner_dmg("sk_chaingunner_dmg", "8");

enum //Schedules
{
	SCHED_CHAINGUNNER_ATTACK_ENEMY = LAST_SHARED_SCHEDULE,
	SCHED_CHAINGUNNER_WAIT_FOR_ENEMY,
};

enum //Tasks
{
	TASK_CHAINGUNNER_SPINUP = LAST_SHARED_TASK,
	TASK_CHAINGUNNER_SPINDOWN,
	TASK_CHAINGUNNER_ATTACK,
};

enum
{
	COND_CHAINGUNNER_WAITING = LAST_SHARED_CONDITION,
	COND_CHAINGUNNER_WAITED_TOO_LONG_TO_SHOOT,
	COND_CHAINGUNNER_ENEMY_IS_VISIBLE,
};


Activity ACT_CHAINGUNNER_RANGE_ATTACK;
Activity ACT_CHAINGUNNER_MELEE_ATTACK;
Activity ACT_CHAINGUNNER_SPIN_UP;
Activity ACT_CHAINGUNNER_SPIN_DOWN;
Activity ACT_CHAINGUNNER_IDLE_ALERT;

class CNPC_Chaingunner : public CAI_BlendedNPC
{
	DECLARE_CLASS(CNPC_Chaingunner, CAI_BlendedNPC);
public:
	void Spawn();
	void Precache();

	Class_T Classify() { return CLASS_COMBINE; }


	int SelectCombatSchedule(void);
	int SelectSchedule(void);

	//void	HandleAnimEvent(animevent_t* pEvent);
	void	StartTask(const Task_t* pTask);
	void	RunTask(const Task_t* pTask);
	void	GatherConditions(void);

	void	FireChaingunRound(Vector vecDir, Vector vecSrc);


private:
	float m_flWaitTime;
	int tracerIndex;


protected:
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

};

LINK_ENTITY_TO_CLASS(npc_chaingunner, CNPC_Chaingunner);

BEGIN_DATADESC(CNPC_Chaingunner)
END_DATADESC()



void CNPC_Chaingunner::Spawn()
{
	Precache();
	

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	SetDefaultEyeOffset();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	SetNavType(NAV_GROUND);

	szEnemyName = "#HLR_EnemyName_Chaingunner";

	m_iMaxHealth = m_iHealth = sk_chaingunner_health.GetFloat();
	
	m_flFieldOfView = -0.4f;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_SQUAD);
	CapabilitiesAdd(bits_CAP_SKIP_NAV_GROUND_CHECK);
}

void CNPC_Chaingunner::Precache()
{
	tracerIndex = PrecacheModel("models/utils/helitracer.mdl");
}

void CNPC_Chaingunner::FireChaingunRound(Vector vecDir, Vector vecSrc)
{
	FireBulletsInfo_t info;

	info.m_iShots = 1;
	info.m_iDamageType = DMG_BULLET;
	info.m_vecDirShooting = vecDir;
	info.m_vecSrc = vecSrc;
	info.m_flDamage = sk_chaingunner_dmg.GetFloat();
	info.m_pAttacker = this;


	FireActualBullet(info, 8000, true, 0, tracerIndex, false, true, vecSrc, vecDir);
}

void CNPC_Chaingunner::GatherConditions()
{
	BaseClass::GatherConditions();
}
int CNPC_Chaingunner::SelectSchedule(void)
{
	if (GetEnemy())
		return SelectCombatSchedule();

	return SCHED_IDLE_WANDER;
}

int CNPC_Chaingunner::SelectCombatSchedule(void)
{
	if (GetEnemy())
	{
		if (HasCondition(COND_HAVE_ENEMY_LOS))
			return SCHED_CHAINGUNNER_ATTACK_ENEMY;

		else if (HasCondition(COND_CHAINGUNNER_WAITED_TOO_LONG_TO_SHOOT))
				return SCHED_ESTABLISH_LINE_OF_FIRE;
	}

	return SelectSchedule();
}

void CNPC_Chaingunner::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_CHAINGUNNER_SPINUP:
		SetIdealActivity(ACT_CHAINGUNNER_SPIN_UP);
		break;
	case TASK_CHAINGUNNER_SPINDOWN:
		SetIdealActivity(ACT_CHAINGUNNER_SPIN_DOWN);
		break;
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CNPC_Chaingunner::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_CHAINGUNNER_ATTACK:
	{
		if (HasCondition(COND_ENEMY_OCCLUDED))
		{
			if (!HasCondition(COND_CHAINGUNNER_WAITING))
			{
				m_flWaitTime = gpGlobals->curtime + 5.0f;
				SetCondition(COND_CHAINGUNNER_WAITING);
			}
			else if (HasCondition(COND_CHAINGUNNER_WAITING) && m_flWaitTime < gpGlobals->curtime)
			{
				ClearCondition(COND_CHAINGUNNER_WAITING);
				SetCondition(COND_CHAINGUNNER_WAITED_TOO_LONG_TO_SHOOT);
				TaskComplete();
			}
		}

		break;
	}

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}*/