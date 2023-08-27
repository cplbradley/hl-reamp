#include "cbase.h"
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
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "particle_parse.h"
#include "weapon_rpg.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define MAX_SHOOT_RANGE 2048.0f
#define MIN_SHOOT_RANGE 90.0f

enum
{
	SCHEDULE_MECHUBUS_SHOOT = LAST_SHARED_SCHEDULE,
	SCHEDULE_MECHUBUS_KICK,
	SCHEDULE_MECHUBUS_GETCLOSER,
	SCHEDULE_MECHUBUS_GETCLOSER_FALLBACK,
};

enum
{
	TASK_MECHUBUS_WARN = LAST_SHARED_TASK,
	TASK_MECHUBUS_SHOOT,
	TASK_MECHUBUS_KICK
};

enum
{
	COND_MECHUBUS_IN_RANGE_TO_SHOOT = LAST_SHARED_CONDITION,
	COND_MECHUBUS_TOO_FAR_TO_SHOOT,
	COND_MECHUBUS_IN_RANGE_TO_KICK,
};

int AE_MECHUBUS_CANNON_SHOOT;
int AE_MECHUBUS_KICK_LAND;
int AE_MECHUBUS_FOOTSTEP;

Activity ACT_MECHUBUS_WARN;
Activity ACT_MECHUBUS_SHOOT;
Activity ACT_MECHUBUS_KICK;

ConVar sk_mechubus_health("sk_mechubus_health", "0");
ConVar sk_mechbus_projectile_speed("sk_mechubus_projectile_speed", "0");
ConVar sk_mechbus_melee_damage("sk_mechbus_melee_damage", "0");


class CNPCMechubus : public CAI_BlendingHost<CAI_BlendedNPC>
{
	DECLARE_CLASS(CNPCMechubus, CAI_BlendingHost<CAI_BlendedNPC>);

public:
	void Spawn();
	void Precache();

	virtual float MaxYawSpeed(void) { return 50.0f; }

	void BuildScheduleTestBits(void);

	bool ShouldShoot();
	bool ShouldKick();

	bool AllowedToShoot();
	bool EnemyTooFar();

	void CheckEnemyDistance();

	float GetEnemyDistance();

	virtual void HandleAnimEvent(animevent_t* pEvent);

	virtual void GatherConditions();

	virtual int SelectSchedule();
	virtual int SelectCombatSchedule();

	void	StartTask(const Task_t* pTask);
	void	RunTask(const Task_t* pTask);
	int TranslateSchedule(int scheduleType);
	virtual void RunAI(void);

	void ShootProjectile();
	void Kick();

	float fNextShoot;
	Class_T Classify() { return CLASS_ZOMBIE; }

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};
LINK_ENTITY_TO_CLASS(npc_mechubus, CNPCMechubus);
BEGIN_DATADESC(CNPCMechubus)
END_DATADESC()

void CNPCMechubus::Spawn()
{
	Precache();
	SetEnemyClass(ENEMYCLASS_HEAVY);
	SetMovementClass(MOVECLASS_HEAVY);

	SetBloodColor(BLOOD_COLOR_MECH);
	SetHullType(HULL_MECHUBUS);

	m_iHealth = sk_mechubus_health.GetInt();

	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_MOVE_JUMP);

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetNavType(NAV_GROUND);
	SetMoveType(MOVETYPE_STEP);

	NPCInit();

	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = 0.2;
}

void CNPCMechubus::Precache()
{
	PrecacheModel("models/mechubus.mdl");
	SetModel("models/mechubus.mdl");
	PrecacheScriptSound("Mechubus.FireCannon");
	PrecacheScriptSound("Mechubus.Footstep");
	PrecacheScriptSound("NPC_Vortigaunt.Swing");
	PrecacheScriptSound("Punch.Impact");
}
void CNPCMechubus::RunAI(void)
{
	BaseClass::RunAI();
}
float CNPCMechubus::GetEnemyDistance()
{
	if (!GetEnemy())
		return -1;

	return (GetAbsOrigin() - GetEnemy()->GetAbsOrigin()).Length();
}

bool CNPCMechubus::AllowedToShoot()
{
	return gpGlobals->curtime > fNextShoot;
}

bool CNPCMechubus::EnemyTooFar()
{
	return GetEnemy() && GetEnemyDistance() > MAX_SHOOT_RANGE;
}
bool CNPCMechubus::ShouldShoot()
{
	return GetEnemy() && !ShouldKick() && !EnemyTooFar() && AllowedToShoot();
}

bool CNPCMechubus::ShouldKick()
{
	return GetEnemy() && GetEnemyDistance() < MIN_SHOOT_RANGE;
}
void CNPCMechubus::CheckEnemyDistance()
{
	ShouldKick() ? SetCondition(COND_MECHUBUS_IN_RANGE_TO_KICK) : ClearCondition(COND_MECHUBUS_IN_RANGE_TO_KICK);
	EnemyTooFar() ? SetCondition(COND_MECHUBUS_TOO_FAR_TO_SHOOT) : ClearCondition(COND_MECHUBUS_TOO_FAR_TO_SHOOT);
	ShouldShoot() ? SetCondition(COND_MECHUBUS_IN_RANGE_TO_SHOOT) : ClearCondition(COND_MECHUBUS_IN_RANGE_TO_SHOOT);
}
void CNPCMechubus::GatherConditions()
{
	CheckEnemyDistance();
	BaseClass::GatherConditions();
}

int CNPCMechubus::SelectSchedule()
{
	if (GetEnemy() && m_NPCState == NPC_STATE_COMBAT)
		return SelectCombatSchedule();

	return SCHED_IDLE_WANDER;
}

int CNPCMechubus::SelectCombatSchedule()
{
	if (HasCondition(COND_ENEMY_OCCLUDED))
		return SCHED_ESTABLISH_LINE_OF_FIRE;

	if (EnemyTooFar())
		return SCHED_ESTABLISH_LINE_OF_FIRE;

	if (ShouldKick())
		return SCHEDULE_MECHUBUS_KICK;

	if (ShouldShoot())
		return SCHEDULE_MECHUBUS_SHOOT;

	return SCHED_ESTABLISH_LINE_OF_FIRE;
}

void CNPCMechubus::HandleAnimEvent(animevent_t* pEvent)
{
	if (pEvent->event == AE_MECHUBUS_CANNON_SHOOT)
	{
		ShootProjectile();
		return;
	}
	if (pEvent->event == AE_MECHUBUS_FOOTSTEP)
	{
		EmitSound("Mechubus.Footstep");
		return;
	}
	if (pEvent->event == AE_MECHUBUS_KICK_LAND)
	{
		Kick();
		return;
	}
	BaseClass::HandleAnimEvent(pEvent);
}

void CNPCMechubus::ShootProjectile()
{
	EmitSound("Mechubus.FireCannon");
	CBaseEntity* pEnemy = GetEnemy();
	float adjustspd = g_pGameRules->SkillAdjustValue(sk_mechbus_projectile_speed.GetFloat());
	if (pEnemy)
	{
		Vector vecSrc;
		GetAttachment(LookupAttachment("cannon"), vecSrc);
		//Vector vecEnemyEyePos = pEnemy->EyePosition();
		Vector vecAim = (pEnemy->WorldSpaceCenter() - vecSrc).Normalized();
		Vector vecVelocity = vecAim * adjustspd;
		CHLRMechubusMissile* pFire = (CHLRMechubusMissile*)CreateEntityByName("hlr_mechubusmissile");
		pFire->SetOwnerEntity(this);
		pFire->SetAbsOrigin(vecSrc);
		pFire->Spawn();
		pFire->SetMoveType(MOVETYPE_FLY);
		pFire->SetAbsVelocity(vecVelocity);
	}
}

void CNPCMechubus::Kick()
{
	CBaseEntity* pHurt = CheckTraceHullAttack(100, -Vector(32, 32, 32), Vector(32, 32, 32), 0, DMG_CLUB);
	CBaseCombatCharacter* pBCC = ToBaseCombatCharacter(pHurt);
	if (pBCC)
	{
		Vector forward, up;
		AngleVectors(GetLocalAngles(), &forward, NULL, &up);
		if (pBCC->IsPlayer())
		{
			pBCC->ViewPunch(QAngle(-12, -20, 0));
			pHurt->ApplyAbsVelocityImpulse(forward * 200 + up * 300);
		}
		CTakeDamageInfo info(this, this, sk_mechbus_melee_damage.GetFloat(), DMG_CLUB);
		CalculateMeleeDamageForce(&info, forward, pBCC->GetAbsOrigin());
		pBCC->TakeDamage(info);
		EmitSound("Punch.Impact");
	}
}
void CNPCMechubus::BuildScheduleTestBits(void)
{
	BaseClass::BuildScheduleTestBits();

	if (IsCurSchedule(SCHEDULE_MECHUBUS_GETCLOSER) || IsCurSchedule(SCHEDULE_MECHUBUS_GETCLOSER_FALLBACK))
	{
		SetCustomInterruptCondition(COND_MECHUBUS_IN_RANGE_TO_KICK);
		SetCustomInterruptCondition(COND_MECHUBUS_IN_RANGE_TO_SHOOT);
	}

	if (IsCurSchedule(SCHEDULE_MECHUBUS_SHOOT))
		SetCustomInterruptCondition(COND_MECHUBUS_IN_RANGE_TO_KICK);
}

int CNPCMechubus::TranslateSchedule(int scheduleType)
{
	if (scheduleType == SCHED_MOVE_TO_WEAPON_RANGE)
		return SCHEDULE_MECHUBUS_GETCLOSER;

	if (scheduleType == SCHED_CHASE_ENEMY || scheduleType == SCHED_ESTABLISH_LINE_OF_FIRE)
		return SCHEDULE_MECHUBUS_GETCLOSER;

	return BaseClass::TranslateSchedule(scheduleType);
}

void CNPCMechubus::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_MECHUBUS_WARN:
		SetActivity(ACT_MECHUBUS_WARN);
		if (AmBeingBuffed())
			SetPlaybackRate(2.0f);
		break;
	case TASK_MECHUBUS_SHOOT:
		SetActivity(ACT_MECHUBUS_SHOOT);
		fNextShoot = gpGlobals->curtime + RandomFloat(1.0f, 3.0f);
		if (AmBeingBuffed())
			SetPlaybackRate(2.0f);
		break;
	case TASK_MECHUBUS_KICK:
		SetActivity(ACT_MECHUBUS_KICK);
		EmitSound("NPC_Vortigaunt.Swing");
		break;
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CNPCMechubus::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_MECHUBUS_WARN:
	{
		if (IsActivityFinished())
			TaskComplete();
		else
			GetMotor()->SetIdealYawToTargetAndUpdate(GetEnemy()->GetAbsOrigin(), MaxYawSpeed());
		break;
	}
	case TASK_MECHUBUS_SHOOT:
	{
		
		if (IsActivityFinished())
		{
			TaskComplete();
		}
		else
			GetMotor()->SetIdealYawToTargetAndUpdate(GetEnemy()->GetAbsOrigin(), MaxYawSpeed());
		break;
	}
	case TASK_MECHUBUS_KICK:
	{
		if (IsActivityFinished())
			TaskComplete();
		break;
	}
	default:
		BaseClass::RunTask(pTask);
		break;
	}

}

AI_BEGIN_CUSTOM_NPC(npc_mechubus,CNPCMechubus)
	DECLARE_ACTIVITY(ACT_MECHUBUS_KICK)
	DECLARE_ACTIVITY(ACT_MECHUBUS_SHOOT)
	DECLARE_ACTIVITY(ACT_MECHUBUS_WARN)

	DECLARE_TASK(TASK_MECHUBUS_KICK)
	DECLARE_TASK(TASK_MECHUBUS_WARN)
	DECLARE_TASK(TASK_MECHUBUS_SHOOT)

	DECLARE_ANIMEVENT(AE_MECHUBUS_CANNON_SHOOT)
	DECLARE_ANIMEVENT(AE_MECHUBUS_FOOTSTEP)
	DECLARE_ANIMEVENT(AE_MECHUBUS_KICK_LAND)
	
	DECLARE_CONDITION(COND_MECHUBUS_TOO_FAR_TO_SHOOT)
	DECLARE_CONDITION(COND_MECHUBUS_IN_RANGE_TO_KICK)
	DECLARE_CONDITION(COND_MECHUBUS_IN_RANGE_TO_SHOOT)

	DEFINE_SCHEDULE
	(
		SCHEDULE_MECHUBUS_KICK,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_MECHUBUS_KICK				0"
		"	"
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_ENEMY_DEAD"
	)
	DEFINE_SCHEDULE
	(
		SCHEDULE_MECHUBUS_SHOOT,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_IDEAL					0"
		"		TASK_MECHUBUS_WARN				0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_MECHUBUS_SHOOT				0"
		"	"
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_ENEMY_DEAD"
	)
		DEFINE_SCHEDULE
		(
			SCHEDULE_MECHUBUS_GETCLOSER,

			"	Tasks"
			"		TASK_STOP_MOVING				0"
			"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHEDULE_MECHUBUS_GETCLOSER_FALLBACK"
			//	"		TASK_SET_TOLERANCE_DISTANCE		24"
			"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
			"		TASK_RUN_PATH					0"
			"		TASK_WAIT_FOR_MOVEMENT			0"
			"		TASK_FACE_ENEMY			0"
			"	"	
			"	Interrupts"
			"		COND_TASK_FAILED"
			"		COND_ENEMY_DEAD"
			"		COND_MECHUBUS_IN_RANGE_TO_SHOOT"
		)
		DEFINE_SCHEDULE
		(
			SCHEDULE_MECHUBUS_GETCLOSER_FALLBACK,

			"	Tasks"
			"		TASK_STOP_MOVING				0"
			"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_IDLE_WANDER"
			//	"		TASK_SET_TOLERANCE_DISTANCE		24"
			"		TASK_GET_PATH_TO_ENEMY_LKP		300"
			"		TASK_RUN_PATH					0"
			"		TASK_WAIT_FOR_MOVEMENT			0"
			"		TASK_FACE_ENEMY			0"
			"	"
			"	Interrupts"
			"		COND_TASK_FAILED"
			"		COND_ENEMY_DEAD"
			"		COND_MECHUBUS_IN_RANGE_TO_SHOOT"
		)

		
AI_END_CUSTOM_NPC()