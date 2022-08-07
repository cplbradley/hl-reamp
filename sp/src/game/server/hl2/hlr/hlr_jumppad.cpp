#include "cbase.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "hl2_player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "weapon_rpg.h"
#include "gamemovement.h"
#include "game.h"
#include "vstdlib/random.h"
#include "particle_parse.h"
#include "hl2_gamerules.h"
#include "movevars_shared.h"
#include "gamestats.h"
#include "triggers.h"
#include "tier0/memdbgon.h"

#define JUMPPAD_MODEL "models/props_combine/combine_mine01.mdl"


ConVar debug_launchpad_dfo("debug_launchpad_dfo", "0", FCVAR_GAMEDLL);

class CLaunchpad : public CBaseEntity //set up the class
{
	DECLARE_CLASS(CLaunchpad, CBaseEntity);
public:
	void Spawn(void); //everything below is for setting up functions and variables
	void Precache(void);
	void Reenable(void);
	void AllowJump(void);
	void SetForce(inputdata_t &inputdata);
	void Enable(inputdata_t &inputdata);
	void Disable(inputdata_t &inputdata);
	void TouchThink(CBaseEntity *pOther);
	void PlayAnim(void);
	COutputEvent m_OnLaunch;
	void DistanceContext(void);
	float m_flgravitymod;
	virtual bool IsEnabled(void);
	bool m_bIsEnabled;
	bool m_bBlockAirControl;
	bool m_bBlockDistanceFromOrigin;
	//nsigned int PhysicsSolidMaskForEntity() const;
	float m_flpushforce;
	float m_fMaxThink;
	float m_fThinkLength;

	string_t m_iszSound;

	const char* target;
	Vector signalPoint;
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(hlr_launchpad, CLaunchpad);
LINK_ENTITY_TO_CLASS(hlr_jumppad, CLaunchpad);
BEGIN_DATADESC(CLaunchpad)
DEFINE_KEYFIELD(m_flpushforce, FIELD_FLOAT, "pushforce"),
DEFINE_KEYFIELD(target, FIELD_STRING, "target"),
DEFINE_KEYFIELD(m_iszSound, FIELD_SOUNDNAME, "puntsound"),
DEFINE_KEYFIELD(m_bIsEnabled, FIELD_BOOLEAN, "isenabled"),
DEFINE_KEYFIELD(m_flgravitymod, FIELD_FLOAT, "gravitymod"),
DEFINE_KEYFIELD(m_bBlockAirControl, FIELD_BOOLEAN, "blockaircontrol"),
DEFINE_KEYFIELD(m_bBlockDistanceFromOrigin, FIELD_BOOLEAN, "blocktype"),
DEFINE_KEYFIELD(m_fThinkLength, FIELD_FLOAT, "maxblocklength"),
DEFINE_OUTPUT(m_OnLaunch, "OnLaunch"),
// Function Pointers
DEFINE_FUNCTION(TouchThink),
DEFINE_FUNCTION(Reenable),

DEFINE_THINKFUNC(DistanceContext),
DEFINE_FUNCTION(AllowJump),
DEFINE_FUNCTION(PlayAnim),
DEFINE_INPUTFUNC(FIELD_VOID,"Enable",Enable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable",Disable),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetForce", SetForce),
END_DATADESC()
void CLaunchpad::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(20.0f, 20.0f, 20.0f), Vector(20.0f, 20.0f, 20.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER | FSOLID_USE_TRIGGER_BOUNDS);

	RegisterThinkContext("DistanceContext");
	RegisterThinkContext("ResetContext");
	RegisterThinkContext("AllowJumpContext");
	RegisterThinkContext("AnimContext");

	//SetModel(JUMPPAD_MODEL);
	SetTouch(&CLaunchpad::TouchThink);
}
void CLaunchpad::Precache(void)
{
	char *szSoundFile = (char *)STRING(m_iszSound);
	if (m_iszSound != NULL_STRING && strlen(szSoundFile) > 1)
	{
		if (*szSoundFile != '!')
		{
			PrecacheScriptSound(szSoundFile);
		}
	}
	PrecacheModel(JUMPPAD_MODEL);
	//PrecacheScriptSound("Weapon_Mortar.Single");
}
void CLaunchpad::SetForce(inputdata_t &inputdata)
{
	m_flpushforce = inputdata.value.Float();
}
void CLaunchpad::Enable(inputdata_t &inputdata)
{
	m_bIsEnabled = true;
}
void CLaunchpad::Disable(inputdata_t &inputdata)
{
	m_bIsEnabled = false;
}
bool CLaunchpad::IsEnabled(void)
{
	return m_bIsEnabled;
}
Vector VecCheckThrow(CBaseEntity *pEdict, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flTolerance, float flgravitymod)
{
	flSpeed = MAX(1.0f, flSpeed);

	float svgravity = GetCurrentGravity();

	float flGravity = svgravity * flgravitymod;
	//flGravity *= flgravitymod;

	Vector vecGrenadeVel = (vecSpot2 - vecSpot1);

	// throw at a constant time
	float time = vecGrenadeVel.Length() / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	vecApex.z += 0.5 * flGravity * (time * 0.5) * (time * 0.5);


	trace_t tr;
	UTIL_TraceLine(vecSpot1, vecApex, MASK_SOLID, pEdict, COLLISION_GROUP_NONE, &tr);


	UTIL_TraceLine(vecApex, vecSpot2, MASK_SOLID_BRUSHONLY, pEdict, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		bool bFail = true;

		// Didn't make it all the way there, but check if we're within our tolerance range
		if (flTolerance > 0.0f)
		{
			float flNearness = (tr.endpos - vecSpot2).LengthSqr();
			if (flNearness < Square(flTolerance))
			{


				bFail = false;
			}
		}

		if (bFail)
		{

			return vec3_origin;
		}
	}

	return vecGrenadeVel;
}
void CLaunchpad::TouchThink(CBaseEntity *pOther) //something touched me
{
	if (!pOther)
		return;
	if (!IsEnabled())
		return;
	if (pOther->IsWorld())
		return;
	SetTouch(NULL); //disable temporarily
	SetContextThink(&CLaunchpad::Reenable, gpGlobals->curtime + 0.1f, "ResetContext");//schedule re-enable
	//emit sound
	char *szSoundFile = (char *)STRING(m_iszSound);
	CBaseEntity *signalEntity = gEntList.FindEntityByName(NULL, target); //point to the target entity assigned in hammer

	if (signalEntity) //if it exists
	{
		signalPoint = signalEntity->GetAbsOrigin(); //get the absolute origin of the target, save it as a vector
		DevMsg("launchpad target found! using launchpad function\n");
	}
	else //if it doesn't exist 
	{
		DevMsg("launchpad target not found! using legacy jumppad function!\n"); //put an error in console
	}

	if (pOther->IsSolid()) //if what i touched is solid
	{
		m_OnLaunch.FireOutput(pOther, this);

		if (!signalEntity)
		{
			QAngle angSrc = this->GetAbsAngles(); //get angles from hammer
			Vector vecDir;
			Vector vecdnspd;
			Vector veccurvel;
			float fldnspd;
			float flpunchforce;
			AngleVectors(angSrc, &vecDir); //transform angles into a vector
			vecdnspd = pOther->GetAbsVelocity(); //get velocity vector
			fldnspd = vecdnspd[2]; //isolate y value of said vector
			flpunchforce = (m_flpushforce + abs(fldnspd)); //calculate punch force by adding abs of downward velocity to defined push force
			if (pOther->IsSolid()) //was the thing that touched me solid?
			{
				veccurvel = pOther->GetAbsVelocity(); //whats its current velocity
				veccurvel[2] += flpunchforce; //add the punch force to the y value of the current velocity
				pOther->SetAbsVelocity(veccurvel); //apply the new velocity
				//pOther->VelocityPunch(vecDir * flpunchforce); //do punch
				SetTouch(NULL); //disable temporarily
				EmitSound(szSoundFile);//emit sound
				if (pOther->IsPlayer())
				{
					SetContextThink(&CLaunchpad::AllowJump, gpGlobals->curtime + 0.5f, "AllowJumpContext");
					SetContextThink(&CLaunchpad::PlayAnim, gpGlobals->curtime + 0.1f, "AnimContext");
					extern IGameMovement *g_pGameMovement;
					CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
					gm->m_iJumpCount = 1;
					gm->m_bLaunchpadTimedBlock = true;
					
				}

			}
		}
		else
		{
			Vector vecToss = VecCheckThrow(this, GetAbsOrigin(), signalPoint, m_flpushforce, (10.0f*12.0f), m_flgravitymod); //calculate the trajectory
			Vector vecToTarget = (signalPoint - GetAbsOrigin());
			VectorNormalize(vecToTarget);
			float flVelocity = VectorNormalize(vecToss);

			Vector vecStall = Vector(0, 0, 0); //stall the entity 
			EmitSound(szSoundFile);
			pOther->SetAbsVelocity(vecStall);
			if (pOther->IsPlayer()) //if it's the player 
			{
				CBasePlayer* pPlayer = ToBasePlayer(pOther);
				CHL2_Player* phl2player = dynamic_cast<CHL2_Player*>(pPlayer);
				extern IGameMovement *g_pGameMovement; //call the game movement code 
				CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement); //create a pointer for the game movement code
				gm->m_iJumpCount = 1; //set the jump count to 1 
				Vector vecVel = gm->GetMoveData()->m_vecVelocity;
				gm->GetMoveData()->m_vecVelocity += -vecVel;
				gm->m_bLaunchpadTimedBlock = true;
				//pOther->VelocityPunch(vecToss * flVelocity); //launch the player
				pOther->SetAbsVelocity(vecToss * flVelocity);
				SetContextThink(&CLaunchpad::AllowJump, gpGlobals->curtime + 0.5f, "AllowJumpContext");
				SetContextThink(&CLaunchpad::PlayAnim, gpGlobals->curtime + 0.1f, "AnimContext");
				if (m_bBlockAirControl)
				{
					gm->m_bBlockingAirControl = true;

					if (m_bBlockDistanceFromOrigin)
					{
						gm->m_bBlockDistanceFromOrigin = true;
						SetContextThink(&CLaunchpad::DistanceContext, gpGlobals->curtime, "DistanceContext");
						m_fMaxThink = gpGlobals->curtime + m_fThinkLength;
					}
				}
				phl2player->SetAnimation(PLAYER_JUMP);
				
			}
			else
			{
				QAngle angDir;
				VectorAngles(vecToTarget, angDir); //get the angle from start to target
				if (pOther->GetMoveType() && pOther->GetMoveType() == MOVETYPE_FLY) //if it isn't affected by gravity
				{
					pOther->SetMoveType(MOVETYPE_FLYGRAVITY); //make it affected by gravity 
				}
				if (pOther->IsNPC()) //if it's an npc
				{
					pOther->SetGroundEntity(NULL); //register it as in the air
				}
				pOther->SetGravity(1.0f); //set the gravity to 100%
				pOther->SetAbsOrigin(GetAbsOrigin()); //set the absolute origin to that of the launchpad
				
				pOther->SetAbsVelocity(vecToss * flVelocity); //launch the object
				if (!pOther->IsNPC())
					pOther->SetAbsAngles(angDir); //point it towards the target
			}

		}
				
				
				//Msg("smarty launch\n");
	}
}
void CLaunchpad::AllowJump(void)
{
	extern IGameMovement* g_pGameMovement;
	CGameMovement* gm = dynamic_cast<CGameMovement*>(g_pGameMovement);
	gm->m_bLaunchpadTimedBlock = false;

}
void CLaunchpad::Reenable(void)
{
	SetTouch(&CLaunchpad::TouchThink);//re-enable
}
void CLaunchpad::PlayAnim(void)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player* phl2player = dynamic_cast<CHL2_Player*>(pPlayer);
	phl2player->SetAnimation(PLAYER_JUMP);
}
void CLaunchpad::DistanceContext(void)
{
	extern IGameMovement *g_pGameMovement; //call the game movement code 
	CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
	if (gpGlobals->curtime >= m_fMaxThink)
	{
		SetThink(NULL);
		gm->m_fDistanceFromOrigin = 100;
		return;
	}
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	Vector playerPos = pPlayer->GetAbsOrigin();
	Vector vecSrc = GetAbsOrigin();
	Vector vecEnd = signalPoint;
	vecEnd[2] = vecSrc[2];
	playerPos[2] = vecSrc[2];
	float disttotarget = (vecEnd - vecSrc).Length();
	float disttoplayer = (playerPos - vecSrc).Length();
	float ratio = disttoplayer / disttotarget;
	if (debug_launchpad_dfo.GetBool())
	{
		DebugDrawLine(vecSrc, playerPos, 0, 128, 255, false, 1.0f);
		DebugDrawLine(vecSrc, vecEnd, 255, 0, 0, false, 1.0f);
		Msg("dist from src to tgt: %f\n", disttotarget);
		Msg("dist from player to tgt: %f\n", disttoplayer);
		DevMsg("dist ratio: %f\n", ratio);
	}
	 //create a pointer for the game movement code
	gm->m_fDistanceFromOrigin = ratio;

	SetNextThink(gpGlobals->curtime + 0.01f, "DistanceContext");
}


class CTriggerLaunch : public CBaseTrigger
{
	DECLARE_CLASS(CTriggerLaunch, CBaseTrigger);
public:
	void Spawn(void);
	void Enable(void);
	void Touch(CBaseEntity* pOther);
	void Reenable(void);


	float m_flPushForce;


	COutputEvent m_OnLaunch;

	DECLARE_DATADESC()
};

LINK_ENTITY_TO_CLASS(trigger_launch, CTriggerLaunch);
BEGIN_DATADESC(CTriggerLaunch)
	DEFINE_KEYFIELD(m_flPushForce,FIELD_FLOAT,"PushForce")
END_DATADESC()

	


void CTriggerLaunch::Spawn(void)
{
	BaseClass::Spawn();
	InitTrigger();

}

void CTriggerLaunch::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
		return;

	QAngle angSrc = this->GetAbsAngles(); //get angles from hammer
	Vector vecDir;
	Vector vecdnspd;
	Vector veccurvel;
	float fldnspd;
	float flpunchforce;
	AngleVectors(angSrc, &vecDir); //transform angles into a vector
	vecdnspd = pOther->GetAbsVelocity(); //get velocity vector
	fldnspd = vecdnspd[2]; //isolate y value of said vector
	if (fldnspd < 0.0f)
		flpunchforce = (m_flPushForce + abs(fldnspd));
	else
		flpunchforce = (m_flPushForce); //calculate punch force by adding abs of downward velocity to defined push force
	if (pOther->IsSolid()) //was the thing that touched me solid?
	{
		veccurvel = pOther->GetAbsVelocity(); //whats its current velocity
		veccurvel[2] += flpunchforce; //add the punch force to the y value of the current velocity
		pOther->SetAbsVelocity(veccurvel); //apply the new velocity
		//pOther->VelocityPunch(vecDir * flpunchforce); //do punch
		SetNextThink(gpGlobals->curtime + 0.1f);//after 0.1 seconds
		if (pOther->IsPlayer())
		{
			extern IGameMovement* g_pGameMovement;
			CGameMovement* gm = dynamic_cast<CGameMovement*>(g_pGameMovement);
			gm->m_iJumpCount = 1;
			gm->m_bLaunchpadTimedBlock = true;
		}
		SetTouch(NULL);
		SetThink(&CTriggerLaunch::Reenable);
		SetNextThink(gpGlobals->curtime + 0.25f);
	}

}

void CTriggerLaunch::Reenable(void)
{
	extern IGameMovement* g_pGameMovement;
	CGameMovement* gm = dynamic_cast<CGameMovement*>(g_pGameMovement);
	gm->m_bLaunchpadTimedBlock = false;
	SetTouch(&CTriggerLaunch::Touch);
}