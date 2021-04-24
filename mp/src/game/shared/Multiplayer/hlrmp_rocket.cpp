//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Base Rockets.
//
//=============================================================================//
#include "cbase.h"
#include "hlrmp_rocket.h"
#include "hl2mp_gamerules.h"
#include "hlrmp_projectile_base.h"

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

RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),

// Server specific.
#else

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
//DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
DEFINE_FIELD(m_hRocketTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_flDamage, FIELD_FLOAT),
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

	// Client specific.
#ifdef CLIENT_DLL

	m_flSpawnTime = 0.0f;

	// Server specific.
#else

	m_flDamage = 0.0f;
	m_hRocketTrail = NULL;
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
	PrecacheModel("models/weapons/w_missile.mdl");
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
	SetModel("models/weapons/w_missile.mdl");

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

	CreateSmokeTrail();

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
int CHLRMPRocket::DrawModel(int flags)
{
	// During the first 0.2 seconds of our life, don't draw ourselves.
	if (gpGlobals->curtime - m_flSpawnTime < 0.1f)
		return 0;

	return BaseClass::DrawModel(flags);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
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
	pRocket->SetupInitialTransmittedVelocity(vecVelocity);

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

	if (m_hRocketTrail)
	{
		m_hRocketTrail->SetLifetime(0.1f);
		m_hRocketTrail = NULL;
	}

	// Remove the rocket.
	UTIL_Remove(this);
}
void CHLRMPRocket::CreateSmokeTrail(void)
{
	if (m_hRocketTrail)
		return;

	// Smoke trail.
	if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL)
	{
		m_hRocketTrail->m_Opacity = 0.2f;
		m_hRocketTrail->m_SpawnRate = 100;
		m_hRocketTrail->m_ParticleLifetime = 0.5f;
		m_hRocketTrail->m_StartColor.Init(0.65f, 0.65f, 0.65f);
		m_hRocketTrail->m_EndColor.Init(0.0, 0.0, 0.0);
		m_hRocketTrail->m_StartSize = 8;
		m_hRocketTrail->m_EndSize = 32;
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;

		m_hRocketTrail->SetLifetime(999);
		m_hRocketTrail->FollowEntity(this, "0");
	}
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#endif
