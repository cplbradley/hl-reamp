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

enum SquadSlot_t
{
	SQUAD_SLOT_VORTBOSS_SPINBEAM = LAST_SHARED_SQUADSLOT,
};
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

ConVar sk_vortboss_health("sk_vortboss_health", "2500");
ConVar sk_vortboss_projectile_speed("sk_vortboss_projectile_speed", "2300");
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
	void	CheckSpinBeam(void);

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
	//void	Event_Killed(const CTakeDamageInfo &info);

	Vector m_vTargetPos;
	Vector m_vStartPos;
	float m_fNextEyeBlast;
	float m_fNextCannonAttack;
	float m_fNextSpinAttack;
	
	static int gm_nBodyPitchPoseParam;

	float m_fNextTrace;

	int iAttackBeamWidth;

	void SpinAttackThink(void);

protected:
	void SetupGlobalModelData();
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(npc_vortboss, CNPC_VortBoss);

IMPLEMENT_SERVERCLASS_ST(CNPC_VortBoss,DT_VortBoss)
	SendPropBool(SENDINFO(bDrawSpinBeam)),
END_SEND_TABLE()


int CNPC_VortBoss::gm_nBodyPitchPoseParam = -1;
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
	SetupGlobalModelData();
	//CapabilitiesClear();
	//CapabilitiesAdd( bits_CAP_NONE );
	m_fNextTrace = gpGlobals->curtime;
	NPCInit();
	m_fNextCannonAttack = gpGlobals->curtime;
	m_fNextEyeBlast = gpGlobals->curtime;
	m_fNextSpinAttack = gpGlobals->curtime;
}

void CNPC_VortBoss::NPCThink(void) //constant thinking 
{
	BaseClass::NPCThink();
	//UpdateAim();
	CheckSpinBeam();
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
	float postAdjustedTimeDelta = g_pGameRules->AdjustProjectileSpeed(preAdjustedTimeDelta); //adjust based on difficulty
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
	float postAdjustedTimeDelta = g_pGameRules->AdjustProjectileSpeed(preAdjustedTimeDelta); //adjust based on difficulty level
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
		Vector enemyDelta = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter(); //get the distance from me to my enemy
		float flDist = enemyDelta.Length();
		if (flDist < 256.0f) //if my enemy is within pounding range 
		{
			SetCondition((COND_CAN_DO_GROUND_ATTACK)); //ground pound baby 
		}
		if (flDist < 400.0f && flDist > 256.0f) //if the enemy is too close to shoot and too far to ground pound
		{
			SetCondition((COND_TOO_CLOSE_TO_ATTACK)); //tell me that so i can move away
			//ClearCondition((COND_CAN_DO_GROUND_ATTACK));
		}
		if (flDist > 400.0f && flDist < 1024 && HasCondition(COND_HAVE_ENEMY_LOS)) //if my enemy is within the acceptable attack range and i can see them
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
			if (gpGlobals->curtime > m_fNextEyeBlast) //if my eyeblast cooldown has passed
				SetCondition(COND_CAN_DO_EYEBLAST); //tell me that so i can eyeblast. eyeblast takes precidence over cannon.
			if (gpGlobals->curtime > m_fNextCannonAttack) //if my cannon cooldown has passed
				SetCondition(COND_CAN_FIRE_CANNON); //tell me that so i can shoot the cannon
		}
		if (flDist > 1024.0f) //if they're too far away
			SetCondition(COND_TOO_FAR_TO_ATTACK); //tell me that so i can get closer
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
	if (HasCondition((COND_CAN_DO_GROUND_ATTACK))) //if i can ground attack
		return SCHED_VORTBOSS_GROUND_ATTACK; //do that
	if (HasCondition((COND_TOO_FAR_TO_ATTACK))) //if the enemy is too far
		return SCHED_CHASE_ENEMY; //get closer
	if (HasCondition((COND_TOO_CLOSE_TO_ATTACK))) //if the enemy is too close but not close enough to ground pound
		return SCHED_MOVE_AWAY;  //move further away

	if (HasCondition(COND_CAN_SPIN_ATTACK)) //if i can spin
	{
		ClearCondition(COND_CAN_SPIN_ATTACK);
		m_fNextSpinAttack = gpGlobals->curtime + 45.0f; //set cooldown
		return SCHED_VORTBOSS_SPINATTACK; //speen
	}

	if (HasCondition(COND_CAN_DO_EYEBLAST)) //if i can eyeblast
	{
		ClearCondition(COND_CAN_DO_EYEBLAST);
		m_fNextEyeBlast = gpGlobals->curtime + 15.0f; //set cooldown
		return SCHED_VORTBOSS_EYEBLAST; //blast away
	}
	if (HasCondition(COND_CAN_FIRE_CANNON)) //if i can shoot my cannon
	{
		ClearCondition(COND_CAN_FIRE_CANNON);
		m_fNextCannonAttack = gpGlobals->curtime + SequenceDuration(); //set cooldown
		return SCHED_VORTBOSS_CANNON; //pew pew pew
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
			SetPlaybackRate(1.4f);
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

	DECLARE_SQUADSLOT(SQUAD_SLOT_VORTBOSS_SPINBEAM)

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