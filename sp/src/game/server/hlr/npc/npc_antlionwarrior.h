#ifndef NPC_ANTLIONWARRIOR_H
#define NPC_ANTLIONWARRIOR_H
#pragma once

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_blended_movement.h"
#include "ai_squad.h"
#include "ai_network.h"
#include "ai_pathfinder.h"
#include "ai_navigator.h"
#include "ai_senses.h"
#include "SpriteTrail.h"
#include "Sprite.h"

inline void TraceHull_SkipPhysics(const Vector& vecAbsStart, const Vector& vecAbsEnd, const Vector& hullMin,
	const Vector& hullMax, unsigned int mask, const CBaseEntity* ignore,
	int collisionGroup, trace_t* ptr, float minMass);

#define ANTLIONWARRIOR_BLOOD_EFFECTS 2

// Spawnflags 
#define	SF_ANTLIONWARRIOR_SERVERSIDE_RAGDOLL	( 1 << 16 )
#define SF_ANTLIONWARRIOR_INSIDE_FOOTSTEPS	( 1 << 17 )

#define	ENVELOPE_CONTROLLER		(CSoundEnvelopeController::GetController())
#define	ANTLIONWARRIOR_MODEL		"models/antlion_warrior.mdl"
#define	MIN_BLAST_DAMAGE		25.0f
#define MIN_CRUSH_DAMAGE		20.0f


#define ANTLIONWARRIOR_MAX_OBJECTS				128
#define	ANTLIONWARRIOR_MIN_OBJECT_MASS			8
#define	ANTLIONWARRIOR_MAX_OBJECT_MASS			750
#define	ANTLIONWARRIOR_FARTHEST_PHYSICS_OBJECT	350
#define ANTLIONWARRIOR_OBJECTFINDING_FOV			DOT_45DEGREE // 1/sqrt(2)

//Melee definitions
#define	ANTLIONWARRIOR_MELEE1_RANGE		156.0f
#define	ANTLIONWARRIOR_MELEE1_CONE		0.7f

// Antlion summoning
#define ANTLIONWARRIOR_SUMMON_COUNT		3

// Sight
#define	ANTLIONWARRIOR_FOV_NORMAL			-0.4f

// cavern guard's poisoning behavior
#if HL2_EPISODIC
#define ANTLIONWARRIOR_POISON_TO			12 // we only poison Gordon down to twelve to give him a chance to regen up to 20 by the next charge
#endif

#define	ANTLIONWARRIOR_CHARGE_MIN			128
#define	ANTLIONWARRIOR_CHARGE_MAX			99999



int	g_interactionAntlionWarriorFoundPhysicsObject = 0;	// We're moving to a physics object to shove it, don't all choose the same object
int	g_interactionAntlionWarriorShovedPhysicsObject = 0;	// We've punted an object, it is now clear to be chosen by others

//==================================================
// AntlionWarriorSchedules
//==================================================

enum
{
	SCHED_ANTLIONWARRIOR_CHARGE = LAST_SHARED_SCHEDULE,
	SCHED_ANTLIONWARRIOR_CHARGE_CRASH,
	SCHED_ANTLIONWARRIOR_CHARGE_CANCEL,
	SCHED_ANTLIONWARRIOR_PHYSICS_ATTACK,
	SCHED_ANTLIONWARRIOR_PHYSICS_DAMAGE_HEAVY,
	SCHED_ANTLIONWARRIOR_UNBURROW,
	SCHED_ANTLIONWARRIOR_CHARGE_TARGET,
	SCHED_ANTLIONWARRIOR_FIND_CHARGE_POSITION,
	SCHED_ANTLIONWARRIOR_MELEE_ATTACK1,
	SCHED_ANTLIONWARRIOR_SUMMON,
	SCHED_ANTLIONWARRIOR_PATROL_RUN,
	SCHED_ANTLIONWARRIOR_ROAR,
	SCHED_ANTLIONWARRIOR_CHASE_ENEMY_TOLERANCE,
	SCHED_FORCE_ANTLIONWARRIOR_PHYSICS_ATTACK,
	SCHED_ANTLIONWARRIOR_CANT_ATTACK,
	SCHED_ANTLIONWARRIOR_TAKE_COVER_FROM_ENEMY,
	SCHED_ANTLIONWARRIOR_RANGE_ATTACK1,
	SCHED_ANTLIONWARRIOR_RANGE_ATTACK2,
	SCHED_ANTLIONWARRIOR_CHASE_ENEMY
};


//==================================================
// AntlionWarriorTasks
//==================================================

enum
{
	TASK_ANTLIONWARRIOR_CHARGE = LAST_SHARED_TASK,
	TASK_ANTLIONWARRIOR_GET_PATH_TO_PHYSOBJECT,
	TASK_ANTLIONWARRIOR_SHOVE_PHYSOBJECT,
	TASK_ANTLIONWARRIOR_SUMMON,
	//TASK_DISABLE_LAUNCHER,
	//TASK_ENABLE_LAUNCHER,
	TASK_ANTLIONWARRIOR_SET_FLINCH_ACTIVITY,
	TASK_ANTLIONWARRIOR_GET_PATH_TO_CHARGE_POSITION,
	TASK_ANTLIONWARRIOR_GET_PATH_TO_NEAREST_NODE,
	TASK_ANTLIONWARRIOR_GET_CHASE_PATH_ENEMY_TOLERANCE,
	TASK_ANTLIONWARRIOR_OPPORTUNITY_THROW,
	TASK_ANTLIONWARRIOR_FIND_PHYSOBJECT
};

//==================================================
// AntlionWarriorConditions
//==================================================

enum
{
	COND_ANTLIONWARRIOR_PHYSICS_TARGET = LAST_SHARED_CONDITION,
	COND_ANTLIONWARRIOR_PHYSICS_TARGET_INVALID,
	COND_ANTLIONWARRIOR_HAS_CHARGE_TARGET,
	COND_ANTLIONWARRIOR_CAN_SUMMON,
	COND_ANTLIONWARRIOR_CAN_CHARGE,
	COND_ANTLIONWARRIOR_CAN_SPIT,
	COND_ANTLIONWARRIOR_CAN_BOMBARD
};

enum
{
	SQUAD_SLOT_ANTLIONWARRIOR_CHARGE = LAST_SHARED_SQUADSLOT,
};

//==================================================
// AntlionWarrior Activities
//==================================================

struct PhysicsObjectCriteria_t
{
	CBaseEntity* pTarget;
	Vector	vecCenter;		// Center point to look around
	float	flRadius;		// Radius to search within
	float	flTargetCone;
	bool	bPreferObjectsAlongTargetVector;	// Prefer objects that we can strike easily as we move towards our target
	float	flNearRadius;						// If we won't hit the player with the object, but get this close, throw anyway
};

#define MAX_FAILED_PHYSOBJECTS 8

class CNPC_AntlionWarrior : public CAI_BlendedNPC
{
public:
	DECLARE_CLASS(CNPC_AntlionWarrior, CAI_BlendedNPC);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNPC_AntlionWarrior(void);

	Class_T	Classify(void) { return CLASS_ANTLION; }
	virtual int		GetSoundInterests(void) { return (SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_DANGER); }
	virtual bool	QueryHearSound(CSound* pSound);

	const impactdamagetable_t& GetPhysicsImpactDamageTable(void);

	virtual int		MeleeAttack1Conditions(float flDot, float flDist);
	virtual int		SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);

	virtual int		TranslateSchedule(int scheduleType);
	virtual int		OnTakeDamage_Alive(const CTakeDamageInfo& info);
	virtual void	DeathSound(const CTakeDamageInfo& info);
	virtual void	Event_Killed(const CTakeDamageInfo& info);
	virtual int		SelectSchedule(void);

	virtual float GetAutoAimRadius() { return 36.0f; }

	virtual void	Precache(void);
	virtual void	Spawn(void);
	virtual void	Activate(void);
	virtual void	PostNPCInit(void);
	virtual void	HandleAnimEvent(animevent_t* pEvent);
	virtual void	UpdateEfficiency(bool bInPVS) { SetEfficiency((GetSleepState() != AISS_AWAKE) ? AIE_DORMANT : AIE_NORMAL); SetMoveEfficiency(AIME_NORMAL); }
	virtual void	PrescheduleThink(void);
	virtual void	GatherConditions(void);
	virtual void	TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr, CDmgAccumulator* pAccumulator);
	virtual void	StartTask(const Task_t* pTask);
	virtual void	RunTask(const Task_t* pTask);
	virtual void	StopLoopingSounds();
	virtual bool	HandleInteraction(int interactionType, void* data, CBaseCombatCharacter* sender);

	virtual int		RangeAttack1Conditions(float flDot, float flDist);
	virtual int		RangeAttack2Conditions(float flDot, float flDist);

	Vector	m_vecSaveSpitVelocity;

	// Input handlers.
	void	InputSetShoveTarget(inputdata_t& inputdata);
	void	InputSetChargeTarget(inputdata_t& inputdata);
	void	InputClearChargeTarget(inputdata_t& inputdata);
	void	InputUnburrow(inputdata_t& inputdata);
	void	InputRagdoll(inputdata_t& inputdata);
	void	InputEnableBark(inputdata_t& inputdata);
	void	InputDisableBark(inputdata_t& inputdata);
	void	InputSummonedAntlionDied(inputdata_t& inputdata);
	void	InputEnablePreferPhysicsAttack(inputdata_t& inputdata);
	void	InputDisablePreferPhysicsAttack(inputdata_t& inputdata);

	virtual bool	IsLightDamage(const CTakeDamageInfo& info);
	virtual bool	IsHeavyDamage(const CTakeDamageInfo& info);
	virtual bool	OverrideMoveFacing(const AILocalMoveGoal_t& move, float flInterval);
	virtual bool	BecomeRagdollOnClient(const Vector& force);
	virtual void UpdateOnRemove(void);
	virtual bool		IsUnreachable(CBaseEntity* pEntity);			// Is entity is unreachable?

	bool IsCharging();

	virtual bool GetSpitVector(const Vector& vecStartPos, const Vector& vecTarget, Vector* vecOut);
	virtual bool GetMissileVector(const Vector& vecStartPos, const Vector& vecTarget, Vector* vecOut);

	virtual float	MaxYawSpeed(void);
	virtual bool	OverrideMove(float flInterval);
	virtual bool	CanBecomeRagdoll(void);

	virtual bool	ShouldProbeCollideAgainstEntity(CBaseEntity* pEntity);

	virtual Activity	NPC_TranslateActivity(Activity baseAct);

#if HL2_EPISODIC
	//---------------------------------
	// Navigation & Movement -- prevent stopping paths for the guard
	//---------------------------------
	class CNavigator : public CAI_ComponentWithOuter<CNPC_AntlionWarrior, CAI_Navigator>
	{
		typedef CAI_ComponentWithOuter<CNPC_AntlionWarrior, CAI_Navigator> BaseClass;
	public:
		CNavigator(CNPC_AntlionWarrior* pOuter)
			: BaseClass(pOuter)
		{
		}

		bool GetStoppingPath(CAI_WaypointList* pClippedWaypoints);
	};
	CAI_Navigator* CreateNavigator() { return new CNavigator(this); }
#endif

	DEFINE_CUSTOM_AI;

private:

	inline bool CanStandAtPoint(const Vector& vecPos, Vector* pOut);
	bool	RememberFailedPhysicsTarget(CBaseEntity* pTarget);
	void	GetPhysicsShoveDir(CBaseEntity* pObject, float flMass, Vector* pOut);
	void	CreateGlow(CSprite** pSprite, const char* pAttachName);
	void	DestroyGlows(void);
	void	Footstep(bool bHeavy);
	int		SelectCombatSchedule(void);
	int		SelectUnreachableSchedule(void);
	bool	CanSummon(bool bIgnoreTime);
	void	SummonAntlions(void);

	void	ChargeLookAhead(void);
	bool	EnemyIsRightInFrontOfMe(CBaseEntity** pEntity);
	bool	HandleChargeImpact(Vector vecImpact, CBaseEntity* pEntity);
	bool	ShouldCharge(const Vector& startPos, const Vector& endPos, bool useTime, bool bCheckForCancel);
	bool	ShouldWatchEnemy(void);

	float	m_fNextBombard;
	float	m_fNextFireball;

	void	ImpactShock(const Vector& origin, float radius, float magnitude, CBaseEntity* pIgnored = NULL);
	void	BuildScheduleTestBits(void);
	void	Shove(void);
	void	FoundEnemy(void);
	void	LostEnemy(void);
	void	UpdateHead(void);
	void	UpdatePhysicsTarget(bool bPreferObjectsAlongTargetVector, float flRadius = ANTLIONWARRIOR_FARTHEST_PHYSICS_OBJECT);
	void	MaintainPhysicsTarget(void);
	void	ChargeDamage(CBaseEntity* pTarget);
	void	StartSounds(void);
	void	SetHeavyDamageAnim(const Vector& vecSource);
	float	ChargeSteer(void);
	CBaseEntity* FindPhysicsObjectTarget(const PhysicsObjectCriteria_t& criteria);
	Vector	GetPhysicsHitPosition(CBaseEntity* pObject, CBaseEntity* pTarget, Vector* vecTrajectory, float* flClearDistance);
	bool	CanStandAtShoveTarget(CBaseEntity* pShoveObject, CBaseEntity* pTarget, Vector* pOut);
	CBaseEntity* GetNextShoveTarget(CBaseEntity* pLastEntity, AISightIter_t& iter);

	int				m_nFlinchActivity;

	bool			m_bStopped;
	bool			m_bIsBurrowed;
	bool			m_bBarkEnabled;
	float			m_flNextSummonTime;
	int				m_iNumLiveAntlions;

	float			m_flSearchNoiseTime;
	float			m_flAngerNoiseTime;
	float			m_flBreathTime;
	float			m_flChargeTime;
	float			m_flPhysicsCheckTime;
	float			m_flNextHeavyFlinchTime;
	float			m_flNextRoarTime;
	int				m_iChargeMisses;
	bool			m_bDecidedNotToStop;
	bool			m_bPreferPhysicsAttack;
	bool			m_bIsAlive;
	float			gHealth;
	float			gMaxHealth;

	bool bCharging;
	CNetworkVar(bool, m_bCavernBreed);	// If this guard is meant to be a cavern dweller (uses different assets)
	CNetworkVar(bool, m_bInCavern);		// Behavioral hint telling the guard to change his behavior

	Vector			m_vecPhysicsTargetStartPos;
	Vector			m_vecPhysicsHitPosition;

	EHANDLE			m_hShoveTarget;
	EHANDLE			m_hChargeTarget;
	EHANDLE			m_hChargeTargetPosition;
	EHANDLE			m_hOldTarget;
	EHANDLE			m_hPhysicsTarget;

	CUtlVectorFixed<EHANDLE, MAX_FAILED_PHYSOBJECTS>		m_FailedPhysicsTargets;


	//CHandle< CParticleSystem >	m_hSpitEffect;

	//bool	DispatchFlame(void);
	//bool m_bIsFlaming;

	COutputEvent	m_OnSummon;
	COutputEvent	m_OnBarageEnd;
	COutputEvent	m_OnBarageStart;

	CSoundPatch* m_pGrowlHighSound;
	CSoundPatch* m_pGrowlLowSound;
	CSoundPatch* m_pGrowlIdleSound;
	CSoundPatch* m_pBreathSound;
	CSoundPatch* m_pConfusedSound;

	string_t		m_iszPhysicsPropClass;
	string_t		m_strShoveTargets;

	CSprite* m_hCaveGlow[2];

#if ANTLIONWARRIOR_BLOOD_EFFECTS
	CNetworkVar(uint8, m_iBleedingLevel);

	unsigned char GetBleedingLevel(void) const;
#endif
protected:

	int m_poseThrow;
	int m_poseHead_Yaw, m_poseHead_Pitch;
	virtual void	PopulatePoseParameters(void);


	// inline accessors
public:
	inline bool IsCavernBreed(void) const { return m_bCavernBreed; }
	inline bool IsInCavern(void) const { return m_bInCavern; }
};

#endif