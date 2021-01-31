//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
// This is a skeleton file for use when creating a new 
// NPC. Copy and rename this file for the new
// NPC and add the copy to the build.
//
// Leave this file in the build until we ship! Allowing 
// this file to be rebuilt with the rest of the game ensures
// that it stays up to date with the rest of the NPC code.
//
// Replace occurances of CNewNPC with the new NPC's
// classname. Don't forget the lower-case occurance in 
// LINK_ENTITY_TO_CLASS()
//
//
// ASSUMPTIONS MADE:
//
// You're making a character based on CAI_BaseNPC. If this 
// is not true, make sure you replace all occurances
// of 'CAI_BaseNPC' in this file with the appropriate 
// parent class.
//
// You're making a human-sized NPC that walks.
//
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
#include "hlr/hlr_projectile.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "particle_parse.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// Custom schedules
//=========================================================
enum
{
	SCHED_VORTBOSS_CANNON = LAST_SHARED_SCHEDULE,
	SCHED_VORTBOSS_EYEBLAST,
	SCHED_VORTBOSS_CHASEENEMY,
	SCHED_VORTBOSS_GROUND_ATTACK,
	SCHED_VORTBOSS_SPINATTACK,
	SCHED_VORTBOSS_BACKOFF
};

//=========================================================
// Custom tasks
//=========================================================
enum
{
	TASK_VORTBOSS_CANNON = LAST_SHARED_TASK,
	TASK_VORTBOSS_EYEBLAST,
	TASK_VORTBOSS_GROUNDATTACK,
	TASK_VORTBOSS_SPINATTACK
};


//=========================================================
// Custom Conditions
//=========================================================
enum
{
	COND_CAN_FIRE_CANNON = LAST_SHARED_CONDITION,
	COND_CAN_DO_EYEBLAST,
	COND_CAN_DO_GROUND_ATTACK,
	COND_CAN_SPIN_ATTACK
};

ConVar sk_vortboss_health("sk_vortboss_health", "250");
ConVar sk_vortboss_projectile_speed("sk_vortboss_projectile_speed", "1700");
ConVar g_debug_vortboss_spinattack("g_debug_vortboss_spinattack", "0");
Activity ACT_VORTBOSS_EYEBLAST;
Activity ACT_VORTBOSS_CANNON;
Activity ACT_VORTBOSS_GROUND_ATTACK;
Activity ACT_VORTBOSS_SPINATTACK;

int AE_VORTBOSS_CHARGECANNON;
int AE_VORTBOSS_FIRECANNON;
int AE_VORTBOSS_START_TARGETBEAM;
int AE_VORTBOSS_FIRE_ATTACKBEAM;
int AE_VORTBOSS_GROUNDATTACK;
int AE_VORTBOSS_CLEAR_BEAM;
int AE_VORTBOSS_START_SPINATTACK;
int AE_VORTBOSS_END_SPINATTACK;

#define VORTBOSS_EYE_ATTACHMENT 1
#define VORTBOSS_HAND_ATTACHMENT 2
#define VORTBOSS_MAX_LASER_RANGE 3600
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

	//CBeam *pSpinBeam;

	/*
	int RangeAttack1Conditions(float flDot, float flDist);
	int RangeAttack2Conditions(float flDot, float flDist);

	void	FireCannonProjectile(void);

	void	GroundAttack(void);*/

	CBeam *pEyeTargetBeam;
	CBeam *pEyeAttackBeam;


	void	CreateTargetBeam(void);
	void	EyeBeamThink(void);
	void	CreateAttackBeam(void);
	void	AttackBeamThink(void);
	void	ClearBeam(void);

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
	
	virtual int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	CNetworkVar(bool,bDrawSpinBeam);
	CNetworkVar(int, iAmmoType);
	//void	Event_Killed(const CTakeDamageInfo &info);

	Vector m_vTargetPos;
	Vector m_vStartPos;
	float m_fNextEyeBlast;
	float m_fNextCannonAttack;
	float m_fNextSpinAttack;
	
	float m_fNextTrace;

	int iAttackBeamWidth;

	void SpinAttackThink(void);

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(npc_vortboss, CNPC_VortBoss);

IMPLEMENT_SERVERCLASS_ST(CNPC_VortBoss,DT_VortBoss)
	SendPropBool(SENDINFO(bDrawSpinBeam)),
	SendPropInt(SENDINFO(iAmmoType)),
END_SEND_TABLE()

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_VortBoss)


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
	PrecacheModel("models/vortboss_test.mdl");
	UTIL_PrecacheOther("hlr_vortprojectile");
	PrecacheMaterial("sprites/laser.vmt");
	PrecacheParticleSystem("vortboss_cannon_core");
	PrecacheParticleSystem("vortboss_ground_attack");
	PrecacheScriptSound("BadDog.Smash");
	PrecacheScriptSound("VortBoss.EyeBlast");
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

	SetModel("models/vortboss_test.mdl");
	SetHullType(HULL_LARGE);

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_RED);
	SetNavType(NAV_GROUND);
	
	bDrawSpinBeam = false;

	RegisterThinkContext("EyeUpdateContext");
	RegisterThinkContext("EyeAttackContext");

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	CapabilitiesAdd(bits_CAP_MOVE_JUMP);
//	CapabilitiesAdd(bits_CAP_SKIP_NAV_GROUND_CHECK);
	CapabilitiesAdd(bits_CAP_MOVE_SHOOT);
	//CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK2 | bits_CAP_INNATE_MELEE_ATTACK1);
	m_iHealth = sk_vortboss_health.GetFloat();
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;
	SetEfficiency(AIE_HYPER);

	iAmmoType = GetAmmoDef()->Index("SMG1");
	//CapabilitiesClear();
	//CapabilitiesAdd( bits_CAP_NONE );
	m_fNextTrace = gpGlobals->curtime;
	NPCInit();
	m_fNextCannonAttack = gpGlobals->curtime;
	m_fNextEyeBlast = gpGlobals->curtime;
	m_fNextSpinAttack = gpGlobals->curtime;
}

void CNPC_VortBoss::NPCThink(void)
{
	BaseClass::NPCThink();
	if (bDrawSpinBeam)
	{
			SetEfficiency(AIE_HYPER);
			Vector vecHandPos, m_vLaserDir;
			QAngle angHandAng;
			GetAttachment(VORTBOSS_HAND_ATTACHMENT, vecHandPos, angHandAng);
			AngleVectors(angHandAng, &m_vLaserDir);
			trace_t tr;
			m_vLaserDir[ROLL] = 0;
			for (int i = 0; i < 3; i++)
			{

				if (i == 0)
				{

					AngleVectors(angHandAng, &m_vLaserDir);
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
						/*ClearMultiDamage();
						CTakeDamageInfo info(this, this, 20, DMG_SHOCK);
						info.AdjustPlayerDamageTakenForSkillLevel();
						CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
						pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
						ApplyMultiDamage();*/
						//DevMsg("applying damage\n");

					}
				}
				if (i == 1)
				{
					angHandAng[1] += 1;
					AngleVectors(angHandAng, &m_vLaserDir);
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
						/*ClearMultiDamage();
						CTakeDamageInfo info(this, this, 20, DMG_SHOCK);
						info.AdjustPlayerDamageTakenForSkillLevel();
						CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
						pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
						ApplyMultiDamage();*/
						//DevMsg("applying damage\n");

					}
				}
				if (i == 2)
				{
					angHandAng[1] -= 2;
					AngleVectors(angHandAng, &m_vLaserDir);
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
						/*ClearMultiDamage();
						CTakeDamageInfo info(this, this, 20, DMG_SHOCK);
						info.AdjustPlayerDamageTakenForSkillLevel();
						CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
						pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
						ApplyMultiDamage();*/
						//DevMsg("applying damage\n");

					}
				}
			}
			//ITraceFilter *pFilter = GetBeamTraceFilter();
			//AI_TraceHull(tr.startpos, tr.endpos, GetHullMins(), GetHullMaxs(), MASK_SHOT, COLLISION_GROUP_NONE, &tr);
			/*AI_TraceHull(tr.startpos, tr.endpos, GetHullMins(), GetHullMaxs(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
			DebugDrawLine(tr.startpos, tr.endpos, 255, 0, 0, false, 0.5);*/
			
	}
	UpdateAim();
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
		(eActivity != ACT_VORTBOSS_SPINATTACK))
	{
		Vector vecShootOrigin;

		vecShootOrigin = WorldSpaceCenter();
		Vector vecShootDir = GetShootEnemyDir(vecShootOrigin, false);

		SetAim(vecShootDir, flInterval);
	}
	/*else
	{
		RelaxAim(flInterval);
	}*/
}
void CNPC_VortBoss::SetAim(const Vector &aimDir, float flInterval)
{
	QAngle angDir;
	VectorAngles(aimDir, angDir);

	float newPitch = angDir[PITCH];
	float newYaw = angDir[YAW];

	/*QAngle curAngle = GetLocalAngles();
	int curPitch = curAngle[PITCH];
	int curYaw = curAngle[YAW];
	float newPitch;
	float newYaw;

	if (GetEnemy())
	{
		// clamp and dampen movement
		newPitch = curAngle[PITCH] + 0.8 * UTIL_AngleDiff(UTIL_ApproachAngle(angDir.x, curPitch, 20), curPitch);

		float flRelativeYaw = UTIL_AngleDiff(angDir.y, GetAbsAngles().y);
		newYaw = curYaw + UTIL_AngleDiff(flRelativeYaw, curYaw);
	}
	else
	{
		// Sweep your weapon more slowly if you're not fighting someone
		newPitch = curPitch + 0.6 * UTIL_AngleDiff(UTIL_ApproachAngle(angDir.x, curPitch, 20), curPitch);

		float flRelativeYaw = UTIL_AngleDiff(angDir.y, GetAbsAngles().y);
		newYaw = curYaw + 0.6 * UTIL_AngleDiff(flRelativeYaw, curYaw);
	}

	newPitch = AngleNormalize(newPitch);
	newYaw = AngleNormalize(newYaw);
	*/
	QAngle NewAngle = QAngle(newPitch, newYaw, 0);
	SetLocalAngles(NewAngle);
	//Msg( "pitch=%f, yaw=%f\n", newPitch, newYaw );
}
void CNPC_VortBoss::CreateTargetBeam(void)
{
	QAngle angHandAng;
	float timeDelta;
	Vector vecEyePos,m_vLaserDir;

	GetAttachment(VORTBOSS_EYE_ATTACHMENT, vecEyePos, angHandAng);
	float preAdjustedTimeDelta = 1;
	float postAdjustedTimeDelta = g_pGameRules->AdjustProjectileSpeed(preAdjustedTimeDelta);
	timeDelta = 1 / postAdjustedTimeDelta;

	UTIL_PredictedPosition(GetEnemy(), timeDelta, &m_vTargetPos);
	Vector vecAdjustedTargetPos = m_vTargetPos + Vector(0, 0, 32);
	m_vLaserDir = vecAdjustedTargetPos - vecEyePos;
	VectorNormalize(m_vLaserDir);
	trace_t tr;
	UTIL_TraceLine(vecEyePos, vecEyePos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	
	pEyeTargetBeam = CBeam::BeamCreate("sprites/laser.vmt", 2);
	pEyeTargetBeam->PointEntInit(tr.endpos, this);
	pEyeTargetBeam->SetEndAttachment(VORTBOSS_EYE_ATTACHMENT);
	pEyeTargetBeam->SetBrightness(255);
	pEyeTargetBeam->SetRenderColor(200, 0, 50);
	pEyeTargetBeam->SetNoise(0);
	SetContextThink(&CNPC_VortBoss::EyeBeamThink, gpGlobals->curtime, "EyeUpdateContext");
}
void CNPC_VortBoss::EyeBeamThink(void)
{
	QAngle angHandAng;
	float timeDelta;
	Vector vecEyePos, m_vLaserDir;

	GetAttachment(VORTBOSS_EYE_ATTACHMENT, vecEyePos, angHandAng);
	float preAdjustedTimeDelta = 0.25;
	float postAdjustedTimeDelta = g_pGameRules->AdjustProjectileSpeed(preAdjustedTimeDelta);
	timeDelta = 0.5 - postAdjustedTimeDelta;

	UTIL_PredictedPosition(GetEnemy(), timeDelta, &m_vTargetPos);
	Vector vecAdjustedTargetPos = m_vTargetPos + Vector(0, 0, 32);
	m_vLaserDir = vecAdjustedTargetPos - vecEyePos;
	VectorNormalize(m_vLaserDir);
	trace_t tr;
	UTIL_TraceLine(vecEyePos, vecEyePos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	pEyeTargetBeam->SetStartPos(tr.endpos);
	pEyeTargetBeam->RelinkBeam();
	SetNextThink(gpGlobals->curtime + 0.001f, "EyeUpdateContext");

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
void CNPC_VortBoss::HandleAnimEvent(animevent_t *pEvent)
{
	if (pEvent->event == AE_VORTBOSS_FIRECANNON)
	{
		int nAttachment = LookupAttachment("cannonmuzzle");
		Vector vecHandPos;
		QAngle vecHandAng;
		GetAttachment(nAttachment, vecHandPos, vecHandAng);
		CHLRVortProjectile *pVort = (CHLRVortProjectile*)CreateEntityByName("hlr_vortprojectile");
		UTIL_SetOrigin(pVort, vecHandPos);
		pVort->Spawn();

		Vector enemyDelta = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		float flDist = enemyDelta.Length();
		float fBaseSpeed = sk_vortboss_projectile_speed.GetFloat();
		float fAdjustedSpeed = g_pGameRules->AdjustProjectileSpeed(fBaseSpeed);
		float timeDelta = flDist / fAdjustedSpeed;

		Vector vecTargetPos;
		Vector vecTargetCenter;
		UTIL_PredictedPosition(GetEnemy(), timeDelta, &vecTargetPos);
		if (GetEnemy()->GetGroundEntity() != NULL)
		{
			vecTargetCenter = vecTargetPos + Vector(0, 0, 32);
		}
		else
		{
			vecTargetCenter = vecTargetPos;
		}
		Vector vecAim = vecTargetCenter - vecHandPos;
		VectorNormalize(vecAim);

		pVort->SetAbsVelocity(vecAim * fAdjustedSpeed);
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
	}
	if (pEvent->event == AE_VORTBOSS_END_SPINATTACK)
	{
		bDrawSpinBeam = false;
		ClearSpinBeam();
	}
	if (pEvent->event == AE_VORTBOSS_GROUNDATTACK)
	{
		ClearBeam();
		RadiusDamage(CTakeDamageInfo(this, this, 50, DMG_SONIC), GetAbsOrigin(), 300, CLASS_VORTIGAUNT, NULL);
		DispatchParticleEffect("vortboss_ground_attack", GetAbsOrigin(), GetAbsAngles(), this);
		EmitSound("BadDog.Smash");
		UTIL_ScreenShake(GetAbsOrigin(), 40.0, 60, 1.0, 500, SHAKE_START);
	}
	if (pEvent->event == AE_VORTBOSS_START_TARGETBEAM)
	{
		CreateTargetBeam();
	}
	if (pEvent->event == AE_VORTBOSS_FIRE_ATTACKBEAM)
	{
		EmitSound("VortBoss.EyeBlast");
		CreateAttackBeam();
	}
	if (pEvent->event == AE_VORTBOSS_CLEAR_BEAM)
	{
		ClearBeam();
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
	return BaseClass::OnTakeDamage_Alive(dInfo);
}
void CNPC_VortBoss::SpinAttackThink(void)
{
	/*int nAttachment = LookupAttachment("cannonmuzzle");
	Vector vecHandPos;
	QAngle vecHandAng;
	Vector vecAiming;
	//float fNextShot;
	float fBaseSpeed = sk_vortboss_projectile_speed.GetFloat();
	float fAdjustedSpeed = g_pGameRules->AdjustProjectileSpeed(fBaseSpeed);
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
	BaseClass::GatherConditions();
	if (GetEnemy())
	{
		Vector enemyDelta = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		float flDist = enemyDelta.Length();
		if (flDist < 256.0f)
		{
			SetCondition((COND_CAN_DO_GROUND_ATTACK));
		}
		if (flDist < 400.0f && flDist > 256.0f)
		{
			SetCondition((COND_TOO_CLOSE_TO_ATTACK));
			//ClearCondition((COND_CAN_DO_GROUND_ATTACK));
		}
		if (flDist > 400.0f && flDist < 1024)
		{
			if (gpGlobals->curtime > m_fNextEyeBlast)
				SetCondition(COND_CAN_DO_EYEBLAST);
			if (gpGlobals->curtime > m_fNextCannonAttack)
				SetCondition(COND_CAN_FIRE_CANNON);
			if (gpGlobals->curtime > m_fNextSpinAttack)
				SetCondition(COND_CAN_SPIN_ATTACK);
		}
		if (flDist > 1024.0f)
			SetCondition(COND_TOO_FAR_TO_ATTACK);
	}
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
int CNPC_VortBoss::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_COMBAT && GetEnemy())
		return SelectCombatSchedule();
	return BaseClass::SelectSchedule();
}
int CNPC_VortBoss::SelectCombatSchedule(void)
{
	if (HasCondition((COND_CAN_DO_GROUND_ATTACK)))
		return SCHED_VORTBOSS_GROUND_ATTACK;
	if (HasCondition((COND_TOO_FAR_TO_ATTACK)))
		return SCHED_CHASE_ENEMY;
	if (HasCondition((COND_TOO_CLOSE_TO_ATTACK)))
		return SCHED_RUN_FROM_ENEMY;

	if (HasCondition(COND_CAN_SPIN_ATTACK))
	{
		ClearCondition(COND_CAN_SPIN_ATTACK);
		m_fNextSpinAttack = gpGlobals->curtime + 45.0f;
		return SCHED_VORTBOSS_SPINATTACK;
	}

	if (HasCondition(COND_CAN_FIRE_CANNON))
	{
		ClearCondition(COND_CAN_FIRE_CANNON);
		m_fNextCannonAttack = gpGlobals->curtime + 5.0f;
		return SCHED_VORTBOSS_CANNON;
	}

	if (HasCondition(COND_CAN_DO_EYEBLAST))
	{
		ClearCondition(COND_CAN_DO_EYEBLAST);
		m_fNextEyeBlast = gpGlobals->curtime + 15.0f;
		return SCHED_VORTBOSS_EYEBLAST;
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

	case TASK_VORTBOSS_EYEBLAST:
	{
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
		if (IsActivityFinished())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_VORTBOSS_EYEBLAST:
	{
		if (IsActivityFinished())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_VORTBOSS_SPINATTACK:
	{
		if (IsActivityFinished())
		{
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
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}


AI_BEGIN_CUSTOM_NPC(npc_vortboss, CNPC_VortBoss)

	DECLARE_ACTIVITY(ACT_VORTBOSS_CANNON)
	DECLARE_ACTIVITY(ACT_VORTBOSS_EYEBLAST)
	DECLARE_ACTIVITY(ACT_VORTBOSS_GROUND_ATTACK)
	DECLARE_ACTIVITY(ACT_VORTBOSS_SPINATTACK)

	DECLARE_ANIMEVENT(AE_VORTBOSS_FIRECANNON)
	DECLARE_ANIMEVENT(AE_VORTBOSS_CHARGECANNON)
	DECLARE_ANIMEVENT(AE_VORTBOSS_FIRE_ATTACKBEAM)
	DECLARE_ANIMEVENT(AE_VORTBOSS_START_TARGETBEAM)
	DECLARE_ANIMEVENT(AE_VORTBOSS_CLEAR_BEAM)
	DECLARE_ANIMEVENT(AE_VORTBOSS_GROUNDATTACK)
	DECLARE_ANIMEVENT(AE_VORTBOSS_START_SPINATTACK)
	DECLARE_ANIMEVENT(AE_VORTBOSS_END_SPINATTACK)

	DECLARE_CONDITION(COND_CAN_DO_EYEBLAST)
	DECLARE_CONDITION(COND_CAN_DO_GROUND_ATTACK)
	DECLARE_CONDITION(COND_CAN_FIRE_CANNON)
	DECLARE_CONDITION(COND_CAN_SPIN_ATTACK)

	DECLARE_TASK(TASK_VORTBOSS_CANNON)
	DECLARE_TASK(TASK_VORTBOSS_EYEBLAST)
	DECLARE_TASK(TASK_VORTBOSS_GROUNDATTACK)
	DECLARE_TASK(TASK_VORTBOSS_SPINATTACK)
	DEFINE_SCHEDULE
	(
			SCHED_VORTBOSS_GROUND_ATTACK,

			"	Tasks"
			"		TASK_STOP_MOVING					0"
			"		TASK_FACE_ENEMY						0"
			"		TASK_VORTBOSS_GROUNDATTACK			0"
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
			//"		TASK_MOVE_AWAY_FROM_ENEMY			0"
			"	"
			"	Interrupts"
			"		COND_NEW_ENEMY"
			"		COND_ENEMY_DEAD"
			"		COND_LOST_ENEMY"
			"		COND_CAN_DO_GROUND_ATTACK"
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
			"		COND_CAN_DO_GROUND_ATTACK"
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
AI_END_CUSTOM_NPC()