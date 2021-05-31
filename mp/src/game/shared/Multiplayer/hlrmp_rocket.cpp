//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Base Rockets.
//
//=============================================================================//
#include "cbase.h"
#include "hlrmp_rocket.h"
#include "hl2mp_gamerules.h"
#include "hlrmp_projectile_base.h"
#include "effect_dispatch_data.h"

// Server specific.
#ifdef GAME_DLL
#include "explode.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "iscorer.h"
extern void SendProxy_Origin(const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
extern void SendProxy_Angles(const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
#endif
#ifdef CLIENT_DLL
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"
#include "c_te_effect_dispatch.h"
#include "cliententitylist.h"

#endif


#define ROCKET_MODEL "models/weapons/w_missile.mdl"
#define ROCKET_CLIENT_EFFECT "ClientProjectile_Rocket"

ConVar hlrmp_debug_smoketrail("hlrmp_debug_smoketrail", "0", FCVAR_REPLICATED);
//=============================================================================
//
// TF Base Rocket tables.
//


LINK_ENTITY_TO_CLASS(hlrmp_rocket, CHLRMPRocket);
PRECACHE_REGISTER(hlrmp_rocket);

short g_sModelIndexRocket;
void PrecacheRocket(void *pUser)
{
	g_sModelIndexRocket = modelinfo->GetModelIndex(ROCKET_MODEL);
}
PRECACHE_REGISTER_FN(PrecacheRocket);
IMPLEMENT_NETWORKCLASS_ALIASED(HLRMPRocket, DT_HLRMPRocket)

BEGIN_NETWORK_TABLE(CHLRMPRocket, DT_HLRMPRocket)
// Client specific.
#ifdef CLIENT_DLL

RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),
RecvPropVector(RECVINFO(m_vecSentAngleVector)),
RecvPropVector(RECVINFO(m_vecSentVelocity)),

// Server specific.
#else

//SendPropExclude("DT_BaseEntity", "m_vecOrigin"),
//SendPropExclude("DT_BaseEntity", "m_angRotation"),

SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD_MP_INTEGRAL | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
SendPropQAngles(SENDINFO(m_angRotation), 6, SPROP_CHANGES_OFTEN, SendProxy_Angles),
SendPropVector(SENDINFO(m_vecSentAngleVector)),
SendPropVector(SENDINFO(m_vecSentVelocity)),

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
	AddEffects(EF_NODRAW);

	//SetCollisionGroup(TFCOLLISION_GROUP_ROCKETS);

	UTIL_SetSize(this, -Vector(0, 0, 0), Vector(0, 0, 0));

	// Setup attributes.
	m_takedamage = DAMAGE_NO;
	SetGravity(0.0f);

	// Setup the touch and think functions.
	SetTouch(&CHLRMPRocket::RocketTouch);
	//SetThink(&CHLRMPRocket::FlyThink);
	SetNextThink(gpGlobals->curtime);

	if (hlrmp_debug_smoketrail.GetInt() == 0)
	{
		CreateSmokeTrail();
	}
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
		/*C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		C_BaseEntity *pViewModel = pLocalPlayer->GetViewModel();
		Vector vForward, vRight, vUp;
		pLocalPlayer->EyeVectors(&vForward, &vRight, &vUp);

		Vector	muzzlePoint = pLocalPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;
		Vector vecSrc;

		//trace_t trace;
		//UTIL_TraceLine(vecSrc + vForward * -50, vecSrc, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace);
		
		Vector vecVelocity = (vForward * 1100.0f);

		pTemp = tempents->ClientProjectile(muzzlePoint, vecVelocity, vForward, this->GetModelIndex(), 3.0f, GetOwnerEntity(), NULL, NULL);*/
	}
	
}
int CHLRMPRocket::DrawModel(int flags)
{
	// During the first 0.2 seconds of our life, don't draw ourselves.
	/*if (gpGlobals->curtime - m_flSpawnTime < 0.2f)
		return 0;*/

	return BaseClass::DrawModel(flags);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//=============================================================================
//
// Server specific functions.
//
void ClientProjectileRocketCallback(const CEffectData &data)
{
	DevMsg("Projectile Callback\n");
	C_BasePlayer *pPlayer = dynamic_cast<C_BasePlayer*>(ClientEntityList().GetBaseEntityFromHandle(data.m_hEntity));
	if (pPlayer)
	{
		C_LocalTempEntity *pProj = ClientsideProjectileCallback(data, 0.5f);
		if (pProj)
		{
			pProj->AddEffects(EF_NOSHADOW);
			
		}
	}
}

DECLARE_CLIENT_EFFECT(ROCKET_CLIENT_EFFECT, ClientProjectileRocketCallback);

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHLRMPRocket *CHLRMPRocket::Create(const char *pszClassname, const Vector &vecOrigin,
	const QAngle &vecAngles, CBaseEntity *pOwner, const float flVelocity, float flTime)
{

	return static_cast<CHLRMPRocket*>( CHLRMPProjectileBase::Create("hlrmp_rocket", vecOrigin, vecAngles, pOwner, flVelocity, flTime, g_sModelIndexRocket, ROCKET_CLIENT_EFFECT));
	/*CHLRMPRocket *pRocket = static_cast<CHLRMPRocket*>(CBaseEntity::Create(pszClassname, vecOrigin, vecAngles, pOwner));
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
	
	Vector m_vecSentAngleVector = vecForward;
	Vector m_vecSentVelocity = vecVelocity;
	// Setup the initial angles.
	QAngle angles;
	VectorAngles(vecVelocity, angles);
	pRocket->SetAbsAngles(angles);

	// Set team.
	pRocket->ChangeTeam(pOwner->GetTeamNumber());

	return pRocket;*/
}
#ifdef GAME_DLL
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
		m_hRocketTrail->m_EndSize = 0;
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 8;

		m_hRocketTrail->SetLifetime(999);
		m_hRocketTrail->FollowEntity(this, "0");
	}
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#endif
