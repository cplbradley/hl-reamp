#ifndef VORTBRUTE_H
#define VORTBRUTE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ai_schedule.h"
#include "ai_basenpc.h"
#include "ai_blended_movement.h"

class CBruteProjectile : public CBaseAnimating
{
	DECLARE_CLASS(CBruteProjectile, CBaseAnimating);
public:
	void Spawn();
	void Precache();
	void CreateParticles();
	void Touch(CBaseEntity* pOther);


	static CBruteProjectile* Create(const Vector& vecOrigin, const QAngle& angAngle, CBaseEntity* pOwner = NULL);

	DECLARE_DATADESC();
};

class CNPC_VortBrute : public CAI_BlendedNPC
{
	DECLARE_CLASS(CNPC_VortBrute, CAI_BlendedNPC);
	//DECLARE_SERVERCLASS();
public:
	void Precache();
	void Spawn();
	Class_T Classify() { return CLASS_VORTIGAUNT; }

	Vector GetLeapDestination();
	void LeapTeleport(void);
	void TestTeleport(inputdata_t& inputdata);
	bool IsLeapAttackValid();

	bool ShouldThrowProjectile();
	bool ShouldPunch();
	float GetEnemyDistance();

	void MeleeAttack();

	int SelectCombatSchedule(void);
	int SelectSchedule(void);

	void	HandleAnimEvent(animevent_t* pEvent);
	void	StartTask(const Task_t* pTask);
	void	RunTask(const Task_t* pTask);
	void	GatherConditions(void);

	void BuildScheduleTestBits(void);

	void		PostNPCInit();

	float	MaxYawSpeed(void);

	Vector FaceEnemyLead();

	int m_iMissCount;

private:
	float flNextLeap;
	float flNextThrow;

protected:
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
};

#endif 