//=======================================================//
//================== VORT BRUTE =========================//
//=======================================================//
//======== CREATED BY JUST WAX @ VELOCITY PUNCH =========//
//=======================================================//

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
#include "npc_vortbrute.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define VORTBRUTE_MODEL "models/jackedvort.mdl"
#define PROJECTILE_MODEL "models/spitball_large.mdl"

enum //Schedules
{
	SCHED_VORTBRUTE_LEAPATTACK = LAST_SHARED_SCHEDULE,
	SCHED_VORTBRUTE_MELEE,
	SCHED_VORTBRUTE_RANGED_ATTACK,
	SCHED_VORTBRUTE_CHASE_ENEMY,
	SCHED_VORTBRUTE_RAGESLAM,
};

enum //Tasks
{
	TASK_VORTBRUTE_LEAPATTACK = LAST_SHARED_TASK,
	TASK_VORTBRUTE_RIGHTHOOK,
	TASK_VORTBRUTE_LEFTHOOK,
	TASK_VORTBRUTE_RIGHTTHROW,
	TASK_VORTBRUTE_LEFTTHROW,
	TASK_VORTBRUTE_FACE_ENEMY_LEAD,
	TASK_VORTBRUTE_RAGESLAM,
};

enum
{
	COND_IN_RANGE_TO_PUNCH = LAST_SHARED_CONDITION,
	COND_OUT_OF_PUNCH_RANGE,
	COND_IN_RANGE_TO_LEAP,
	COND_IN_RANGE_TO_THROW,
};

Activity ACT_VORTBRUTE_LEAPATTACK;
Activity ACT_VORTBRUTE_RIGHTHOOK;
Activity ACT_VORTBRUTE_LEFTHOOK;
Activity ACT_VORTBRUTE_RIGHTTHROW;
Activity ACT_VORTBRUTE_LEFTTHROW;
Activity ACT_VORTBRUTE_RUN;
Activity ACT_VORTBRUTE_RAGESLAM;

int nRingTexture = -1;

int AE_VORTBRUTE_RIGHTHOOK_CONNECT;
int AE_VORTBRUTE_LEFTHOOK_CONNECT;
int AE_VORTBRUTE_RIGHTTHROW_THROW;
int AE_VORTBRUTE_LEFTTHROW_THROW;
int AE_VORTBRUTE_LEAPATTACK_TELEPORT;
int AE_VORTBRUTE_LEAPATTACK_LAND;
int AE_VORTBRUTE_SWING_SOUND;
int AE_VORTBRUTE_RAGESLAM_IMPACT;

ConVar sk_vortbrute_health("sk_vortbrute_health", "0");
ConVar ai_vortbrute_legacy_face("ai_vortbrute_legacy_face", "0");
ConVar sk_vortbrute_projectile_speed("sk_vortbrute_projectile_speed", "0");
ConVar sk_vortbrute_leap_frequency("sk_vortbrute_leap_frequency", "10");
ConVar sk_vortbrute_throw_frequency("sk_vortbrute_throw_frequency", "5");
ConVar sk_vortbrute_projectile_dmg("sk_vortbrute_projectile_dmg", "0");


LINK_ENTITY_TO_CLASS(brute_projectile, CBruteProjectile);
BEGIN_DATADESC(CBruteProjectile)
DEFINE_FUNCTION(Touch),
END_DATADESC()

CBruteProjectile* CBruteProjectile::Create(const Vector& vecOrigin, const QAngle& angAngle, CBaseEntity* pOwner)
{
	CBruteProjectile* pProj = (CBruteProjectile*)CreateEntityByName("brute_projectile");
	UTIL_SetOrigin(pProj, vecOrigin);
	pProj->SetAbsAngles(angAngle);
	pProj->SetOwnerEntity(pOwner);
	pProj->Spawn();

	return pProj;
}

void CBruteProjectile::Spawn()
{
	Precache();
	SetModel(PROJECTILE_MODEL);
	SetTouch(&CBruteProjectile::Touch);
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	SetGravity(1.0f);
	CreateParticles();
}

void CBruteProjectile::Precache()
{
	PrecacheModel(PROJECTILE_MODEL);
	PrecacheParticleSystem("vortbrute_handflames");
}

void CBruteProjectile::CreateParticles()
{
	DispatchParticleEffect("vortbrute_handflames", PATTACH_ABSORIGIN_FOLLOW,this,"root");
}


void CBruteProjectile::Touch(CBaseEntity* pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
			return;
	}

	if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
	{
		trace_t	tr; //initialize info
		tr = BaseClass::GetTouchTrace(); //trace touch
		Vector	vecNormalizedVel = GetAbsVelocity(); //this is how fast i'm going!

		ClearMultiDamage(); //gotta reset everything before i zap ya
		VectorNormalize(vecNormalizedVel); //convert the vector into a direction
		CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_vortbrute_projectile_dmg.GetFloat(), DMG_SHOCK); //i'm setting up some damage info
		dmgInfo.AdjustPlayerDamageTakenForSkillLevel(); //how fair do i feel?
		dmgInfo.SetDamagePosition(tr.endpos); //gotta attack at the exact point of impact
		pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr); //you're getting zapped!
		//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this);
		ApplyMultiDamage(); //zap!
	}

	if (!pOther->IsPlayer() && GetOwnerEntity())
	{
		CNPC_VortBrute* brute = dynamic_cast<CNPC_VortBrute*>(GetOwnerEntity());
		if(brute)
			brute->m_iMissCount++;
	}
	SetMoveType(MOVETYPE_NONE); //stop moving completely
	SetSolid(SOLID_NONE); //get rid of my collision model
	SetSolidFlags(FSOLID_NOT_SOLID); //alert the game i'm not solid anymore
	RemoveDeferred();
	SetTouch(NULL); //i can't touch anything anymore
}


LINK_ENTITY_TO_CLASS(npc_vortbrute, CNPC_VortBrute);


BEGIN_DATADESC(CNPC_VortBrute)

DEFINE_INPUTFUNC(FIELD_VOID,"TestTeleport", TestTeleport),
DEFINE_FIELD(flNextLeap,FIELD_FLOAT),
DEFINE_FIELD(flNextThrow,FIELD_FLOAT),
DEFINE_FIELD(m_iMissCount,FIELD_INTEGER),

END_DATADESC()

void CNPC_VortBrute::Spawn()
{
	Precache();
	SetMaxHealth(sk_vortbrute_health.GetInt());
	SetHealth(GetMaxHealth());
	SetMoveType(MOVETYPE_STEP);
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_MOVE_JUMP | bits_CAP_SKIP_NAV_GROUND_CHECK);
	SetEnemyClass(ENEMYCLASS_HEAVY);	
	SetMovementClass(MOVECLASS_ALWAYSJUMP);
	SetNavType(NAV_GROUND);
	SetHullType(HULL_MEDIUM_TALL);
	m_NPCState = NPC_STATE_NONE;
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);

	SetBloodColor(BLOOD_COLOR_GREEN);
	NPCInit();
	flNextLeap = flNextThrow = gpGlobals->curtime;

	m_iMissCount = 0;

	m_flFieldOfView = -0.9f;
}

void CNPC_VortBrute::PostNPCInit()
{
	BaseClass::PostNPCInit();
	int iLeft = LookupAttachment("leftclaw");
	int iRight = LookupAttachment("rightclaw");
	DispatchParticleEffect("vortbrute_handflames", PATTACH_POINT_FOLLOW, this, iLeft);
	DispatchParticleEffect("vortbrute_handflames", PATTACH_POINT_FOLLOW, this, iRight);
}
void CNPC_VortBrute::Precache()
{
	PrecacheModel(VORTBRUTE_MODEL);
	SetModel(VORTBRUTE_MODEL);
	PrecacheParticleSystem("vortigaunt_teleout");
	PrecacheParticleSystem("vortigaunt_telein");
	PrecacheParticleSystem("vortbrute_handflames");
	PrecacheParticleSystem("vortbrute_groundslam");
	PrecacheScriptSound("NPC_Vortigaunt.Swing");
	PrecacheScriptSound("Punch.Impact");
	PrecacheScriptSound("Vortbrute.Teleport");
	nRingTexture = PrecacheModel("sprites/lgtning.vmt");
	PrecacheScriptSound("BadDog.Smash");
}
float CNPC_VortBrute::GetEnemyDistance()
{
	if (GetEnemy())
		return (GetAbsOrigin() - GetEnemy()->GetAbsOrigin()).Length();

	return 0.0f;
}
float CNPC_VortBrute::MaxYawSpeed(void)
{
	return 80.0f;
}
Vector CNPC_VortBrute::GetLeapDestination()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer(); //Get the player

	if (!pPlayer) //Verify the player. If player is null, return 0,0,0
		return vec3_origin;

	if (GetEnemyDistance() < 300.0f)
	{
		DevWarning(2, "Player is too close to justify a teleport leap. Teleport blocked...\n");
		return vec3_origin;
	}

	QAngle angAbsAng = pPlayer->GetAbsAngles(); //Get player yaw
	QAngle angViewAng = pPlayer->EyeAngles(); //Get player viewangles
	angViewAng[PITCH] = 0.0f;
	Vector vecAbsAng, vecViewAng;
	AngleVectors(angAbsAng, &vecAbsAng);
	AngleVectors(angViewAng, &vecViewAng);
	VectorNormalize(vecAbsAng);
	VectorNormalize(vecViewAng);
	Vector vecStart = pPlayer->GetAbsOrigin() + (vecAbsAng * 90.0f); //Store the ideal teleport destination

	trace_t tr_down, tr_forward;
	UTIL_TraceLine(vecStart, vecStart + Vector(0, 0, -32), MASK_PLAYERSOLID, this, COLLISION_GROUP_NPC, &tr_down); //check the grown below the destination
	UTIL_TraceLine(pPlayer->WorldSpaceCenter(), pPlayer->WorldSpaceCenter() + (vecViewAng * 150.0f), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER, &tr_forward); //check in front of the player

	if (tr_down.fraction == 1.0) //If the downard trace doesn't hit anything, return 0,0,0
	{
		DevMsg("Teleport position has no ground to land on. Teleport blocked...\n");
		return vec3_origin;
	}
	if (tr_forward.fraction != 1.0) //If the forward trace hits something, return 0,0,0
	{
		DevMsg("Teleport position is blocked by something. Teleport blocked...\n");
		return vec3_origin;
	}

	return vecStart; //All checks out, return ideal position.
}
bool CNPC_VortBrute::IsLeapAttackValid()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return false;

	if (!HasCondition(COND_ENEMY_FACING_ME))
		return false;

	float maxleapdist = g_pGameRules->SkillAdjustValue(1200.0f);

	if (GetEnemyDistance() > maxleapdist)
		return false;

	if (UTIL_GetLocalPlayer()->GetGroundEntity() == NULL)
		return false;

	if (flNextLeap > gpGlobals->curtime)
		return false;

	if (GetLeapDestination() == vec3_origin && GetEnemyDistance() > 300.0f)
		return false;

	return true;
}
bool CNPC_VortBrute::ShouldThrowProjectile()
{
	if (flNextThrow > gpGlobals->curtime) //if the timer hasn't passed yet, i can't throw
		return false;

	if (IsLeapAttackValid()) //if i can leap, i should do that instead.
		return false;

	return true;
}

bool CNPC_VortBrute::ShouldPunch()
{
	if (GetEnemy() && GetEnemyDistance() < 100.0f)
		return true;

	return false;
}
void CNPC_VortBrute::GatherConditions(void)
{
	if (ShouldPunch())
	{
		SetCondition(COND_IN_RANGE_TO_PUNCH);
		ClearCondition(COND_OUT_OF_PUNCH_RANGE);
	}
	else
	{
		SetCondition(COND_OUT_OF_PUNCH_RANGE);
		ClearCondition(COND_IN_RANGE_TO_PUNCH);
	}
	if (IsLeapAttackValid())
		SetCondition(COND_IN_RANGE_TO_LEAP);
	else
		ClearCondition(COND_IN_RANGE_TO_LEAP);

	if (ShouldThrowProjectile())
		SetCondition(COND_IN_RANGE_TO_THROW);
	else
		ClearCondition(COND_IN_RANGE_TO_THROW);

	BaseClass::GatherConditions();
}
void CNPC_VortBrute::TestTeleport(inputdata_t& inputdata)
{
	LeapTeleport();
}

int CNPC_VortBrute::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_COMBAT && GetEnemy())
		return SelectCombatSchedule();
	return SCHED_IDLE_WANDER;
}
int CNPC_VortBrute::SelectCombatSchedule(void)
{
	if (m_iMissCount >= 3)
		return SCHED_VORTBRUTE_RAGESLAM;

	if (HasCondition(COND_IN_RANGE_TO_PUNCH))
		return SCHED_VORTBRUTE_MELEE;

	if (HasCondition(COND_IN_RANGE_TO_THROW))
		return SCHED_VORTBRUTE_RANGED_ATTACK;

	if (HasCondition(COND_IN_RANGE_TO_LEAP))
		return SCHED_VORTBRUTE_LEAPATTACK;

	return SCHED_VORTBRUTE_CHASE_ENEMY;
}

void CNPC_VortBrute::LeapTeleport()
{
	Vector vecDest = GetLeapDestination();

	if (vecDest != vec3_origin)
	{
		GetMotor()->SetIdealYawToTargetAndUpdate(GetEnemy()->GetAbsOrigin());
		SetAbsOrigin(vecDest);
		Vector vecOrigin = GetAbsOrigin();
		float diff = WorldSpaceCenter().z - GetAbsOrigin().z;
		vecOrigin.z += diff;
		DispatchParticleEffect("vortigaunt_teleout", WorldSpaceCenter(), vec3_angle, this);
		DispatchParticleEffect("vortigaunt_telein", vecOrigin, vec3_angle, this);
		EmitSound("Vortbrute.Teleport");
	}
	else
	{
		DevWarning(2,"Something caused me to skip the teleport. See above for reason.\n");
	}
}

void CNPC_VortBrute::MeleeAttack()
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
		CTakeDamageInfo info(this, this, 25, DMG_CLUB);
		CalculateMeleeDamageForce(&info, forward, pBCC->GetAbsOrigin());
		pBCC->TakeDamage(info);
		EmitSound("Punch.Impact");
	}
}

Vector CNPC_VortBrute::FaceEnemyLead() //lead my punches so i swing where the player will be when the punch lands rather than where they were when i started swinging
{
	if (!GetEnemy())
		return vec3_origin;

	if (ai_vortbrute_legacy_face.GetBool() || g_pGameRules->GetSkillLevel() == SKILL_EASY)
		return GetEnemyLKP();

	Vector enemypos = GetEnemy()->GetAbsOrigin();
	Vector leadpos;
	UTIL_PredictedPosition(GetEnemy(), 0.25f, &leadpos);

	if (!FInViewCone(leadpos))
		return GetEnemyLKP();

	return leadpos;

}
void CNPC_VortBrute::HandleAnimEvent(animevent_t* pEvent)
{
	if (pEvent->event == AE_VORTBRUTE_LEAPATTACK_TELEPORT)
	{
		LeapTeleport();
		return;
	}

	if (pEvent->event == AE_VORTBRUTE_LEAPATTACK_LAND)
	{
		UTIL_ScreenShake(WorldSpaceCenter(), 40, 60, 1, 256, SHAKE_START, true);
		RadiusDamage(CTakeDamageInfo(this, this, 50, DMG_SONIC), GetAbsOrigin(), 180, CLASS_VORTIGAUNT, NULL);
		CBroadcastRecipientFilter filter2;
		te->BeamRingPoint(filter2, 0, GetAbsOrigin(),	//origin
			128,	//start radius
			512,		//end radius
			nRingTexture, //texture
			0,			//halo index
			0,			//start frame
			2,			//framerate
			0.4f,		//life
			48,			//width
			0,			//spread
			0,			//amplitude
			255,	//r
			255,	//g
			225,	//b
			150,		//a
			0,		//speed
			FBEAM_FADEOUT
		);

		EmitSound("BadDog.Smash");

		DevMsg(2,"Landing Leap Attack\n");
		return;
	}

	if (pEvent->event == AE_VORTBRUTE_RIGHTTHROW_THROW)
	{
		int iAttachment = LookupAttachment("rightclaw");
		Vector vecStart;
		QAngle angStart;
		GetAttachment(iAttachment, vecStart, angStart);
		float adjustedspd = g_pGameRules->SkillAdjustValue(sk_vortbrute_projectile_speed.GetFloat());
		Vector vecDir = VecCheckThrow(this, vecStart, GetEnemy()->WorldSpaceCenter(), adjustedspd);

		if (vecDir == vec3_origin)
		{
			DevWarning(2, "Failed to find throw vector.");
			return;
		}

		CBruteProjectile* pProj = CBruteProjectile::Create(vecStart, angStart, this);
		if (!pProj)
			return;

		pProj->SetAbsVelocity(vecDir);
		return;
	}

	if (pEvent->event == AE_VORTBRUTE_RIGHTHOOK_CONNECT)
	{
		DevMsg("right hook connecting\n");
		MeleeAttack();
		return;
	}
	if (pEvent->event == AE_VORTBRUTE_LEFTHOOK_CONNECT)
	{
		DevMsg("left hook connecting\n");
		MeleeAttack();
		return;
	}
	if (pEvent->event == AE_VORTBRUTE_SWING_SOUND)
	{
		EmitSound("NPC_Vortigaunt.Swing");
		return;
	}
	if (pEvent->event == AE_VORTBRUTE_RAGESLAM_IMPACT)
	{
		Vector vecForward;
		AngleVectors(GetAbsAngles(), &vecForward);
		VectorNormalize(vecForward);
		Vector vecPos = GetAbsOrigin() + (vecForward * 16);
		DispatchParticleEffect("vortbrute_groundslam", vecPos, GetAbsAngles());
	}
}


void CNPC_VortBrute::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_VORTBRUTE_RAGESLAM:
	{
		SetActivity(ACT_VORTBRUTE_RAGESLAM);
		m_iMissCount = 0;
		break;
	}
	case TASK_VORTBRUTE_LEAPATTACK:
	{
		float adjustedfreq = g_pGameRules->SkillAdjustValueInverted(sk_vortbrute_leap_frequency.GetFloat());
		flNextLeap = gpGlobals->curtime + adjustedfreq;
		SetPlaybackRate(0.8f);
		SetActivity(ACT_VORTBRUTE_LEAPATTACK);
		break;
	}
	case TASK_VORTBRUTE_RIGHTTHROW:
	{
		float adjustedfreq = g_pGameRules->SkillAdjustValueInverted(sk_vortbrute_throw_frequency.GetFloat());

		flNextThrow = gpGlobals->curtime + adjustedfreq;
		SetActivity(ACT_VORTBRUTE_RIGHTTHROW);
		break;
	}
	case TASK_VORTBRUTE_RIGHTHOOK:
	{
		SetActivity(ACT_VORTBRUTE_RIGHTHOOK);
		break;
	}
	case TASK_VORTBRUTE_LEFTHOOK:
	{
		SetActivity(ACT_VORTBRUTE_LEFTHOOK);
		break;
	}
	case TASK_VORTBRUTE_FACE_ENEMY_LEAD:
	{
		GetMotor()->SetIdealYawToTarget(FaceEnemyLead());
		GetMotor()->UpdateYaw();

		if (FacingIdeal())
			TaskComplete();
		break;
	}
	default:
	{
		BaseClass::StartTask(pTask);
		break;
	}
	}
}
void CNPC_VortBrute::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_VORTBRUTE_LEAPATTACK:
	{
		AutoMovement();

		if (IsActivityFinished())
		{
			TaskComplete();
			SetPlaybackRate(1.0f);
		}
		break;
	}
	case TASK_VORTBRUTE_RAGESLAM:
	{
		if (IsActivityFinished())
			TaskComplete();
		break;
	}
	case TASK_VORTBRUTE_RIGHTTHROW:
	{
		AutoMovement();
		if (IsActivityFinished())
			TaskComplete();
		break;
	}
	case TASK_VORTBRUTE_RIGHTHOOK:
	{
		if (IsActivityFinished())
			TaskComplete();
		break;
	}
	case TASK_VORTBRUTE_LEFTHOOK:
	{
		if (IsActivityFinished())
			TaskComplete();
		break;
	}
	case TASK_VORTBRUTE_FACE_ENEMY_LEAD:
	{
		GetMotor()->SetIdealYawToTarget(FaceEnemyLead());
		GetMotor()->UpdateYaw();

		if (FacingIdeal())
			TaskComplete();
		break;
	}
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}

void CNPC_VortBrute::BuildScheduleTestBits(void)
{
	BaseClass::BuildScheduleTestBits();

	if (IsCurSchedule(SCHED_VORTBRUTE_CHASE_ENEMY))
	{
		SetCustomInterruptCondition(COND_IN_RANGE_TO_LEAP);
		SetCustomInterruptCondition(COND_IN_RANGE_TO_THROW);
		SetCustomInterruptCondition(COND_IN_RANGE_TO_PUNCH);
	}
}



AI_BEGIN_CUSTOM_NPC(npc_vortbrute,CNPC_VortBrute)

	DECLARE_ACTIVITY(ACT_VORTBRUTE_LEAPATTACK)
	DECLARE_ACTIVITY(ACT_VORTBRUTE_RIGHTHOOK)
	DECLARE_ACTIVITY(ACT_VORTBRUTE_LEFTHOOK)
	DECLARE_ACTIVITY(ACT_VORTBRUTE_RIGHTTHROW)
	DECLARE_ACTIVITY(ACT_VORTBRUTE_LEFTTHROW)
	DECLARE_ACTIVITY(ACT_VORTBRUTE_RUN)
	DECLARE_ACTIVITY(ACT_VORTBRUTE_RAGESLAM)

	DECLARE_ANIMEVENT(AE_VORTBRUTE_RIGHTHOOK_CONNECT)
	DECLARE_ANIMEVENT(AE_VORTBRUTE_LEFTHOOK_CONNECT)
	DECLARE_ANIMEVENT(AE_VORTBRUTE_RIGHTTHROW_THROW)
	DECLARE_ANIMEVENT(AE_VORTBRUTE_LEFTTHROW_THROW)
	DECLARE_ANIMEVENT(AE_VORTBRUTE_LEAPATTACK_TELEPORT)
	DECLARE_ANIMEVENT(AE_VORTBRUTE_LEAPATTACK_LAND)
	DECLARE_ANIMEVENT(AE_VORTBRUTE_SWING_SOUND)
	DECLARE_ANIMEVENT(AE_VORTBRUTE_RAGESLAM_IMPACT)

	DECLARE_CONDITION(COND_IN_RANGE_TO_PUNCH)
	DECLARE_CONDITION(COND_IN_RANGE_TO_LEAP)
	DECLARE_CONDITION(COND_IN_RANGE_TO_THROW)
	DECLARE_CONDITION(COND_OUT_OF_PUNCH_RANGE)

	DECLARE_TASK(TASK_VORTBRUTE_LEAPATTACK)
	DECLARE_TASK(TASK_VORTBRUTE_RIGHTHOOK)
	DECLARE_TASK(TASK_VORTBRUTE_LEFTHOOK)
	DECLARE_TASK(TASK_VORTBRUTE_RIGHTTHROW)
	DECLARE_TASK(TASK_VORTBRUTE_LEFTTHROW)
	DECLARE_TASK(TASK_VORTBRUTE_FACE_ENEMY_LEAD)
	DECLARE_TASK(TASK_VORTBRUTE_RAGESLAM)


	DEFINE_SCHEDULE
	(
		SCHED_VORTBRUTE_LEAPATTACK,

		"	Tasks"
		"		TASK_VORTBRUTE_FACE_ENEMY_LEAD		0"
		"		TASK_VORTBRUTE_LEAPATTACK			0"
		"	"
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_ENEMY_DEAD"
	)
	DEFINE_SCHEDULE
		(
			SCHED_VORTBRUTE_CHASE_ENEMY,

			"	Tasks"
			"		TASK_STOP_MOVING				0"
			"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
			//	"		TASK_SET_TOLERANCE_DISTANCE		24"
			"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
			"		TASK_RUN_PATH					0"
			"		TASK_WAIT_FOR_MOVEMENT			0"
			"		TASK_FACE_ENEMY			0"
			""
			"	Interrupts"
			"		COND_IN_RANGE_TO_LEAP"
			"		COND_IN_RANGE_TO_PUNCH"
			"		COND_IN_RANGE_TO_THROW"
			"		COND_ENEMY_DEAD"
			"		COND_ENEMY_UNREACHABLE"
			"		COND_TASK_FAILED"
			"		COND_LOST_ENEMY"
			
		)
		DEFINE_SCHEDULE
		(
			SCHED_VORTBRUTE_RANGED_ATTACK,

			"	Tasks"
			"		TASK_FACE_ENEMY						0"
			"		TASK_VORTBRUTE_RIGHTTHROW			0"
			"	"
			"	Interrupts"
			"		COND_TASK_FAILED"
			"		COND_ENEMY_DEAD"
		)
		DEFINE_SCHEDULE
		(
			SCHED_VORTBRUTE_MELEE,

			"	Tasks"
			"		TASK_VORTBRUTE_FACE_ENEMY_LEAD		0"
			"		TASK_VORTBRUTE_RIGHTHOOK			0"
			"		TASK_VORTBRUTE_FACE_ENEMY_LEAD		0"
			"		TASK_VORTBRUTE_LEFTHOOK				0"
			"	"
			"	Interrupts"
			"		COND_TASK_FAILED"
			"		COND_ENEMY_DEAD"
		)
		DEFINE_SCHEDULE
		(
			SCHED_VORTBRUTE_RAGESLAM,

			"	Tasks"
			"		TASK_STOP_MOVING					0"
			"		TASK_FACE_ENEMY						0"
			"		TASK_VORTBRUTE_RAGESLAM				0"
			"	"
			"	Interrupts"
			"		COND_TASK_FAILED"
			"		COND_ENEMY_DEAD"
		)
AI_END_CUSTOM_NPC()