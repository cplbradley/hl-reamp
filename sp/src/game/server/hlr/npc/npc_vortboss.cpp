//========= VORTIGAUNT BOSS CREATED BY JUST WAX @ VELOCITY PUNCH ============//
//=============================================================================//



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
#include "ai_squadslot.h"
#include "ai_squad.h"
#include "ai_tacticalservices.h"
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

//=========================================================
// Custom schedules
//=========================================================

enum SquadSlot_t
{
	SQUAD_SLOT_VORTBOSS_SPINBEAM = LAST_SHARED_SQUADSLOT,
};
enum //Body states
{
	BODYSTATE_NORMAL,
	BODYSTATE_DAMAGED,
	BODYSTATE_BADLYDAMAGED
};
enum //Schedules
{
	SCHED_VORTBOSS_CANNON = LAST_SHARED_SCHEDULE,
	SCHED_VORTBOSS_EYEBLAST,
	SCHED_VORTBOSS_CHASEENEMY,
	SCHED_VORTBOSS_GROUND_ATTACK,
	SCHED_VORTBOSS_SPINATTACK,
	SCHED_VORTBOSS_BACKOFF,
	SCHED_VORTBOSS_CHARGE,
	SCHED_VORTBOSS_DODGE_ROCKET,
	SCHED_VORTBOSS_BARAGE,
	SCHED_VORTBOSS_SWING,
	SCHED_VORTBOSS_ESTABLISH_LOF
};

enum //Tasks
{
	TASK_VORTBOSS_CANNON = LAST_SHARED_TASK,
	TASK_VORTBOSS_EYEBLAST,
	TASK_VORTBOSS_GROUNDATTACK,
	TASK_VORTBOSS_SPINATTACK,
	TASK_VORTBOSS_CHARGE,
	TASK_VORTBOSS_DODGEROCKET,
	TASK_VORTBOSS_SWING,
	TASK_VORTBOSS_BARAGE
};

enum //Conditions
{
	COND_CAN_FIRE_CANNON = LAST_SHARED_CONDITION,
	COND_CAN_DO_EYEBLAST,
	COND_CAN_DO_GROUND_ATTACK,
	COND_CAN_SPIN_ATTACK,
	COND_SHOULD_CHARGE,
	COND_SHOULD_DODGE_ROCKET,
	COND_SHOULD_BARAGE,
	COND_SHOULD_SWING
};

ConVar sk_vortboss_health("sk_vortboss_health", "2500");
ConVar sk_vortboss_projectile_speed("sk_vortboss_projectile_speed", "1800");
ConVar g_debug_vortboss_spinattack("g_debug_vortboss_spinattack", "0");
ConVar sk_vortboss_dmg_projectile("sk_vortboss_dmg_projectile", "9");
ConVar sk_vortboss_dmg_charge("sk_vortboss_dmg_charge", "40");
ConVar sk_vortboss_rocket_speed("sk_vortboss_rocket_speed", "1200");
ConVar sk_vortboss_barage_frequency("sk_vortboss_barage_frequency", "20");
ConVar sk_vortboss_eyeblast_frequency("sk_vortboss_eyeblast_frequency", "15");
ConVar sk_vortboss_cannon_frequency("sk_vortboss_cannon_frequency", "5");
ConVar sk_vortboss_swing_dmg("sk_vortboss_swing_dmg", "30");

ConVar g_debug_vortboss_rocket("g_debug_vortboss_rocket", "0");

Activity ACT_VORTBOSS_EYEBLAST;
Activity ACT_VORTBOSS_CANNON;
Activity ACT_VORTBOSS_GROUND_ATTACK;
Activity ACT_VORTBOSS_SPINATTACK;
Activity ACT_VORTBOSS_CHARGE_START;
Activity ACT_VORTBOSS_CHARGE_LOOP;
Activity ACT_VORTBOSS_CHARGE_STOP;
Activity ACT_VORTBOSS_CRASH;
Activity ACT_VORTBOSS_DODGE_LEFT;
Activity ACT_VORTBOSS_DODGE_RIGHT;
Activity ACT_VORTBOSS_BARAGE;
Activity ACT_VORTBOSS_SWING;


int AE_VORTBOSS_CHARGECANNON;
int AE_VORTBOSS_FIRECANNON;
int AE_VORTBOSS_START_TARGETBEAM;
int AE_VORTBOSS_FIRE_ATTACKBEAM;
int AE_VORTBOSS_GROUNDATTACK;
int AE_VORTBOSS_CLEAR_BEAM;
int AE_VORTBOSS_START_SPINATTACK;
int AE_VORTBOSS_END_SPINATTACK;
int AE_VORTBOSS_ROCKET_LAUNCH;
int AE_VORTBOSS_SWING_IMPACT;


#define VORTBOSS_EYE_ATTACHMENT 1
#define VORTBOSS_HAND_ATTACHMENT 2
#define VORTBOSS_MAX_LASER_RANGE 3600
#define VORTBOSS_MODEL "models/vortboss_test_weights.mdl"
class CRocketTarget : public CBaseAnimating
{
	DECLARE_CLASS(CRocketTarget, CBaseAnimating);
public:
	void Precache(void);
	void Spawn(void);
	void Kill(void);
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(rocket_target, CRocketTarget);
BEGIN_DATADESC(CRocketTarget)
END_DATADESC()

void CRocketTarget::Precache(void)
{
	PrecacheModel("models/utilities/floorcrosshair.mdl");
}
void CRocketTarget::Spawn(void)
{
	Precache();
	SetModel("models/utilities/floorcrosshair.mdl");
	SetModelScale(1.5f);
	AddEffects(EF_NOSHADOW);
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_CUSTOM, MOVECOLLIDE_DEFAULT);
	//SetLocalAngularVelocity(QAngle(0, 0, 0));
	ApplyLocalAngularVelocityImpulse(AngularImpulse(0,90,0));
	SetThink(&CRocketTarget::Kill);
	SetNextThink(gpGlobals->curtime + 2.0f);
}
void CRocketTarget::Kill(void)
{
	RemoveDeferred();
}
//=========================================================
//=========================================================
class CNPC_VortBoss : public CAI_BlendedNPC
{
	DECLARE_CLASS(CNPC_VortBoss, CAI_BlendedNPC);
	DECLARE_SERVERCLASS();

public:
	void	Precache(void);
	void	Spawn(void);
	Class_T Classify() { return CLASS_VORTIGAUNT; }

	void	StartSpinBeam(void);
	void	ClearSpinBeam(void);
	void	UpdateSpinBeam(void);
	void	CheckSpinBeam(void);

	void FirePrecalculatedRocket(void);

	//CBeam *pSpinBeam;

	/*
	int RangeAttack1Conditions(float flDot, float flDist);
	int RangeAttack2Conditions(float flDot, float flDist);

	void	FireCannonProjectile(void);

	void	GroundAttack(void);*/

	CHandle<CBeam> pEyeTargetBeam;
	CHandle<CBeam> pEyeAttackBeam;


	void	CreateTargetBeam(void);
	void	EyeBeamThink(void);
	void	CreateAttackBeam(void);
	void	AttackBeamThink(void);
	void	ClearBeam(void);

	virtual float	MaxYawSpeed(void);

	void	NPCThink(void);
	void	RelaxAim(float flInterval);
	void	UpdateAim();
	void	SetAim(const Vector &aimDir, float flInterval);
	void	GatherConditions(void);
	int		SelectSchedule(void);
	int		SelectCombatSchedule(void);
	void	HandleAnimEvent(animevent_t *pEvent);
	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);

	bool	EnemyIsRightInFrontOfMe(CBaseEntity **pEntity);
	bool	ShouldSwing(CBaseEntity *pTarget);
	void  ChargeDamage(CBaseEntity *pTarget);
	

	void UpdateBodyState(void);

	virtual int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	CNetworkVar(bool,bDrawSpinBeam);
	CNetworkVar(bool, bBleeding);
	virtual void Event_Killed(const CTakeDamageInfo &info);

	Vector m_vTargetPos;
	Vector m_vStartPos;
	float m_fNextEyeBlast;
	float m_fNextCannonAttack;
	float m_fNextSpinAttack;
	float m_fNextDodge;
	float m_fNextBarage;
	float m_fNextSwing;
	float	ChargeSteer(void);

	Vector GetRocketTrajectory(void);
	Vector GetPredictedRocketPosition(void);

	bool	CanBarage(void);

	bool	ShouldCharge(const Vector &startPos, const Vector &endPos, bool useTime, bool bCheckForCancel);

	bool	CheckForRockets(void);
	bool	HandleChargeImpact(Vector vecImpact, CBaseEntity *pEntity);
	
	static int gm_nBodyPitchPoseParam;

	int m_iBodyState;

	float m_fNextTrace;

	int iAttackBeamWidth;

	void SpinAttackThink(void);

	virtual Activity	NPC_TranslateActivity(Activity baseAct);
	virtual int		TranslateSchedule(int scheduletype);
	virtual bool	OverrideMove(float flInterval);

protected:
	void SetupGlobalModelData();
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(npc_vortboss, CNPC_VortBoss);

IMPLEMENT_SERVERCLASS_ST(CNPC_VortBoss,DT_VortBoss)
	SendPropBool(SENDINFO(bDrawSpinBeam)),
	SendPropBool(SENDINFO(bBleeding)),
END_SEND_TABLE()


int CNPC_VortBoss::gm_nBodyPitchPoseParam = -1;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_VortBoss)
DEFINE_FIELD(m_fNextEyeBlast,FIELD_FLOAT),
DEFINE_FIELD(m_fNextCannonAttack,FIELD_FLOAT),
DEFINE_FIELD(m_fNextBarage,FIELD_FLOAT),
DEFINE_FIELD(m_fNextSpinAttack,FIELD_FLOAT),
DEFINE_FIELD(m_fNextDodge,FIELD_FLOAT),

DEFINE_FIELD(pEyeTargetBeam,FIELD_EHANDLE),
DEFINE_FIELD(pEyeAttackBeam,FIELD_EHANDLE),

DEFINE_FIELD(bDrawSpinBeam,FIELD_BOOLEAN),
DEFINE_FIELD(m_iBodyState,FIELD_INTEGER),
DEFINE_FIELD(bBleeding,FIELD_BOOLEAN),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Initialize the custom schedules
// Input  :
// Output :
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_VortBoss::Precache(void)
{
	PrecacheModel(VORTBOSS_MODEL);
	UTIL_PrecacheOther("hlr_vortprojectile");
	PrecacheMaterial("sprites/laser.vmt");
	PrecacheParticleSystem("vortboss_cannon_core");
	PrecacheParticleSystem("vortboss_ground_attack");
	PrecacheParticleSystem("blood_vortboss_bleeding");
	PrecacheParticleSystem("hlr_base_explosion1");
	PrecacheScriptSound("BadDog.Smash");
	PrecacheScriptSound("VortBoss.EyeBlast");
	UTIL_PrecacheOther("rpg_missile");
	UTIL_PrecacheOther("rocket_target");

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_VortBoss::Spawn(void)
{
	Precache();

	SetModel(VORTBOSS_MODEL);
	SetHullType(HULL_VORTBOSS);

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_RED);
	SetNavType(NAV_GROUND);
	SetEnemyClass(ENEMYCLASS_SUPERHEAVY);
	bDrawSpinBeam = false;

	RegisterThinkContext("EyeUpdateContext");
	RegisterThinkContext("EyeAttackContext");

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	CapabilitiesAdd(bits_CAP_MOVE_JUMP);
	CapabilitiesAdd(bits_CAP_SKIP_NAV_GROUND_CHECK);
	CapabilitiesAdd(bits_CAP_MOVE_SHOOT);
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);
	//CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK2 | bits_CAP_INNATE_MELEE_ATTACK1);
	SetHealth(sk_vortboss_health.GetFloat());
	SetMaxHealth(sk_vortboss_health.GetFloat());
	m_flFieldOfView = -0.9;
	m_NPCState = NPC_STATE_NONE;
	m_iBodyState = BODYSTATE_NORMAL;
	UpdateBodyState();
	SetupGlobalModelData();
	//CapabilitiesClear();
	//CapabilitiesAdd( bits_CAP_NONE );
	m_fNextTrace = gpGlobals->curtime;
	NPCInit();
	m_fNextCannonAttack = gpGlobals->curtime;
	m_fNextEyeBlast = gpGlobals->curtime;
	m_fNextSwing = gpGlobals->curtime;
	m_fNextSpinAttack = gpGlobals->curtime;
}

void CNPC_VortBoss::NPCThink(void) //constant thinking 
{
	BaseClass::NPCThink();
	//UpdateAim();
	CheckSpinBeam();
	CheckForRockets();
	
}
void CNPC_VortBoss::SetupGlobalModelData()
{
	gm_nBodyPitchPoseParam = LookupPoseParameter("aim_pitch");
}
void CNPC_VortBoss::UpdateAim()
{
	if (!GetModelPtr() || !GetModelPtr()->SequencesAvailable())
		return;

	float flInterval = GetAnimTimeInterval();

	// Some activities look bad if we're giving our enemy the stinkeye.
	int eActivity = GetActivity();

	if (GetEnemy() &&
		GetState() != NPC_STATE_SCRIPT &&
		(eActivity != ACT_VORTBOSS_SPINATTACK || ACT_VORTBOSS_CHARGE_LOOP || ACT_VORTBOSS_CHARGE_STOP || ACT_VORTBOSS_CHARGE_START))
	{
		Vector vecShootOrigin;

		vecShootOrigin = WorldSpaceCenter();
		Vector vecShootDir = GetShootEnemyDir(vecShootOrigin, false);

		SetAim(vecShootDir, flInterval);
	}
	/*else
	{

	}
	{
		RelaxAim(flInterval);
	}*/
}

bool CNPC_VortBoss::CanBarage(void)
{
	trace_t tr;
	UTIL_TraceLine(WorldSpaceCenter(), WorldSpaceCenter() + (Vector(0, 0, 1) * 256), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr); //shoot a trace straight up to see if there's anything directly above us a rocket could hit
	if (g_debug_vortboss_rocket.GetBool())
		DebugDrawLine(tr.startpos, tr.endpos, 255, 0, 0, false, 3.0f); //draw the trace 

	if (tr.fraction != 1.0) //if there's something above us the rockets would hit, preventing them from reaching our target
		return false; //don't fire the barage
	if (GetRocketTrajectory() == vec3_origin) //if the rocket trajectory test fails
		return false; //don't fire the barage
	return true; //fire the barage
}
Vector CNPC_VortBoss::GetPredictedRocketPosition(void)
{
	Vector vecPredPos;
	Vector enemyDelta = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter(); //get the vector from me to my target
	float flDist = enemyDelta.Length(); //get the length of the vector in float form
	float basespd = sk_vortboss_rocket_speed.GetFloat(); //get our base rocket speed
	float adjustedspd = g_pGameRules->SkillAdjustValue(basespd); //scale it based on difficulty 
	float timeDelta = flDist / adjustedspd; //get the amount of time it would take to reach our target
	UTIL_PredictedPosition(GetEnemy(), timeDelta, &vecPredPos); //use the above to estimate where our target will be after the given amount of time

	return vecPredPos; //return that position
}
Vector CNPC_VortBoss::GetRocketTrajectory(void)
{
	int nAttachment = LookupAttachment("cannonmuzzle"); //lookup the cannon muzzle attachment
	Vector vecCannonPos;
	GetAttachment(nAttachment, vecCannonPos); //get the world position of the above attachment
	Vector vecTarget = GetPredictedRocketPosition(); //get our predicted player position estimate
	
	float basespd = sk_vortboss_rocket_speed.GetFloat(); //get the base rocket speed
	float adjustedspd = g_pGameRules->SkillAdjustValue(basespd); //get the scaled rocket speed

	Vector vecShootDir = VecCheckThrow(this, vecCannonPos, vecTarget, adjustedspd, 1.5f); //get our launch trajectory (the calculated arc to fire the rocket at to reach the designated target)

	if (vecShootDir == vec3_origin)
	{
		DevWarning("TRAJECTORY CALCULATION FAILED, ATTEMPING ADJUSTMENTS\n");
		vecShootDir = VecCheckThrow(this, vecCannonPos, GetEnemy()->WorldSpaceCenter(), adjustedspd, 1.5f);
	}

	return vecShootDir; //return that vector
}
void CNPC_VortBoss::FirePrecalculatedRocket(void)
{
	/*CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;
	int nAttachment = LookupAttachment("cannonmuzzle");
	Vector vecCannonPos;
	GetAttachment(nAttachment, vecCannonPos);
	
	Vector vecPredPos;
	Vector enemyDelta = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
	float flDist = enemyDelta.Length();
	float basespd = sk_vortboss_rocket_speed.GetFloat();
	float timeDelta = flDist / adjustedspd;
	UTIL_PredictedPosition(GetEnemy(), timeDelta, &vecPredPos);
	
	
	Vector vecShootDir = VecCheckThrow(this, vecCannonPos, vecPredPos, adjustedspd, 1.0f);

	if (vecShootDir == vec3_origin)
		return;*/
	int nAttachment = LookupAttachment("cannonmuzzle"); //get our cannon attachment
	Vector vecCannonPos;
	GetAttachment(nAttachment, vecCannonPos); //get the world position of the attachment

	Vector vecShootDir = GetRocketTrajectory(); //get the calculated rocket trajectory

	if (vecShootDir == vec3_origin)
	{
		DevWarning("***ADJUSTMENTS FAILED, CANCELLING ROCKET***\n");
		return;
	}

	CMissile *pMissile = (CMissile*)CreateEntityByName("rpg_missile"); //create a rocket
	UTIL_SetOrigin(pMissile, vecCannonPos); //set it's position to the cannon attachment
	pMissile->SetOwnerEntity(this); //set us as the owner
	pMissile->SetGravity(1.5f); //set the gravity to 1.5
	pMissile->Spawn(); //initialize the spawned rocket
	pMissile->SetMoveType(MOVETYPE_FLYGRAVITY); //make it affected by gravity
	pMissile->SetAbsVelocity(vecShootDir); //fire it at the calculated trajectory
	pMissile->CreateSmokeTrail(); //initialize the smoke effects
	
	QAngle angdir; 
	VectorAngles(vecShootDir, angdir); //turn our calculated trajectory into an angle
	pMissile->SetLocalAngles(angdir);//set it to said angle

	if (!pMissile->ActiveSteerMode())
		pMissile->ActiveSteerMode(); //turn on the auto-updating angles
	
	CRocketTarget *pTarget = (CRocketTarget*)CreateEntityByName("rocket_target"); //create a floor target
	UTIL_SetOrigin(pTarget, GetPredictedRocketPosition()); //set it to the predicted enemy position estimate
	pTarget->Spawn(); //initalize it
	pTarget->SetLocalAngularVelocity(QAngle(0, 90, 0));
}

bool CNPC_VortBoss::CheckForRockets(void)
{
	CBaseEntity *pEntity = NULL; //create a null pointer
	pEntity = gEntList.FindEntityByClassname(pEntity, "rpg_missile"); //set the null pointer to test for any rockets

	if (!pEntity) //if it fails to find a rocket, stop.
		return false;
	if (m_fNextDodge > gpGlobals->curtime) //if we already dodged too recently, stop.
		return false;
	if (pEntity != this) //if the object we found is NOT me
	{
		string_t strname = pEntity->m_iClassname; //get the object's classname
		DevMsg("entity classname within radius: %s\n", strname); //tell the dev we found something, print what it is that we found
		if (pEntity->ClassMatches("rpg_missile")) //if what we found is a rocket
		{
			DevMsg("ROCKET DETECTED\n"); //tell the dev we found a rocket
			if (pEntity->GetOwnerEntity() && pEntity->GetOwnerEntity() == this) //if it's my rocket
				return false; //stop
			float dist = (pEntity->GetAbsOrigin() - this->GetAbsOrigin()).Length(); //calculated the distance from me to the rocket
			if (dist < 512.0f) //if it's within our dodgeable range
				return true; //dodge
		}
	}
	return false; //stop
}

void CNPC_VortBoss::CheckSpinBeam(void)
{
	/*if (bDrawSpinBeam) //if we're drawing the spin beam
	{
		//SetEfficiency(AIE_HYPER); UNDONE: useless
		Vector vecHandPos, m_vLaserDir;
		QAngle angHandAng;
		GetAttachment(VORTBOSS_HAND_ATTACHMENT, vecHandPos, angHandAng); //get hand position and angles, save them to a vector and an angle
		//AngleVectors(angHandAng, &m_vLaserDir); 
		trace_t tr;
		//m_vLaserDir[ROLL] = 0;
		for (int i = 0; i < 3; i++) //do the following 3 times
		{

			if (i == 0) //for our first trace
			{

				AngleVectors(angHandAng, &m_vLaserDir); //convert the angles into a vector
				m_vLaserDir[ROLL] = 0; //set the Z value to zero to make it straight
				AI_TraceLine(vecHandPos, vecHandPos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr); //fire our trace from the hand postion vector in the direction of the angle
				if (g_debug_vortboss_spinattack.GetBool()) //if we're debugging
					DebugDrawLine(tr.startpos, tr.endpos, 255, 0, 0, false, 0.05); //draw a debug line
				CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(tr.m_pEnt); //save what our trace hit

				if (pBCC) //if it's a character
				{
					ClearMultiDamage(); //clear the damage schedule 
					CTakeDamageInfo info(this, this, 20, DMG_SHOCK); //register the damage info
					info.AdjustPlayerDamageTakenForSkillLevel(); //adjust it for skill level
					info.SetDamagePosition(tr.endpos); //set the position to where the trace hit
					CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos); //calculate the damage 
					pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr); //display blood
					ApplyMultiDamage(); //apply the damage
					DevMsg("server hit\n"); //if we're in dev mode, send a message to console that we hit
				}
			}
			if (i == 1) //for our second trace
			{
				angHandAng[1] += 1; //adjust the angle out by 1 degree
				AngleVectors(angHandAng, &m_vLaserDir); //repeat the same process
				m_vLaserDir[ROLL] = 0;
				AI_TraceLine(vecHandPos, vecHandPos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
				if (g_debug_vortboss_spinattack.GetBool())
					DebugDrawLine(tr.startpos, tr.endpos, 255, 0, 0, false, 0.05);
				CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(tr.m_pEnt);

				if (pBCC)
				{
					ClearMultiDamage();
					CTakeDamageInfo info(this, this, 20, DMG_SHOCK);
					info.AdjustPlayerDamageTakenForSkillLevel();
					info.SetDamagePosition(tr.endpos);
					CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
					pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
					ApplyMultiDamage();
					DevMsg("server hit\n");
				}
			}
			if (i == 2) //for our third trace
			{
				angHandAng[1] -= 2; //adjust the angle the other way 2 degrees to make up for the 1 degree turn we made in the opposite direction
				AngleVectors(angHandAng, &m_vLaserDir);//repeat the process
				m_vLaserDir[ROLL] = 0;
				AI_TraceLine(vecHandPos, vecHandPos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
				if (g_debug_vortboss_spinattack.GetBool())
					DebugDrawLine(tr.startpos, tr.endpos, 255, 0, 0, false, 0.05);
				CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(tr.m_pEnt);

				if (pBCC)
				{
					ClearMultiDamage();
					CTakeDamageInfo info(this, this, 20, DMG_SHOCK);
					info.AdjustPlayerDamageTakenForSkillLevel();
					info.SetDamagePosition(tr.endpos);
					CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
					pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
					ApplyMultiDamage();
					DevMsg("server hit\n");
				}
			}
		}*/
	//}
}

float CNPC_VortBoss::MaxYawSpeed(void)
{
	Activity eActivity = GetActivity();

	CBaseEntity *pEnemy = GetEnemy();

	if (pEnemy != NULL && pEnemy->IsPlayer() == false)
		return 40.0f;

	if (eActivity == ACT_VORTBOSS_CHARGE_LOOP)
	{
		float dist = (GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter()).Length();

		if (dist > 512)
			return 16.0f;

		//FIXME: Alter by skill level
		/*float yawSpeed = RemapVal(dist, 0, 512, 1.0f, 2.0f);
		yawSpeed = clamp(yawSpeed, 1.0f, 2.0f);*/

		return 0.0f;
	}
	if (eActivity == ACT_VORTBOSS_CHARGE_STOP)
		return 2.0f;
	switch (eActivity)
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 90.0f;
		break;

	case ACT_RUN:
	default:
		return 90.0f;
		break;
	}
}
void CNPC_VortBoss::SetAim(const Vector &aimDir, float flInterval)
{
	QAngle angDir;
	VectorAngles(aimDir, angDir);

	float newYaw = angDir[YAW];

	QAngle curAngle = GetLocalAngles();
	float curPitch = GetPoseParameter(gm_nBodyPitchPoseParam);
	float newPitch;

	if (GetEnemy())
	{
		// clamp and dampen movement
		newPitch = curPitch + 0.8 * UTIL_AngleDiff(UTIL_ApproachAngle(angDir.x, curPitch, 20), curPitch);

	}
	else
	{
		// Sweep your weapon more slowly if you're not fighting someone
		newPitch = curPitch + 0.6 * UTIL_AngleDiff(UTIL_ApproachAngle(angDir.x, curPitch, 20), curPitch);

	}

	newPitch = AngleNormalize(newPitch);

	SetPoseParameter(gm_nBodyPitchPoseParam, clamp(newPitch, -80, 40));
	QAngle NewAngle = QAngle(0, newYaw, 0);
	SetLocalAngles(NewAngle);
	//Msg( "pitch=%f, yaw=%f\n", newPitch, newYaw );
}
void CNPC_VortBoss::CreateTargetBeam(void) //setup the target beam
{
	QAngle angHandAng; //create a blank angle 
	float timeDelta; //create a blank float
	Vector vecEyePos,m_vLaserDir; //create 2 blank vectors

	GetAttachment(VORTBOSS_EYE_ATTACHMENT, vecEyePos, angHandAng); //store the angle and eye position
	float preAdjustedTimeDelta = 1; //preadjusted time delta
	float postAdjustedTimeDelta = g_pGameRules->SkillAdjustValue(preAdjustedTimeDelta); //adjust based on difficulty
	timeDelta = 1 / postAdjustedTimeDelta; //this doesn't work, but it doesn't matter, because this is just the creation, the actual prediction happens in the update function

	UTIL_PredictedPosition(GetEnemy(), timeDelta, &m_vTargetPos);
	Vector vecAdjustedTargetPos = m_vTargetPos + Vector(0, 0, 32);
	m_vLaserDir = vecAdjustedTargetPos - vecEyePos;
	VectorNormalize(m_vLaserDir);
	trace_t tr;
	UTIL_TraceLine(vecEyePos, vecEyePos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	
	pEyeTargetBeam = CBeam::BeamCreate("sprites/laser.vmt", 2); //init our beam
	pEyeTargetBeam->PointEntInit(tr.endpos, this);
	pEyeTargetBeam->SetEndAttachment(VORTBOSS_EYE_ATTACHMENT);
	pEyeTargetBeam->SetBrightness(255);
	pEyeTargetBeam->SetRenderColor(200, 0, 50);
	pEyeTargetBeam->SetNoise(0);
	SetContextThink(&CNPC_VortBoss::EyeBeamThink, gpGlobals->curtime, "EyeUpdateContext"); //start a loop of EyeBeamThink
}
void CNPC_VortBoss::EyeBeamThink(void) //update our target beam
{
	QAngle angHandAng;
	float timeDelta;
	Vector vecEyePos, m_vLaserDir;

	GetAttachment(VORTBOSS_EYE_ATTACHMENT, vecEyePos, angHandAng);
	float preAdjustedTimeDelta = 0.25; //our pre adjusted time delta is 0.25
	float postAdjustedTimeDelta = g_pGameRules->SkillAdjustValue(preAdjustedTimeDelta); //adjust based on difficulty level
	timeDelta = 0.5 - postAdjustedTimeDelta; 

	//so this happens because like i said before, our adjustment 
	//increases based on difficulty level, 
	//so in order to decrease our delta, we have to subtract it from 
	//something higher than the highest possible outcome
	//of our adjustment

	UTIL_PredictedPosition(GetEnemy(), timeDelta, &m_vTargetPos); //calculate our predicted position, which essentially predicts where our target is going to be in timeDelta amount of time
	Vector vecAdjustedTargetPos = m_vTargetPos + Vector(0, 0, 32); //adjust our calulated postion up 32 units to our target's worldcenter
	m_vLaserDir = vecAdjustedTargetPos - vecEyePos; //calculate the line from that point to our eyeball
	VectorNormalize(m_vLaserDir); //remove the magnitude to return only a direction
	trace_t tr;
	UTIL_TraceLine(vecEyePos, vecEyePos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr); //fire the trace from our eyeball in the direction we previously calculated
	pEyeTargetBeam->SetStartPos(tr.endpos); //set our start position, because we're starting our laser at the end of the trace instead of the start
	pEyeTargetBeam->RelinkBeam(); //update the beam
	SetNextThink(gpGlobals->curtime + 0.001f, "EyeUpdateContext"); //set the next think time of our loop

}
void CNPC_VortBoss::CreateAttackBeam(void)
{
	if (pEyeTargetBeam)
	{
		SetContextThink(NULL, gpGlobals->curtime, "EyeUpdateContext");
		pEyeTargetBeam->SetRenderColorA(0);
		pEyeTargetBeam = NULL;
		UTIL_Remove(pEyeTargetBeam);
	}
	iAttackBeamWidth = 64;
	QAngle angHandAng;
	Vector vecEyePos, m_vLaserDir;
	Vector vecAdjustedTargetPos = m_vTargetPos + Vector(0, 0, 32);
	m_vLaserDir = vecAdjustedTargetPos - vecEyePos;
	VectorNormalize(m_vLaserDir);
	GetAttachment(VORTBOSS_EYE_ATTACHMENT, vecEyePos, angHandAng);
	trace_t tr;
	UTIL_TraceLine(vecEyePos, vecEyePos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	m_vStartPos = vecEyePos;
	pEyeAttackBeam = CBeam::BeamCreate("sprites/laser.vmt", iAttackBeamWidth);
	pEyeAttackBeam->PointEntInit(tr.endpos, this);
	pEyeAttackBeam->SetEndAttachment(VORTBOSS_EYE_ATTACHMENT);
	pEyeAttackBeam->SetBrightness(255);
	pEyeAttackBeam->SetRenderColor(200, 0, 50);
	pEyeAttackBeam->SetNoise(0);
	SetContextThink(&CNPC_VortBoss::AttackBeamThink, gpGlobals->curtime, "AttackBeamContext");

}
void CNPC_VortBoss::AttackBeamThink(void)
{
	iAttackBeamWidth -= 4;
	Vector vecAdjustedTargetPos = m_vTargetPos + Vector(0, 0, 32);
	Vector m_vLaserDir = vecAdjustedTargetPos - m_vStartPos;

	trace_t tr;
	UTIL_TraceLine(m_vStartPos, m_vStartPos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	pEyeAttackBeam->SetStartPos(tr.endpos);
	pEyeAttackBeam->SetEndPos(tr.startpos);
	//DebugDrawLine(tr.startpos, tr.endpos, 0, 255, 0, false, 0.5f);
	pEyeAttackBeam->SetWidth(iAttackBeamWidth);
	pEyeAttackBeam->RelinkBeam();
	CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(tr.m_pEnt);
	if (pBCC)
	{
		ClearMultiDamage();
		CTakeDamageInfo info(this, this, 10, DMG_SHOCK);
		info.AdjustPlayerDamageTakenForSkillLevel();
		CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
		pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
		ApplyMultiDamage();
		DevMsg("applying damage\n");
	}
	SetNextThink(gpGlobals->curtime + 0.01f, "AttackBeamContext");

}
void CNPC_VortBoss::ClearBeam(void)
{
	if (pEyeTargetBeam)
	{
		SetContextThink(NULL, gpGlobals->curtime, "EyeUpdateContext");
		pEyeTargetBeam->SetRenderColorA(0);
		pEyeTargetBeam = NULL;
		UTIL_Remove(pEyeTargetBeam);
	}
	if (pEyeAttackBeam)
	{
		SetContextThink(NULL, gpGlobals->curtime, "AttackBeamContext");
		pEyeAttackBeam->SetRenderColorA(0);
		pEyeAttackBeam = NULL;
		UTIL_Remove(pEyeAttackBeam);
	}

}

Activity CNPC_VortBoss::NPC_TranslateActivity(Activity baseAct)
{
	if ((baseAct == ACT_RUN) && IsCurSchedule(SCHED_VORTBOSS_CHARGE))
		return (Activity)ACT_VORTBOSS_CHARGE_LOOP;
	return baseAct;
}

int CNPC_VortBoss::TranslateSchedule(int scheduletype)
{
	switch (scheduletype)
	{
	case SCHED_ESTABLISH_LINE_OF_FIRE:
	{
		return TranslateSchedule(SCHED_VORTBOSS_ESTABLISH_LOF);
		break;
	}
	case SCHED_RANGE_ATTACK1:
	{
		return SCHED_VORTBOSS_CANNON;
		break;
	}
	}
	return BaseClass::TranslateSchedule(scheduletype);

}

bool CNPC_VortBoss::OverrideMove(float flInterval)
{
	// If the guard's charging, we're handling the movement
	if (IsCurSchedule(SCHED_VORTBOSS_CHARGE))
		return true;

	// TODO: This code increases the guard's ability to successfully hit a target, but adds a new dimension of navigation
	//		 trouble to do with him not being able to "close the distance" between himself and the object he wants to hit.
	//		 Fixing this will require some thought on how he picks the correct distances to his targets and when he's "close enough". -- jdw

	/*
	if ( m_hPhysicsTarget != NULL )
	{
	float flWidth = m_hPhysicsTarget->CollisionProp()->BoundingRadius2D();
	GetLocalNavigator()->AddObstacle( m_hPhysicsTarget->WorldSpaceCenter(), flWidth * 0.75f, AIMST_AVOID_OBJECT );
	//NDebugOverlay::Sphere( m_hPhysicsTarget->WorldSpaceCenter(), vec3_angle, flWidth, 255, 255, 255, 0, true, 0.5f );
	}
	*/

	return BaseClass::OverrideMove(flInterval);
}

bool CNPC_VortBoss::ShouldCharge(const Vector &startPos, const Vector &endPos, bool useTime, bool bCheckForCancel)
{
	// Don't charge in tight spaces unless forced to

	/*if (gpGlobals->curtime > m_fNextFireball)
	return false;
	if (gpGlobals->curtime > m_fNextBombard)
	return false;*/
	// Must have a target
	if (!GetEnemy())
		return false;

	// No one else in the squad can be charging already
	/*if (IsStrategySlotRangeOccupied(SQUAD_SLOT_ANTLIONWARRIOR_CHARGE, SQUAD_SLOT_ANTLIONWARRIOR_CHARGE))
		return false;*/

	// Don't check the distance once we start charging

	//FIXME: We'd like to exclude small physics objects from this check!

	// We only need to hit the endpos with the edge of our bounding box
	Vector vecDir = endPos - startPos;
	VectorNormalize(vecDir);
	float flWidth = WorldAlignSize().x * 0.5;
	Vector vecTargetPos = endPos - (vecDir * flWidth);

	// See if we can directly move there
	AIMoveTrace_t moveTrace;
	GetMoveProbe()->MoveLimit(NAV_GROUND, startPos, vecTargetPos, MASK_NPCSOLID_BRUSHONLY, GetEnemy(), &moveTrace);

	// Draw the probe

	// If we're not blocked, charge
	if (IsMoveBlocked(moveTrace))
	{
		// Don't allow it if it's too close to us

		// Allow some special cases to not block us
		if (moveTrace.pObstruction != NULL)
		{
			// If we've hit the world, see if it's a cliff
			if (moveTrace.pObstruction == GetContainingEntity(INDEXENT(0)))
			{
				// Can't be too far above/below the target
				if (fabs(moveTrace.vEndPosition.z - vecTargetPos.z) > StepHeight())
					return false;

				// Allow it if we got pretty close
				if (UTIL_DistApprox(moveTrace.vEndPosition, vecTargetPos) < 64)
					return true;
			}

			// Hit things that will take damage
			if (moveTrace.pObstruction->m_takedamage != DAMAGE_NO)
				return true;

			// Hit things that will move
			if (moveTrace.pObstruction->GetMoveType() == MOVETYPE_VPHYSICS)
				return true;
		}

		return false;
	}

	// Only update this if we've requested it

	return true;
}
bool CNPC_VortBoss::ShouldSwing(CBaseEntity* pTarget)
{
	if (!pTarget)
		return false;
	if (m_fNextSwing > gpGlobals->curtime)
		return false;

	Vector myPos = GetAbsOrigin();
	float myZpos = myPos[2];
	Vector enemyPos = pTarget->GetAbsOrigin();
	float enemyZpos = enemyPos[2];
	float zCompare = abs(myZpos - enemyZpos);
	if (zCompare > 192.0f)
		return false;
	if (UTIL_IsFacingWithinTolerance(this,pTarget,0.8f))
		return true;

	return false;

}
void CNPC_VortBoss::HandleAnimEvent(animevent_t *pEvent)
{
	if (pEvent->event == AE_VORTBOSS_FIRECANNON) //we're firing the cannon
	{
		int nAttachment = LookupAttachment("cannonmuzzle"); //get the cannon attachment
		Vector vecHandPos;
		QAngle vecHandAng;
		GetAttachment(nAttachment, vecHandPos, vecHandAng);				//get the world position of our attachment
		CHLRVortProjectile *pVort = (CHLRVortProjectile*)CreateEntityByName("hlr_vortprojectile"); //create our projectile
		UTIL_SetOrigin(pVort, vecHandPos);								//set the projectile's position to the attachment position
		pVort->Spawn();													//initialize the projectile

		Vector enemyDelta = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter(); //get the vector from me to the enemy
		float flDist = enemyDelta.Length();							//get the length of the vector
		float fBaseSpeed = sk_vortboss_projectile_speed.GetFloat(); //get the base projectile speed
		float fAdjustedSpeed = g_pGameRules->SkillAdjustValue(fBaseSpeed); //scale the projectile speed based on difficulty
		float timeDelta = flDist / fAdjustedSpeed;						//calculate the time it'll take to reach the target

		Vector vecTargetPos;
		Vector vecTargetCenter;
		UTIL_PredictedPosition(GetEnemy(), timeDelta, &vecTargetPos); //get the predicted enemy position in the given amount of time it'll take to reach the enemy
		if (GetEnemy()->GetGroundEntity() != NULL)					//if our enemy is on the ground
		{
			vecTargetCenter = vecTargetPos + Vector(0, 0, 32); //shoot at his torso
		}
		else //if he is NOT on the ground
		{
			vecTargetCenter = vecTargetPos; //shoot at his feet
		}
		Vector vecAim = vecTargetCenter - vecHandPos; //get the vector from the cannon attachment to the target position
		VectorNormalize(vecAim); //normalize the vector

		pVort->SetAbsVelocity(vecAim * fAdjustedSpeed); //fire the projectile
		return;
	}
	if (pEvent->event == AE_VORTBOSS_START_SPINATTACK)
	{
		QAngle angCurAngles;
		angCurAngles = GetLocalAngles();
		QAngle angNewAngle = angCurAngles;
		angNewAngle[PITCH] = 0;
		SetLocalAngles(angNewAngle);
		StartSpinBeam();
		bDrawSpinBeam = true;
		return;
	}
	if (pEvent->event == AE_VORTBOSS_END_SPINATTACK)
	{
		bDrawSpinBeam = false;
		ClearSpinBeam();
		return;
	}
	if (pEvent->event == AE_VORTBOSS_GROUNDATTACK)
	{
		ClearBeam();
		RadiusDamage(CTakeDamageInfo(this, this, 50, DMG_SONIC), GetAbsOrigin(), 300, CLASS_VORTIGAUNT, NULL);
		DispatchParticleEffect("vortboss_ground_attack", GetAbsOrigin(), GetAbsAngles(), this);
		EmitSound("BadDog.Smash");
		UTIL_ScreenShake(GetAbsOrigin(), 40.0, 60, 1.0, 500, SHAKE_START);
		return;
	}
	if (pEvent->event == AE_VORTBOSS_START_TARGETBEAM)
	{
		CreateTargetBeam();
		return;
	}
	if (pEvent->event == AE_VORTBOSS_FIRE_ATTACKBEAM)
	{
		EmitSound("VortBoss.EyeBlast");
		CreateAttackBeam();
		return;
	}
	if (pEvent->event == AE_VORTBOSS_CLEAR_BEAM)
	{
		ClearBeam();
		return;
	}
	if (pEvent->event == AE_VORTBOSS_ROCKET_LAUNCH)
	{
		FirePrecalculatedRocket();
		return;
	}
	if (pEvent->event == AE_VORTBOSS_SWING_IMPACT)
	{
		Vector right, forward, dir;
		AngleVectors(GetLocalAngles(), &forward, &right, NULL);
		//
		// Trace out a cubic section of our hull and see what we hit.
		//
		Vector vecMins = GetHullMins();
		Vector vecMaxs = GetHullMaxs();
		vecMins.z = vecMins.x;
		vecMaxs.z = vecMaxs.x;

		CBaseEntity* pHurt = CheckTraceHullAttack(80, vecMins, vecMaxs, 1, DMG_SLASH);

		if (pHurt)
		{
			CBasePlayer* pPlayer = ToBasePlayer(pHurt);
			if (pPlayer)
				{
				pPlayer->ViewPunch(QAngle(20, 20, -30));

				Vector	dir = pPlayer->GetAbsOrigin() - GetAbsOrigin();
				VectorNormalize(dir);
				dir.z = 0.0f;

				Vector vecNewVelocity = dir * 1650.0f;
				vecNewVelocity[2] += 650.0f;
				//pPlayer->VelocityPunch(vecNewVelocity);
				DevMsg("throwing player\n");
				}
			ChargeDamage(pPlayer);
		}
	}
	BaseClass::HandleAnimEvent(pEvent);
}

int CNPC_VortBoss::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	CTakeDamageInfo dInfo = info;
	if (GetIdealActivity() == ACT_VORTBOSS_SPINATTACK)
	{
		dInfo.ScaleDamage(0.2f);
	}
	if (GetIdealActivity() == ACT_VORTBOSS_EYEBLAST)
	{
		dInfo.ScaleDamage(0.3f);
	}

	UpdateBodyState();
	return BaseClass::OnTakeDamage_Alive(dInfo);
}

void CNPC_VortBoss::Event_Killed(const CTakeDamageInfo &info)
{
	bBleeding = false;
	ClearBeam();
	BaseClass::Event_Killed(info);
}

void CNPC_VortBoss::UpdateBodyState(void)
{
	static int BodyGroup_Body = FindBodygroupByName("body");
	if (GetHealth() < (GetMaxHealth() * 0.25) && m_iBodyState != BODYSTATE_BADLYDAMAGED)
	{
		m_iBodyState = BODYSTATE_BADLYDAMAGED;
		SetBodygroup(BodyGroup_Body, 1);
		bBleeding = true;
		m_nSkin = 2;
		DispatchParticleEffect("hlr_base_explosion1", WorldSpaceCenter(), vec3_angle, this);
	}
}
void CNPC_VortBoss::SpinAttackThink(void)
{
	/*int nAttachment = LookupAttachment("cannonmuzzle");
	Vector vecHandPos;
	QAngle vecHandAng;
	Vector vecAiming;
	//float fNextShot;
	float fBaseSpeed = sk_vortboss_projectile_speed.GetFloat();
	GetAttachment(nAttachment, vecHandPos, vecHandAng);
	AngleVectors(vecHandAng, &vecAiming);
	CHLRVortProjectile *pVort = (CHLRVortProjectile*)CreateEntityByName("hlr_vortprojectile");
	UTIL_SetOrigin(pVort, vecHandPos);
	pVort->Spawn();
	VectorNormalize(vecAiming);
	vecAiming[ROLL] = 0;
	pVort->SetAbsVelocity(vecAiming * fAdjustedSpeed);*/
	/*Vector vecHandPos, m_vLaserDir;
	QAngle angHandAng;
	GetAttachment(VORTBOSS_HAND_ATTACHMENT, vecHandPos, angHandAng);
	AngleVectors(angHandAng, &m_vLaserDir);
	trace_t tr;
	m_vLaserDir[ROLL] = 0;
	UTIL_TraceLine(vecHandPos, vecHandPos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	//DebugDrawLine(tr.startpos, tr.endpos, 0, 255, 0, false, 0.5);
	CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(tr.m_pEnt);
	if (pBCC)
	{
		DevMsg("server hit\n");
	}
	//pSpinBeam->SetStartPos(tr.endpos);
	//pSpinBeam->RelinkBeam();
	SetNextThink(gpGlobals->curtime + 0.001f, "SpinAttackContext");*/
}
bool CNPC_VortBoss::EnemyIsRightInFrontOfMe(CBaseEntity **pEntity)
{
	if (!GetEnemy())
		return false;

	if ((GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter()).LengthSqr() < (64 * 64))
	{
		Vector vecLOS = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
		vecLOS.z = 0;
		VectorNormalize(vecLOS);
		Vector vBodyDir = BodyDirection2D();
		if (DotProduct(vecLOS, vBodyDir) > 0.8)
		{
			// He's in front of me, and close. Make sure he's not behind a wall.
			trace_t tr;
			UTIL_TraceLine(WorldSpaceCenter(), GetEnemy()->EyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
			if (tr.m_pEnt == GetEnemy())
			{
				*pEntity = tr.m_pEnt;
				return true;
			}
		}
	}

	return false;
}

bool CNPC_VortBoss::HandleChargeImpact(Vector vecImpact, CBaseEntity *pEntity)
{

	if (!pEntity || pEntity->IsWorld())
	{
		EnemyIsRightInFrontOfMe(&pEntity);

		// Did we manage to find him? If not, increment our charge miss count and abort.
		if (pEntity->IsWorld())
		{
			return true;
		}

	}

	if (pEntity->ClassMatches("plasma_ball"))
		return false;
	// Cause a shock wave from this point which will disrupt nearby physics objects
	// Hit anything we don't like
	if ((IRelationType(pEntity) == D_NU || D_HT) && (GetNextAttack() < gpGlobals->curtime))
	{
		EmitSound("NPC_AntlionGuard.Shove");

		ChargeDamage(pEntity);

		pEntity->ApplyAbsVelocityImpulse((BodyDirection2D() * 800) + Vector(0, 0, 400));

		if (!pEntity->IsAlive() && GetEnemy() == pEntity)
		{
			SetEnemy(NULL);
		}

		SetNextAttack(gpGlobals->curtime + 2.0f);
		SetActivity(ACT_VORTBOSS_CHARGE_STOP);

		// We've hit something, so clear our miss count

		return false;
	}

	// Hit something we don't hate. If it's not moveable, crash into it.
	if (pEntity->GetMoveType() == MOVETYPE_NONE || pEntity->GetMoveType() == MOVETYPE_PUSH)
		return true;

	// If it's a vphysics object that's too heavy, crash into it too.
	if (pEntity->GetMoveType() == MOVETYPE_VPHYSICS)
	{
		IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
		if (pPhysics)
		{
			// If the object is being held by the player, knock it out of his hands
			if (pPhysics->GetGameFlags() & FVPHYSICS_PLAYER_HELD)
			{
				Pickup_ForcePlayerToDropThisObject(pEntity);
				return false;
			}

			if ((!pPhysics->IsMoveable() || pPhysics->GetMass() > VPhysicsGetObject()->GetMass() * 0.5f))
				return true;
		}
	}

	return false;
}
float CNPC_VortBoss::ChargeSteer(void)
{
	trace_t	tr;
	Vector	testPos, steer, forward, right;
	QAngle	angles;
	const float	testLength = m_flGroundSpeed * 0.15f;

	//Get our facing
	GetVectors(&forward, &right, NULL);

	steer = forward;

	const float faceYaw = UTIL_VecToYaw(forward);

	//Offset right
	VectorAngles(forward, angles);
	angles[YAW] += 45.0f;
	AngleVectors(angles, &forward);

	//Probe out
	testPos = GetAbsOrigin() + (forward * testLength);

	//Offset by step height
	Vector testHullMins = GetHullMins();
	testHullMins.z += (StepHeight() * 2);

	//Probe

	//Add in this component
	steer += (right * 0.5f) * (1.0f - tr.fraction);

	//Offset left
	angles[YAW] -= 90.0f;
	AngleVectors(angles, &forward);

	//Probe out
	testPos = GetAbsOrigin() + (forward * testLength);

	// Probe

	//Add in this component
	steer -= (right * 0.5f) * (1.0f - tr.fraction);

	//Debug
	return UTIL_AngleDiff(UTIL_VecToYaw(steer), faceYaw);
}

void CNPC_VortBoss::StartSpinBeam(void)
{
	Vector vecHandPos, m_vLaserDir;
	QAngle angHandAng;
	GetAttachment(VORTBOSS_HAND_ATTACHMENT, vecHandPos, angHandAng);
	AngleVectors(angHandAng, &m_vLaserDir);
	trace_t tr;
	m_vLaserDir[ROLL] = 0;
	AI_TraceLine(vecHandPos, vecHandPos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	/*pSpinBeam = CBeam::BeamCreate("sprites/laser.vmt", 16.0);
	pSpinBeam->PointEntInit(tr.endpos, this);
	pSpinBeam->SetEndAttachment(VORTBOSS_HAND_ATTACHMENT);
	pSpinBeam->SetBrightness(255);
	pSpinBeam->SetNoise(0);*/
	SetContextThink(&CNPC_VortBoss::SpinAttackThink, gpGlobals->curtime + 0.001f, "SpinAttackContext");
}
void CNPC_VortBoss::ClearSpinBeam(void)
{
	//UTIL_Remove(pSpinBeam);
	//pSpinBeam = NULL;
	SetContextThink(NULL, gpGlobals->curtime, "SpinAttackContext");

}
void CNPC_VortBoss::GatherConditions(void)
{
	if (CheckForRockets())
	{
		SetCondition(COND_SHOULD_DODGE_ROCKET);
	}
	if (GetEnemy())
	{
		Vector enemyDelta = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter(); //get the distance from me to my enemy
		float flDist = enemyDelta.Length();
		if (flDist < 128.0f) //if my enemy is within pounding range 
		{
			if (ShouldSwing(GetEnemy()))
				SetCondition(COND_SHOULD_SWING);
			else
				SetCondition((COND_CAN_DO_GROUND_ATTACK)); //ground pound baby 
		}
		if (flDist < 300.0f && flDist > 128.0f) //if the enemy is too close to shoot and too far to ground pound
		{

			if (ShouldSwing(GetEnemy()))
				SetCondition(COND_SHOULD_SWING);
			else if (ShouldCharge(GetAbsOrigin(), GetEnemy()->GetAbsOrigin(), false, false))
				SetCondition(COND_SHOULD_CHARGE);
			else
				ClearCondition(COND_SHOULD_CHARGE);

			/*if (ShouldCharge(GetAbsOrigin(), GetEnemy()->GetAbsOrigin(), false, false))
				SetCondition(COND_SHOULD_CHARGE);
			else
				ClearCondition(COND_SHOULD_CHARGE);//tell me that so i can move away
			//ClearCondition((COND_CAN_DO_GROUND_ATTACK));*/
		}
		if (flDist > 300.0f && flDist < 1024 && HasCondition(COND_HAVE_ENEMY_LOS)) //if my enemy is within the acceptable attack range and i can see them
		{
			if (gpGlobals->curtime > m_fNextSpinAttack) //if my spin attack cooldown has passed
			{
				//compare the Z values of me and my enemy
				Vector vecOrigin, vecEnemyOrigin;
				float vecOriginZ, vecEnemyOriginZ;
				vecOrigin = GetAbsOrigin();
				vecEnemyOrigin = GetEnemy()->GetAbsOrigin();
				vecOriginZ = vecOrigin[2];
				vecEnemyOriginZ = vecEnemyOrigin[2];
				float ZCompare = vecOriginZ - vecEnemyOriginZ;

				if (ZCompare < 16.0f && ZCompare > -16.0f) //if they're close to the same level as me and I can see them
					SetCondition(COND_CAN_SPIN_ATTACK); //allow me to spin attack
			}
			if (CanBarage() && gpGlobals->curtime > m_fNextBarage)
				SetCondition(COND_SHOULD_BARAGE);
			if (gpGlobals->curtime > m_fNextEyeBlast) //if my eyeblast cooldown has passed
				SetCondition(COND_CAN_DO_EYEBLAST); //tell me that so i can eyeblast. eyeblast takes precidence over cannon.
			if (gpGlobals->curtime > m_fNextCannonAttack) //if my cannon cooldown has passed
				SetCondition(COND_CAN_FIRE_CANNON); //tell me that so i can shoot the cannon
			
			SetCondition(COND_SHOULD_CHARGE); //otherwise, just charge them
		}
		if (flDist > 1024.0f)
		{
			if (ShouldCharge(GetAbsOrigin(), GetEnemy()->GetAbsOrigin(), true, false))//if they're too far away
				SetCondition(COND_SHOULD_CHARGE);
			else
				ClearCondition(COND_SHOULD_CHARGE);//tell me that so i can charge them
		}
	}
	BaseClass::GatherConditions();
}
void CNPC_VortBoss::RelaxAim(float flInterval)
{
	QAngle curAngle = GetAbsAngles();
	int curPitch = curAngle[PITCH];
	int curYaw = curAngle[YAW];

	// dampen existing aim
	float newPitch = AngleNormalize(UTIL_ApproachAngle(0, curPitch, 3));
	float newYaw = AngleNormalize(UTIL_ApproachAngle(0, curYaw, 2));

	QAngle NewAngle = QAngle(newPitch, newYaw, 0);
	SetAbsAngles(NewAngle);
}
void ApplyChargeImpactDamage(CBaseEntity *pCharger, CBaseEntity *pTarget, float flDamage)
{
	Vector attackDir = (pTarget->WorldSpaceCenter() - pCharger->WorldSpaceCenter());
	VectorNormalize(attackDir);
	Vector offset = RandomVector(-32, 32) + pTarget->WorldSpaceCenter();

	// Generate enough force to make a 75kg guy move away at 700 in/sec
	Vector vecForce = attackDir * ImpulseScale(75, 1200);

	// Deal the damage
	CTakeDamageInfo	info(pCharger, pCharger, vecForce, offset, flDamage, DMG_CLUB);
	pTarget->TakeDamage(info);
}
void CNPC_VortBoss::ChargeDamage(CBaseEntity *pTarget)
{
	if (pTarget == NULL)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pTarget);

	if (pPlayer != NULL)
	{
		//Kick the player angles
		pPlayer->ViewPunch(QAngle(20, 20, -30));

		Vector	dir = pPlayer->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize(dir);
		dir.z = 0.0f;

		Vector vecNewVelocity = dir * 250.0f;
		vecNewVelocity[2] += 128.0f;
		pPlayer->SetAbsVelocity(vecNewVelocity);

		color32 red = { 128, 0, 0, 128 };
		UTIL_ScreenFade(pPlayer, red, 1.0f, 0.1f, FFADE_IN);
	}

	// Player takes less damage
	float flDamage = (pPlayer == NULL) ? 250 : sk_vortboss_dmg_charge.GetFloat();

	// If it's being held by the player, break that bond
	Pickup_ForcePlayerToDropThisObject(pTarget);

	// Calculate the physics force
	ApplyChargeImpactDamage(this, pTarget, flDamage);
}


int CNPC_VortBoss::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_COMBAT && GetEnemy())
		return SelectCombatSchedule();
	return BaseClass::SelectSchedule();
}
int CNPC_VortBoss::SelectCombatSchedule(void)
{
	if (HasCondition(COND_SHOULD_SWING))
		return SCHED_VORTBOSS_SWING;
	if (HasCondition((COND_CAN_DO_GROUND_ATTACK))) //if i can ground attack
		return SCHED_VORTBOSS_GROUND_ATTACK; //do that
	if (HasCondition((COND_TOO_FAR_TO_ATTACK))) //if the enemy is too far
		return SCHED_CHASE_ENEMY; //get closer
	if (HasCondition((COND_TOO_CLOSE_TO_ATTACK))) //if the enemy is too close but not close enough to ground pound
		return SCHED_MOVE_AWAY;  //move further away
	if (HasCondition(COND_ENEMY_OCCLUDED))
		return SCHED_ESTABLISH_LINE_OF_FIRE;

	if (HasCondition(COND_SHOULD_DODGE_ROCKET))
		return SCHED_VORTBOSS_DODGE_ROCKET;

	/*if (HasCondition(COND_CAN_SPIN_ATTACK)) //if i can spin
	{
		ClearCondition(COND_CAN_SPIN_ATTACK);
		m_fNextSpinAttack = gpGlobals->curtime + 45.0f; //set cooldown
		return SCHED_VORTBOSS_SPINATTACK; //speen
	}*/
	if (HasCondition(COND_SHOULD_BARAGE))
	{
		ClearCondition(COND_SHOULD_BARAGE);
		m_fNextBarage = gpGlobals->curtime + sk_vortboss_barage_frequency.GetFloat();
		return SCHED_VORTBOSS_BARAGE;
	}
	if (HasCondition(COND_CAN_DO_EYEBLAST)) //if i can eyeblast
	{
		ClearCondition(COND_CAN_DO_EYEBLAST);
		m_fNextEyeBlast = gpGlobals->curtime + sk_vortboss_eyeblast_frequency.GetFloat(); //set cooldown
		return SCHED_VORTBOSS_EYEBLAST; //blast away
	}
	if (HasCondition(COND_CAN_FIRE_CANNON)) //if i can shoot my cannon
	{
		ClearCondition(COND_CAN_FIRE_CANNON);
		m_fNextCannonAttack = gpGlobals->curtime + sk_vortboss_cannon_frequency.GetFloat(); //set cooldown
		return SCHED_VORTBOSS_CANNON; //pew pew pew
	}
	if (HasCondition(COND_SHOULD_CHARGE))
	{
		return SCHED_VORTBOSS_CHARGE;
	}
	return BaseClass::SelectSchedule();
}
void CNPC_VortBoss::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_VORTBOSS_CANNON:
	{
		SetIdealActivity(ACT_VORTBOSS_CANNON);
		int iAttachment = LookupAttachment("cannonmuzzle");
		DispatchParticleEffect("vortboss_cannon_core", PATTACH_POINT_FOLLOW, this, iAttachment, true);
		break;
	}

	case TASK_VORTBOSS_BARAGE:
	{
		SetIdealActivity(ACT_VORTBOSS_BARAGE);
		break;
	}
	case TASK_VORTBOSS_EYEBLAST:
	{
		m_bShouldDrawShieldOverlay = true;
		SetIdealActivity(ACT_VORTBOSS_EYEBLAST);
		break;
	}
	case TASK_VORTBOSS_SPINATTACK:
	{
		SetIdealActivity(ACT_VORTBOSS_SPINATTACK);
		break;
	}
	case TASK_VORTBOSS_GROUNDATTACK:
	{
		SetIdealActivity(ACT_VORTBOSS_GROUND_ATTACK);
		break;
	}
	case TASK_VORTBOSS_CHARGE:
	{
		GetMotor()->MoveStop();
		SetActivity(ACT_VORTBOSS_CHARGE_START);
		break;
	}
	case TASK_VORTBOSS_DODGEROCKET:
	{
		GetMotor()->MoveStop();
		QAngle ang = GetLocalAngles();
		Vector vecF,vecR,vecU;
		AngleVectors(ang, &vecF,&vecR,&vecU);
		trace_t tr,tr2;
		UTIL_TraceLine(WorldSpaceCenter(), WorldSpaceCenter() + (vecR * 150), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		UTIL_TraceLine(WorldSpaceCenter(), WorldSpaceCenter() + (-vecR * 150), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr2);

		if ((tr.fraction == 1) && (tr2.fraction == 1))
		{
			int i = RandomInt(0, 1);
			switch (i)
			{
			case 0:
			{
				SetActivity(ACT_VORTBOSS_DODGE_RIGHT);
				break;
			}
			case 1:
			{
				SetActivity(ACT_VORTBOSS_DODGE_LEFT);
				break;
			}
			default:
			{
				SetActivity(ACT_VORTBOSS_DODGE_RIGHT);
				break;
			}
			}
		}
		if ((tr.fraction == 1) && (tr2.fraction != 1))
			SetActivity(ACT_VORTBOSS_DODGE_RIGHT);
		else if ((tr.fraction != 1) && (tr2.fraction == 1))
			SetActivity(ACT_VORTBOSS_DODGE_LEFT);
		m_fNextDodge = gpGlobals->curtime + 2.5f;
		ClearBeam();
		break;
	}
	case TASK_VORTBOSS_SWING:
	{
		SetIdealActivity(ACT_VORTBOSS_SWING);
		float nextswing;
		int skill = g_pGameRules->GetSkillLevel();
		switch (skill)
		{
		case SKILL_EASY:
		{
			nextswing = 2.0f;
			break;
		}
		case SKILL_MEDIUM:
		{
			nextswing = 1.0f;
			break;
		}
		case SKILL_HARD:
		{
			nextswing = 0.5f;
			break;
		}
		default:
		{
			nextswing = 1.0f;
			break;
		}
		}
		m_fNextSwing = gpGlobals->curtime + nextswing;
		break;
	}
	default:
	{
		BaseClass::StartTask( pTask );
		break;
	}
	}

}
void CNPC_VortBoss::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_VORTBOSS_CANNON:
	{
		UpdateAim();
		if (IsActivityFinished())
		{

			TaskComplete();
		}
		break;
	}
	case TASK_VORTBOSS_EYEBLAST:
	{
		UpdateAim();
		if (IsActivityFinished())
		{
			m_bShouldDrawShieldOverlay = false;
			TaskComplete();
		}
		break;
	}
	case TASK_VORTBOSS_BARAGE:
	{
		UpdateAim();
		if (IsActivityFinished())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_VORTBOSS_SPINATTACK:
	{
		int skill = g_pGameRules->GetSkillLevel();
		switch (skill)
		{
		case SKILL_EASY:
		{
			break;
		}
		case SKILL_MEDIUM:
		{
			SetPlaybackRate(1.1f);
			break;
		}
		case SKILL_HARD:
		{
			SetPlaybackRate(1.3f);
			break;
		}
		default:
		{
			break;
		}
		}
		if (IsActivityFinished())
		{
			SetPlaybackRate(1.0f);
			TaskComplete();
		}
		break;
	}
	case TASK_VORTBOSS_GROUNDATTACK:
	{
		if (IsActivityFinished())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_VORTBOSS_DODGEROCKET:
	{
		AutoMovement();
		if (IsActivityFinished())
		{
			TaskComplete();
			return;
		}
		break;
	}
	case TASK_VORTBOSS_CHARGE:
	{
		int skill = g_pGameRules->GetSkillLevel();
		switch (skill)
		{
		case SKILL_EASY:
		{
			SetPlaybackRate(0.75f);
			break;
		}
		case SKILL_MEDIUM:
		{
			SetPlaybackRate(0.9f);
			break;
		}
		case SKILL_HARD:
		{
			SetPlaybackRate(1.1f);
			break;
		}
		default:
		{
			break;
		}
		}
		Activity eActivity = GetActivity();

		if (eActivity == ACT_VORTBOSS_CHARGE_STOP || eActivity == ACT_VORTBOSS_CRASH)
		{
			SetPlaybackRate(1.0f);
			if (IsActivityFinished())
			{
				TaskComplete();
				return;
			}
			AutoMovement();
			return;
		}
		if ((eActivity == ACT_VORTBOSS_CHARGE_START) && (IsActivityFinished()))
		{
			SetActivity(ACT_VORTBOSS_CHARGE_LOOP);
		}
		if (eActivity == ACT_VORTBOSS_CHARGE_LOOP || eActivity == ACT_VORTBOSS_CHARGE_START)
		{
			if (HasCondition(COND_NEW_ENEMY) || HasCondition(COND_LOST_ENEMY) || HasCondition(COND_ENEMY_DEAD))
			{
				SetActivity(ACT_VORTBOSS_CHARGE_STOP);
				return;
			}
			else
			{
				if (GetEnemy() != NULL)
				{
					Vector	goalDir = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
					VectorNormalize(goalDir);

					if (DotProduct(BodyDirection2D(), goalDir) < 0.25f)
					{
							// We've missed the target. Randomly decide not to stop, which will cause
							// the guard to just try and swing around for another pass.
							SetActivity(ACT_VORTBOSS_CHARGE_STOP);

					}
				}
			}
		}

		float idealYaw;
		if (GetEnemy() == NULL)
		{
			idealYaw = GetMotor()->GetIdealYaw();
		}
		else
		{
			idealYaw = CalcIdealYaw(GetEnemy()->GetAbsOrigin());
		}

		// Add in our steering offset
		//idealYaw += ChargeSteer();

		// Turn to face
		GetMotor()->SetIdealYawAndUpdate(idealYaw);

		AIMoveTrace_t moveTrace;
		if (AutoMovement(GetEnemy(), &moveTrace) == false)
		{
			// Only stop if we hit the world
			if (HandleChargeImpact(moveTrace.vEndPosition, moveTrace.pObstruction))
			{
				// If we're starting up, this is an error
				if (eActivity == ACT_VORTBOSS_CHARGE_START)
				{
					TaskFail("Unable to make initial movement of charge\n");
					return;
				}

				// Crash unless we're trying to stop already
				if (eActivity != ACT_VORTBOSS_CHARGE_STOP)
				{
					if (moveTrace.fStatus == AIMR_BLOCKED_WORLD && moveTrace.vHitNormal == vec3_origin)
					{
						SetActivity(ACT_VORTBOSS_CRASH);
					}
					else
					{
						SetActivity(ACT_VORTBOSS_CRASH);
					}
				}
			}
			else if (moveTrace.pObstruction)
			{
				// If we hit an antlion, don't stop, but kill it
				if (moveTrace.pObstruction->Classify() == CLASS_VORTIGAUNT)
				{
					if (FClassnameIs(moveTrace.pObstruction, "npc_vortboss"))
					{
						// Crash unless we're trying to stop already
						if (eActivity != ACT_VORTBOSS_CHARGE_STOP)
						{
							SetActivity(ACT_VORTBOSS_CHARGE_STOP);
						}
					}
					else
					{
						ApplyChargeImpactDamage(this, moveTrace.pObstruction, moveTrace.pObstruction->GetHealth());
					}
				}
			}
		}
		break;
	}
	case TASK_VORTBOSS_SWING:
	{
		AutoMovement();
		if (IsActivityFinished())
		{
			TaskComplete();
		}
		/*//Get the ideal angle to swing at
		float idealYaw;
		if (GetEnemy() == NULL)
		{
			idealYaw = GetMotor()->GetIdealYaw();
		}
		else
		{
			idealYaw = CalcIdealYaw(GetEnemy()->GetAbsOrigin());
		}
		//Set myself to that angle
		GetMotor()->SetIdealYawAndUpdate(idealYaw);
		
		//trace our movement
		AIMoveTrace_t moveTrace;*/
		break;
	}
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}

void CC_vorthurt(const CCommand &args)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	pPlayer->TakeDamage(CTakeDamageInfo(pPlayer, pPlayer, 25, DMG_PREVENT_PHYSICS_FORCE));
}

static ConCommand vorthurt("vorthurt", CC_vorthurt, "vorthurt", FCVAR_REPLICATED | FCVAR_HIDDEN | FCVAR_CLIENTCMD_CAN_EXECUTE);


AI_BEGIN_CUSTOM_NPC(npc_vortboss, CNPC_VortBoss)

	DECLARE_ACTIVITY(ACT_VORTBOSS_CANNON)
	DECLARE_ACTIVITY(ACT_VORTBOSS_EYEBLAST)
	DECLARE_ACTIVITY(ACT_VORTBOSS_GROUND_ATTACK)
	DECLARE_ACTIVITY(ACT_VORTBOSS_SPINATTACK)
	DECLARE_ACTIVITY(ACT_VORTBOSS_CHARGE_LOOP)
	DECLARE_ACTIVITY(ACT_VORTBOSS_CHARGE_START)
	DECLARE_ACTIVITY(ACT_VORTBOSS_CHARGE_STOP)
	DECLARE_ACTIVITY(ACT_VORTBOSS_CRASH)
	DECLARE_ACTIVITY(ACT_VORTBOSS_DODGE_LEFT)
	DECLARE_ACTIVITY(ACT_VORTBOSS_DODGE_RIGHT)
	DECLARE_ACTIVITY(ACT_VORTBOSS_BARAGE)
	DECLARE_ACTIVITY(ACT_VORTBOSS_SWING)


	DECLARE_ANIMEVENT(AE_VORTBOSS_FIRECANNON)
	DECLARE_ANIMEVENT(AE_VORTBOSS_CHARGECANNON)
	DECLARE_ANIMEVENT(AE_VORTBOSS_FIRE_ATTACKBEAM)
	DECLARE_ANIMEVENT(AE_VORTBOSS_START_TARGETBEAM)
	DECLARE_ANIMEVENT(AE_VORTBOSS_CLEAR_BEAM)
	DECLARE_ANIMEVENT(AE_VORTBOSS_GROUNDATTACK)
	DECLARE_ANIMEVENT(AE_VORTBOSS_START_SPINATTACK)
	DECLARE_ANIMEVENT(AE_VORTBOSS_END_SPINATTACK)
	DECLARE_ANIMEVENT(AE_VORTBOSS_ROCKET_LAUNCH)
	DECLARE_ANIMEVENT(AE_VORTBOSS_SWING_IMPACT)

	DECLARE_CONDITION(COND_CAN_DO_EYEBLAST)
	DECLARE_CONDITION(COND_CAN_DO_GROUND_ATTACK)
	DECLARE_CONDITION(COND_CAN_FIRE_CANNON)
	DECLARE_CONDITION(COND_CAN_SPIN_ATTACK)
	DECLARE_CONDITION(COND_SHOULD_CHARGE)
	DECLARE_CONDITION(COND_SHOULD_DODGE_ROCKET)
	DECLARE_CONDITION(COND_SHOULD_BARAGE)
	DECLARE_CONDITION(COND_SHOULD_SWING)

	DECLARE_SQUADSLOT(SQUAD_SLOT_VORTBOSS_SPINBEAM)

	DECLARE_TASK(TASK_VORTBOSS_CANNON)
	DECLARE_TASK(TASK_VORTBOSS_EYEBLAST)
	DECLARE_TASK(TASK_VORTBOSS_GROUNDATTACK)
	DECLARE_TASK(TASK_VORTBOSS_SPINATTACK)
	DECLARE_TASK(TASK_VORTBOSS_CHARGE)
	DECLARE_TASK(TASK_VORTBOSS_DODGEROCKET)
	DECLARE_TASK(TASK_VORTBOSS_BARAGE)
	DECLARE_TASK(TASK_VORTBOSS_SWING)

	DEFINE_SCHEDULE
	(
			SCHED_VORTBOSS_GROUND_ATTACK,

			"	Tasks"
			"		TASK_STOP_MOVING					0"
			"		TASK_FACE_ENEMY						0"
			"		TASK_VORTBOSS_GROUNDATTACK			0"
			"	"
			"	Interrupts"
			"		COND_SHOULD_SWING"
			"		COND_SHOULD_DODGE_ROCKET"
			"		COND_NEW_ENEMY"
			"		COND_ENEMY_DEAD"
			"		COND_LOST_ENEMY"
	)
	DEFINE_SCHEDULE
	(
		SCHED_VORTBOSS_DODGE_ROCKET,

		"	Tasks"
		"		TASK_VORTBOSS_DODGEROCKET			0"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_LOST_ENEMY"
	)
	DEFINE_SCHEDULE
	(
	SCHED_VORTBOSS_BARAGE,

		"	Tasks"
		"		TASK_FACE_ENEMY						0"
		"		TASK_VORTBOSS_BARAGE			0"
		"		TASK_VORTBOSS_DODGEROCKET			0"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_LOST_ENEMY"
	)
	DEFINE_SCHEDULE
	(
	SCHED_VORTBOSS_SWING,
		"	Tasks"
		"		TASK_FACE_ENEMY						0"
		"		TASK_VORTBOSS_SWING			0"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_LOST_ENEMY"
	)
	DEFINE_SCHEDULE
	(
			SCHED_VORTBOSS_CANNON,

			"	Tasks"
			"		TASK_FACE_ENEMY						0"
			"		TASK_VORTBOSS_CANNON				0"
			"		TASK_VORTBOSS_DODGEROCKET			0"
			//"		TASK_MOVE_AWAY_FROM_ENEMY			0"
			"	"
			"	Interrupts"
			"		COND_NEW_ENEMY"
			"		COND_ENEMY_DEAD"
			"		COND_LOST_ENEMY"
			"		COND_CAN_DO_GROUND_ATTACK"
			"		COND_SHOULD_DODGE_ROCKET"
			"		COND_SHOULD_SWING"
	)
	DEFINE_SCHEDULE
	(
		SCHED_VORTBOSS_SPINATTACK,

			"	Tasks"
			"		TASK_FACE_ENEMY						0"
			"		TASK_VORTBOSS_SPINATTACK			0"
	//"		TASK_MOVE_AWAY_FROM_ENEMY			0"
			"	"
			"	Interrupts"
			"		COND_NEW_ENEMY"
			"		COND_ENEMY_DEAD"
			"		COND_LOST_ENEMY"

	//"		COND_TOO_CLOSE_TO_ATTACK"
	)
	DEFINE_SCHEDULE
	(
			SCHED_VORTBOSS_EYEBLAST,

			"	Tasks"
			"		TASK_STOP_MOVING					0"
			"		TASK_FACE_ENEMY						0"
			"		TASK_VORTBOSS_EYEBLAST				0"
			"	"
			"	Interrupts"
			"		COND_NEW_ENEMY"
			"		COND_ENEMY_DEAD"
			"		COND_LOST_ENEMY"
	)
	DEFINE_SCHEDULE
	(
			SCHED_VORTBOSS_BACKOFF,

			"	Tasks"
			"		TASK_MOVE_AWAY_PATH						120"
			"		TASK_RUN_PATH							0"
			"		TASK_WAIT_FOR_MOVEMENT					0"
			"		TASK_SET_SCHEDULE						SCHEDULE:SCHED_MOVE_AWAY_END"
			"	"
			"	Interrupts"
			"		COND_NEW_ENEMY"
			"		COND_ENEMY_DEAD"
			"		COND_LOST_ENEMY"
	)
	DEFINE_SCHEDULE
	(
		SCHED_VORTBOSS_CHARGE,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_ENEMY						0"
		"		TASK_VORTBOSS_CHARGE				0"
		"	"
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_ENEMY_DEAD"
	)
	DEFINE_SCHEDULE
	(
		SCHED_VORTBOSS_ESTABLISH_LOF,
		"	Tasks "
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK"
		"		TASK_GET_PATH_TO_ENEMY_LOS		0"
		"		TASK_SPEAK_SENTENCE				1"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		""
		"	Interrupts "
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_LOST_ENEMY"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_HEAR_DANGER"
		"		COND_SEE_ENEMY"
	)
AI_END_CUSTOM_NPC()