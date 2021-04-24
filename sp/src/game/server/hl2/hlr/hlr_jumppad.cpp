#include "cbase.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
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

#include "tier0/memdbgon.h"

#define JUMPPAD_MODEL "models/props_combine/combine_mine01.mdl"
class CJumppad : public CBaseEntity
{
	DECLARE_CLASS(CJumppad, CBaseEntity);
public:
	void Spawn(void);
	void Precache(void);
	void Reenable(void);
	void SetForce(float flpushforce);
	void TouchThink(CBaseEntity *pOther);
	unsigned int PhysicsSolidMaskForEntity() const;
	float m_flpushforce;
DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(hlr_jumppad, CJumppad);
BEGIN_DATADESC(CJumppad)
DEFINE_KEYFIELD(m_flpushforce, FIELD_FLOAT, "pushforce"),
// Function Pointers
DEFINE_FUNCTION(TouchThink),
END_DATADESC()
unsigned int CJumppad::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CJumppad::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(20.0f, 20.0f, 20.0f), Vector(20.0f, 20.0f, 20.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	//SetModel(JUMPPAD_MODEL);
	SetTouch(&CJumppad::TouchThink);
}
void CJumppad::Precache(void)
{
	PrecacheModel(JUMPPAD_MODEL);
	PrecacheScriptSound("Weapon_Mortar.Single");
}
void CJumppad::SetForce(float flpushforce)
{
	m_flpushforce = flpushforce;
}
void CJumppad::TouchThink(CBaseEntity *pOther) //something touched me
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
	if (m_flpushforce == NULL) //if no value is set in hammer
	{
		m_flpushforce = 1000;
	}
	flpunchforce = (m_flpushforce + abs(fldnspd)); //calculate punch force by adding abs of downward velocity to defined push force
	if (pOther->IsSolid()) //was the thing that touched me solid?
	{
		veccurvel = pOther->GetAbsVelocity(); //whats its current velocity
		veccurvel[2] += flpunchforce; //add the punch force to the y value of the current velocity
		pOther->SetAbsVelocity(veccurvel); //apply the new velocity
		//pOther->VelocityPunch(vecDir * flpunchforce); //do punch
		SetTouch(NULL); //disable temporarily
		SetThink(&CJumppad::Reenable);//schedule re-enable
		SetNextThink(gpGlobals->curtime + 0.1f);//after 0.1 seconds
		EmitSound("Weapon_Mortar.Single");//emit sound
		if (pOther->IsPlayer())
		{
			extern IGameMovement *g_pGameMovement;
			CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
			gm->m_iJumpCount = 1;
		}
	}

}
void CJumppad::Reenable(void)
{
	SetTouch(&CJumppad::TouchThink);//re-enable
}
///
///
///
///
class CLaunchpad : public CBaseEntity //set up the class
{
	DECLARE_CLASS(CLaunchpad, CBaseEntity);
public:
	void Spawn(void); //everything below is for setting up functions and variables
	void Precache(void);
	void Reenable(void);
	void SetForce(inputdata_t &inputdata);
	void Enable(inputdata_t &inputdata);
	void Disable(inputdata_t &inputdata);
	void TouchThink(CBaseEntity *pOther);
	float m_flgravitymod;
	virtual bool IsEnabled(void);
	bool m_bIsEnabled;
	//nsigned int PhysicsSolidMaskForEntity() const;
	float m_flpushforce;
	const char* target;
	Vector signalPoint;
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(hlr_launchpad, CLaunchpad);
BEGIN_DATADESC(CLaunchpad)
DEFINE_INPUT(m_flpushforce, FIELD_FLOAT, "pushforce"),
DEFINE_KEYFIELD(target, FIELD_STRING, "target"),
DEFINE_KEYFIELD(m_bIsEnabled, FIELD_BOOLEAN, "isenabled"),
DEFINE_KEYFIELD(m_flgravitymod, FIELD_FLOAT, "gravitymod"),
// Function Pointers
DEFINE_FUNCTION(TouchThink),
DEFINE_FUNCTION(Reenable),
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


	//SetModel(JUMPPAD_MODEL);
	SetTouch(&CLaunchpad::TouchThink);
}
void CLaunchpad::Precache(void)
{
	PrecacheModel(JUMPPAD_MODEL);
	PrecacheScriptSound("Weapon_Mortar.Single");
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
	SetTouch(NULL); //disable temporarily
	SetThink(&CLaunchpad::Reenable);//schedule re-enable
	SetNextThink(gpGlobals->curtime + 0.1f);//after 0.1 seconds
	//emit sound
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
				SetThink(&CJumppad::Reenable);//schedule re-enable
				SetNextThink(gpGlobals->curtime + 0.1f);//after 0.1 seconds
				EmitSound("Weapon_Mortar.Single");//emit sound
				if (pOther->IsPlayer())
				{
					extern IGameMovement *g_pGameMovement;
					CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
					gm->m_iJumpCount = 1;
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
			EmitSound("Weapon_Mortar.Single");
			pOther->SetAbsVelocity(vecStall);
			if (pOther->IsPlayer()) //if it's the player 
			{
				extern IGameMovement *g_pGameMovement; //call the game movement code 
				CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement); //create a pointer for the game movement code
				gm->m_iJumpCount = 1; //set the jump count to 1 
				Vector vecVel = gm->GetMoveData()->m_vecVelocity;
				gm->GetMoveData()->m_vecVelocity += -vecVel;
				//pOther->VelocityPunch(vecToss * flVelocity); //launch the player
				pOther->SetAbsVelocity(vecToss * flVelocity);

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
				pOther->SetAbsAngles(angDir); //point it towards the target
			}

		}
				
				
				//Msg("smarty launch\n");
	}
}
void CLaunchpad::Reenable(void)
{
	SetTouch(&CLaunchpad::TouchThink);//re-enable
}