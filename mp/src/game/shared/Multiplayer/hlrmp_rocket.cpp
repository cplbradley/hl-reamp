//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Base Rockets.
//
//=============================================================================//
#include "cbase.h"
#include "hlrmp_rocket.h"
#include "hl2mp_gamerules.h"

// Server specific.
#ifdef GAME_DLL
#include "explode.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "iscorer.h"
extern void SendProxy_Origin(const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
extern void SendProxy_Angles(const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
#else

#endif

//=============================================================================
//
// TF Base Rocket tables.
//

#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS(hlrmp_rocket, CHLRMPRocket);
PRECACHE_REGISTER(hlrmp_rocket);
#endif
IMPLEMENT_NETWORKCLASS_ALIASED(HLRMPRocket, DT_HLRMPRocket)

BEGIN_NETWORK_TABLE(CHLRMPRocket, DT_HLRMPRocket)
// Client specific.
#ifdef CLIENT_DLL
RecvPropVector(RECVINFO(m_vInitialVelocity)),

RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),

// Server specific.
#else
SendPropVector(SENDINFO(m_vInitialVelocity), 12 /*nbits*/, 0 /*flags*/, -3000 /*low value*/, 3000 /*high value*/),

SendPropExclude("DT_BaseEntity", "m_vecOrigin"),
SendPropExclude("DT_BaseEntity", "m_angRotation"),

SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD_MP_INTEGRAL | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
SendPropQAngles(SENDINFO(m_angRotation), 6, SPROP_CHANGES_OFTEN, SendProxy_Angles),

#endif
END_NETWORK_TABLE()

// Server specific.
#ifdef GAME_DLL
BEGIN_DATADESC(CHLRMPRocket)
DEFINE_FUNCTION(RocketTouch),
//DEFINE_THINKFUNC(FlyThink),
END_DATADESC()
#endif

//ConVar tf_rocket_show_radius("tf_rocket_show_radius", "0", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Render rocket radius.");

//=============================================================================
//
// Shared (client/server) functions.
//

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CHLRMPRocket::CHLRMPRocket()
{
	m_vInitialVelocity.Init();

	// Client specific.
#ifdef CLIENT_DLL

	m_flSpawnTime = 0.0f;

	// Server specific.
#else

	m_flDamage = 0.0f;

#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CHLRMPRocket::~CHLRMPRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLRMPRocket::Precache(void)
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLRMPRocket::Spawn(void)
{
	// Precache.
	Precache();

	// Client specific.
#ifdef CLIENT_DLL

	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();
	//RemoveFromInterpolationList();

	// Server specific.
#else

	//Derived classes must have set model.
	SetModel("models/weapons/w_missile_launch.mdl");

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	AddEFlags(EFL_NO_WATER_VELOCITY_CHANGE);
	AddEffects(EF_NOSHADOW);

	//SetCollisionGroup(TFCOLLISION_GROUP_ROCKETS);

	UTIL_SetSize(this, -Vector(0, 0, 0), Vector(0, 0, 0));

	// Setup attributes.
	m_takedamage = DAMAGE_NO;
	SetGravity(0.0f);

	// Setup the touch and think functions.
	SetTouch(&CHLRMPRocket::RocketTouch);
	//SetThink(&CHLRMPRocket::FlyThink);
	SetNextThink(gpGlobals->curtime);

	// Don't collide with players on the owner's team for the first bit of our life
	m_flCollideWithTeammatesTime = gpGlobals->curtime + 0.25;
	m_bCollideWithTeammates = false;

#endif
}

//=============================================================================
//
// Client specific functions.
//
#ifdef CLIENT_DLL
static ConVar hlrmp_useinitialvelocity("hlrmp_useinitialvelocity", "1", FCVAR_CLIENTDLL, "");
static ConVar hlrmp_interpsamples("hlrmp_interpsamples", "1", FCVAR_CLIENTDLL, "");
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLRMPRocket::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);
	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}
void CHLRMPRocket::PostDataUpdate(DataUpdateType_t type)
{
	// Pass through to the base class.
	BaseClass::PostDataUpdate(type);


	if (!hlrmp_interpsamples.GetBool())
	{
		DevMsg("firing without adding interp samples\n");
		return;
	}
	if (type == DATA_UPDATE_CREATED)
	{
		/*// Now stick our initial velocity and angles into the interpolation history.
		/*CInterpolatedVar<Vector> &interpolator = GetOriginInterpolator();
		interpolator.ClearHistory();

		CInterpolatedVar<QAngle> &rotInterpolator = GetRotationInterpolator();
		rotInterpolator.ClearHistory();

		float flChangeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

		// Add a sample 1 second back.
		Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
		interpolator.AddToHead(flChangeTime - 1.0f, &vCurOrigin, false);

		QAngle vCurAngles = GetLocalAngles();
		rotInterpolator.AddToHead(flChangeTime - 1.0f, &vCurAngles, false);

		// Add the current sample.
		vCurOrigin = GetLocalOrigin();
		interpolator.AddToHead(flChangeTime, &vCurOrigin, false);

		rotInterpolator.AddToHead(flChangeTime - 1.0, &vCurAngles, false);
		// Now stick our initial velocity into the interpolation history 
		CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();

		interpolator.ClearHistory();
		float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

		// Add a sample 1 second back.
		Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
		interpolator.AddToHead(changeTime - 1.1, &vCurOrigin, false);

		// Add the current sample.
		vCurOrigin = GetLocalOrigin();
		interpolator.AddToHead(changeTime, &vCurOrigin, false);*/

		if (hlrmp_useinitialvelocity.GetInt() == 1)
		{
			DevMsg("Firing with initial velocity\n");
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();

			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

			/*			for (float i = -0.5f; i < 0.5f; i++)
			{
			Vector vecPosition = GetLocalOrigin() + GetAbsVelocity() * i;
			interpolator.AddToHead(changeTime + i, &vecPosition, false);
			}
			return;*/

			// Add a sample 2 seconds back.
			Vector vecCurOrigin = GetLocalOrigin() - m_vInitialVelocity * 0.75f;
			interpolator.AddToHead(changeTime - 2.0f, &vecCurOrigin, false);

			// Add a sample 1 second back.
			vecCurOrigin = GetLocalOrigin() - (m_vInitialVelocity * 0.5f);
			interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);

			/*			Vector vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 1.5f;
			interpolator.AddToHead(changeTime - 1.5f, &vecCurOrigin, false);
			vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 1.0f;
			interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);
			vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 0.5f;
			interpolator.AddToHead(changeTime - 0.5f, &vecCurOrigin, false);*/

			// Add the current sample.
			vecCurOrigin = GetLocalOrigin();
			interpolator.AddToHead(changeTime, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + m_vInitialVelocity * 0.5f;
			interpolator.AddToHead(changeTime + 0.5f, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + m_vInitialVelocity * 1.0f;
			interpolator.AddToHead(changeTime + 1.0f, &vecCurOrigin, false);
		}
		else
		{
			DevMsg("Firing without initial velocity\n");
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();

			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

			/*			for (float i = -0.5f; i < 0.5f; i++)
			{
			Vector vecPosition = GetLocalOrigin() + GetAbsVelocity() * i;
			interpolator.AddToHead(changeTime + i, &vecPosition, false);
			}
			return;*/

			// Add a sample 2 seconds back.
			Vector vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 0.75f;
			interpolator.AddToHead(changeTime - 2.0f, &vecCurOrigin, false);

			// Add a sample 1 second back.
			vecCurOrigin = GetLocalOrigin() - (/*m_vecInitialVelocity*/ GetAbsVelocity() * 0.5f);
			interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);

			/*			Vector vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 1.5f;
			interpolator.AddToHead(changeTime - 1.5f, &vecCurOrigin, false);
			vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 1.0f;
			interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);
			vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 0.5f;
			interpolator.AddToHead(changeTime - 0.5f, &vecCurOrigin, false);*/

			// Add the current sample.
			vecCurOrigin = GetLocalOrigin();
			interpolator.AddToHead(changeTime, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + GetAbsVelocity() * 0.5f;
			interpolator.AddToHead(changeTime + 0.5f, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + GetAbsVelocity() * 1.0f;
			interpolator.AddToHead(changeTime + 1.0f, &vecCurOrigin, false);

		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CHLRMPRocket::DrawModel(int flags)
{
	// During the first 0.2 seconds of our life, don't draw ourselves.
	/*if (gpGlobals->curtime - m_flSpawnTime < 0.2f)
		return 0;*/

	return BaseClass::DrawModel(flags);
}

//=============================================================================
//
// Server specific functions.
//
#else

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHLRMPRocket *CHLRMPRocket::Create(const char *pszClassname, const Vector &vecOrigin,
	const QAngle &vecAngles, CBaseEntity *pOwner)
{
	CHLRMPRocket *pRocket = static_cast<CHLRMPRocket*>(CBaseEntity::Create(pszClassname, vecOrigin, vecAngles, pOwner));
	if (!pRocket)
		return NULL;

	// Initialize the owner.
	pRocket->SetOwnerEntity(pOwner);

	// Spawn.
	pRocket->Spawn();

	// Setup the initial velocity.
	Vector vecForward, vecRight, vecUp;
	AngleVectors(vecAngles, &vecForward, &vecRight, &vecUp);

	Vector vecVelocity = vecForward * 1100.0f;
	pRocket->SetAbsVelocity(vecVelocity);
	pRocket->SetupInitialTransmittedGrenadeVelocity(vecVelocity);

	// Setup the initial angles.
	QAngle angles;
	VectorAngles(vecVelocity, angles);
	pRocket->SetAbsAngles(angles);

	// Set team.
	pRocket->ChangeTeam(pOwner->GetTeamNumber());

	return pRocket;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLRMPRocket::RocketTouch(CBaseEntity *pOther)
{
	// Verify a correct "other."
	Assert(pOther);
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
		return;

	// Handle hitting skybox (disappear).
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	if (pTrace->surface.flags & SURF_SKY)
	{
		UTIL_Remove(this);
		return;
	}

	trace_t trace;
	memcpy(&trace, pTrace, sizeof(trace_t));
	Explode(&trace, pOther);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
unsigned int CHLRMPRocket::PhysicsSolidMaskForEntity(void) const
{
	int teamContents = 0;

	if (m_bCollideWithTeammates == false)
	{
		// Only collide with the other team
		teamContents = (GetTeamNumber() == TEAM_COMBINE) ? CONTENTS_TEAM1 : CONTENTS_TEAM2;
	}
	else
	{
		// Collide with both teams
		teamContents = CONTENTS_TEAM1 | CONTENTS_TEAM2;
	}

	return BaseClass::PhysicsSolidMaskForEntity() | teamContents;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLRMPRocket::Explode(trace_t *pTrace, CBaseEntity *pOther)
{
	// Save this entity as enemy, they will take 100% damage.
	m_hEnemy = pOther;

	// Invisible.
	SetModelName(NULL_STRING);
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = DAMAGE_NO;

	// Pull out a bit.
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
	}

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	CPVSFilter filter(vecOrigin);
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), GetDamage(), GetDamage() * 2,
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);
	CSoundEnt::InsertSound(SOUND_COMBAT, vecOrigin, 1024, 3.0);

	// Damage.
	CBaseEntity *pAttacker = GetOwnerEntity();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>(pAttacker);
	if (pScorerInterface)
	{
		pAttacker = pScorerInterface->GetScorer();
	}

	CTakeDamageInfo info(this, pAttacker, vec3_origin, vecOrigin, GetDamage(), GetDamageType());
	float flRadius = GetRadius();
	RadiusDamage(info, vecOrigin, flRadius, CLASS_NONE, NULL);

	// Debug!
	/*if (tf_rocket_show_radius.GetBool())
	{
		DrawRadius(flRadius);
	}*/

	// Don't decal players with scorch.
	if (!pOther->IsPlayer())
	{
		UTIL_DecalTrace(pTrace, "Scorch");
	}

	// Remove the rocket.
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#endif
