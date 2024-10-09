//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef MONSTERMAKER_H
#define MONSTERMAKER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"


//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------
#define	SF_NPCMAKER_START_ON		1	// start active ( if has targetname )
#define SF_NPCMAKER_NPCCLIP			8	// Children are blocked by NPCclip
#define SF_NPCMAKER_FADE			16	// Children's corpses fade
#define SF_NPCMAKER_INF_CHILD		32	// Infinite number of children
#define	SF_NPCMAKER_NO_DROP			64	// Do not adjust for the ground's position when checking for spawn
#define SF_NPCMAKER_HIDEFROMPLAYER	128 // Don't spawn if the player's looking at me
#define SF_NPCMAKER_ALWAYSUSERADIUS	256	// Use radius spawn whenever spawning
#define SF_NPCMAKER_NOPRELOADMODELS 512	// Suppress preloading into the cache of all referenced .mdl files
#define SF_NPCMAKER_INSTANT			1024 //Spawn instantly with the instant spawn particle
#define SF_NPCMAKER_NOPARTICLE		2048 //Don't use any particles at all, will always spawn the NPC instantly
#define SF_NPCMAKER_FAST			4096 //Fast-spawn NPCs. Takes 1 second instead of 2.
#define SF_NPCMAKER_SKILLADJUST_MAXCHILDREN 8192 //Adjust the Max Alive Children value based on skill. Only works if value is greater than 2.

//=========================================================
//=========================================================
class CNPCSpawnDestination : public CPointEntity
{
	DECLARE_CLASS(CNPCSpawnDestination, CPointEntity);

public:
	CNPCSpawnDestination();
	bool IsAvailable();						// Is this spawn destination available for selection?
	void OnSpawnedNPC(CAI_BaseNPC *pNPC);	// Notify this spawn destination that an NPC has spawned here.

	float		m_ReuseDelay;		// How long to be unavailable after being selected
	string_t	m_RenameNPC;		// If not NULL, rename the NPC that spawns here to this.
	float		m_TimeNextAvailable;// The time at which this destination will be available again.

	COutputEvent	m_OnSpawnNPC;

	bool bDontReuse;

	DECLARE_DATADESC();
};

abstract_class CBaseNPCMaker : public CBaseAnimating
{
public:
	DECLARE_CLASS(CBaseNPCMaker, CBaseAnimating);

	void Spawn(void);
	virtual int	ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void MakerThink(void);
	bool HumanHullFits(const Vector &vecLocation);
	bool CanMakeNPC(bool bIgnoreSolidEntities = false);
	//void TriggerMake(void);
	void Precache(void);

	virtual void DeathNotice(CBaseEntity *pChild);// NPC maker children use this to tell the NPC maker that they have died.
	virtual void MakeNPC(void) = 0;
	virtual void MakeParticle(Vector vecSrc);
	virtual void MakeInstantParticle(Vector vecSrc);
	virtual void MakeFastParticle(Vector vecSrc);
	virtual void SpawnNPC(void);

	virtual	void ChildPreSpawn(CAI_BaseNPC *pChild) {};
	virtual	void ChildPostSpawn(CAI_BaseNPC *pChild);

	virtual Vector GetNPCHullCenter(CAI_BaseNPC *pNPC);

	CBaseNPCMaker(void) {}

	// Input handlers
	void InputSpawnNPC(inputdata_t &inputdata);
	void InputEnable(inputdata_t &inputdata);
	void InputDisable(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);
	void InputSetMaxChildren(inputdata_t &inputdata);
	void InputAddMaxChildren(inputdata_t &inputdata);
	void InputSetMaxLiveChildren(inputdata_t &inputdata);
	void InputSetSpawnFrequency(inputdata_t &inputdata);
	

	bool m_bSuccessfulSpawn;
	bool SuccessfullySpawnedNPC(void) { return m_bSuccessfulSpawn; }

	// State changers
	void Toggle(void);
	virtual void Enable(void);
	virtual void Disable(void);

	virtual bool IsDepleted(void);

	DECLARE_DATADESC();

	int			m_nMaxNumNPCs;			// max number of NPCs this ent can create
	float		m_flSpawnFrequency;		// delay (in secs) between spawns 

	COutputEHANDLE m_OnSpawnNPC;
	COutputEvent m_OnAllSpawned;
	COutputEvent m_OnAllSpawnedDead;
	COutputEvent m_OnAllLiveChildrenDead;

	int		m_nLiveChildren;	// how many NPCs made by this NPC maker that are currently alive
	int		m_nMaxLiveChildren;	// max number of NPCs that this maker may have out at one time.
	int		m_nMaxHardChildren; // max number of live NPCs when difficulty is set to hard
	int		m_nMaxEasyChildren;// max number of live NPCs when difficulty is set to easy

	bool	m_bDisabled;

	EHANDLE m_hIgnoreEntity;
	EHANDLE m_hNPC[2048];
	int npcid;
	int queuedid[32];
	string_t m_iszIngoreEnt;
	string_t m_iszEnemyCounter;

	EHANDLE m_hEnemyCounter;
};


class CNPCMaker : public CBaseNPCMaker
{
public:
	DECLARE_CLASS(CNPCMaker, CBaseNPCMaker);

	CNPCMaker(void);

	void Precache(void);
	void Spawn(void);

	virtual void MakeNPC(void);
	virtual void SpawnNPC(void);

	DECLARE_DATADESC();

	string_t m_iszNPCClassname;			// classname of the NPC(s) that will be created.
	string_t m_SquadName;
	string_t m_strHintGroup;
	string_t m_spawnEquipment;
	string_t m_RelationshipString;		// Used to load up relationship keyvalues
	string_t m_ChildTargetName;
};

class CTemplateNPCMaker : public CBaseNPCMaker
{
public:
	DECLARE_CLASS(CTemplateNPCMaker, CBaseNPCMaker);

	CTemplateNPCMaker(void)
	{
		m_iMinSpawnDistance = 0;
	}

	virtual void Precache();
	void Spawn(void);

	virtual CNPCSpawnDestination *FindSpawnDestination();
	virtual void MakeNPC(void);
	void MakeNPCInRadius(void);
	void MakeNPCInLine(void);
	virtual void MakeMultipleNPCS(int nNPCs);
	virtual void SpawnNPC(void);

protected:
	virtual void PrecacheTemplateEntity(CBaseEntity *pEntity);

	bool PlaceNPCInRadius(CAI_BaseNPC *pNPC);
	bool PlaceNPCInLine(CAI_BaseNPC *pNPC);

	// Inputs
	void InputSpawnInRadius(inputdata_t &inputdata) { MakeNPCInRadius(); }
	void InputSpawnInLine(inputdata_t &inputdata) { MakeNPCInLine(); }
	void InputSpawnMultiple(inputdata_t &inputdata);
	void InputChangeDestinationGroup(inputdata_t &inputdata);
	void InputSetMinimumSpawnDistance(inputdata_t &inputdata);

	float	m_flRadius;

	DECLARE_DATADESC();

	string_t m_iszTemplateName;		// The name of the NPC that will be used as the template.
	string_t m_iszTemplateData;		// The keyvalue data blob from the template NPC that will be used to spawn new ones.
	string_t m_iszDestinationGroup;

	int		m_iMinSpawnDistance;

	enum ThreeStateYesNo_t
	{
		TS_YN_YES = 0,
		TS_YN_NO,
		TS_YN_DONT_CARE,
	};

	enum ThreeStateDist_t
	{
		TS_DIST_NEAREST = 0,
		TS_DIST_FARTHEST,
		TS_DIST_DONT_CARE,
	};

	ThreeStateYesNo_t	m_CriterionVisibility;
	ThreeStateDist_t	m_CriterionDistance;
};

#ifdef HLR
class CEnemyCounter : public CBaseEntity
{
	DECLARE_CLASS(CEnemyCounter, CBaseEntity);
	DECLARE_DATADESC();
public:
	void Spawn();
	virtual int UpdateTransmitState() { return SetTransmitState(FL_EDICT_ALWAYS); }

	void CounterThink();
	bool ShouldDraw();

	void SendData();

	void InputEnable(inputdata_t& inputdata);
	void InputDisable(inputdata_t& inputdata);
	void InputAdd(inputdata_t& data);
	void InputSubtract(inputdata_t& data);

	COutputEvent OnHitZero;
	COutputEvent OnHitThreshold;
	int m_iEnemyCount;
	int m_iDisplayThreshold;

private:
	
	bool m_bEnabled;
	bool bFiredOutput;
	int iPrevCount;
};

#endif
#endif // MONSTERMAKER_H