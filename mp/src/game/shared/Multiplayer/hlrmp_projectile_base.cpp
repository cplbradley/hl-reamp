


#include "cbase.h"
#include "hlrmp_projectile_base.h"
#include "effect_dispatch_data.h"

#ifdef GAME_DLL
#include "soundent.h"
#include "util.h"
#include "baseanimating.h"
#include "te_effect_dispatch.h"
#else
#include "iinput.h"
#include "c_te_legacytempents.h"
#include "c_te_effect_dispatch.h"

#endif 

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

//=============================================================================
// CFFProjectileBase tables
//=============================================================================

#ifdef GAME_DLL
BEGIN_DATADESC(CHLRMPProjectileBase)
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(HLRMPProjectileBase, DT_HLRMPProjectileBase)

BEGIN_NETWORK_TABLE(CHLRMPProjectileBase, DT_HLRMPProjectileBase)
#ifdef CLIENT_DLL
RecvPropVector(RECVINFO(m_vecInitialVelocity)),
#else
SendPropVector(SENDINFO(m_vecInitialVelocity), 12 /*nbits*/, 0 /*flags*/, -3000 /*low value*/, 3000 /*high value*/),
#endif
END_NETWORK_TABLE()

//=============================================================================
// CFFProjectileBase implementation
//=============================================================================

#ifdef CLIENT_DLL

//----------------------------------------------------------------------------
// Purpose: When the rocket enters the client's PVS, add the flight sound
//			to it. This is done here rather than PostDataUpdate because 
//			origins (needed for emitsound) are not valid there
//----------------------------------------------------------------------------

static ConVar hlrmp_useinitialvelocity("hlrmp_useinitialvelocity", "1", FCVAR_CLIENTDLL, "");
static ConVar hlrmp_interpsamples("hlrmp_interpsamples", "1", FCVAR_CLIENTDLL, "");


void CHLRMPProjectileBase::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::PostDataUpdate(type);


	if (!hlrmp_interpsamples.GetBool())
	{
		DevMsg("firing without adding interp samples\n");
		return;
	}
	if (type == DATA_UPDATE_CREATED)
	{
		if (hlrmp_useinitialvelocity.GetInt() == 1)
		{
			DevMsg("Firing with initial velocity\n");
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();

			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

			// Add a sample 2 seconds back.
			Vector vecCurOrigin = GetLocalOrigin() - m_vecInitialVelocity * 0.75f;
			interpolator.AddToHead(changeTime - 2.0f, &vecCurOrigin, false);

			// Add a sample 1 second back.
			vecCurOrigin = GetLocalOrigin() - (m_vecInitialVelocity * 0.5f);
			interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);

			// Add the current sample.
			vecCurOrigin = GetLocalOrigin();
			interpolator.AddToHead(changeTime, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + m_vecInitialVelocity * 0.5f;
			interpolator.AddToHead(changeTime + 0.5f, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + m_vecInitialVelocity * 1.0f;
			interpolator.AddToHead(changeTime + 1.0f, &vecCurOrigin, false);
		}
		else
		{
			DevMsg("Firing without initial velocity\n");
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();

			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

			// Add a sample 2 seconds back.
			Vector vecCurOrigin = GetLocalOrigin() - GetAbsVelocity() * 0.75f;
			interpolator.AddToHead(changeTime - 2.0f, &vecCurOrigin, false);

			// Add a sample 1 second back.
			vecCurOrigin = GetLocalOrigin() - (/*m_vecInitialVelocity*/ GetAbsVelocity() * 0.5f);
			interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);

			// Add the current sample.
			vecCurOrigin = GetLocalOrigin();
			interpolator.AddToHead(changeTime, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + GetAbsVelocity() * 0.5f;
			interpolator.AddToHead(changeTime + 0.5f, &vecCurOrigin, false);

			vecCurOrigin = GetLocalOrigin() + GetAbsVelocity() * 1.0f;
			interpolator.AddToHead(changeTime + 1.0f, &vecCurOrigin, false);

		}

		/*C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
//		C_BaseEntity *pViewModel = pLocalPlayer->GetViewModel();
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
int CHLRMPProjectileBase::DrawModel(int flags)
{
	// During the first 0.2 seconds of our life, don't draw ourselves.
	if (gpGlobals->curtime - m_flSpawnTime < 0.1f)
		return 0;

	return BaseClass::DrawModel(flags);
}

C_LocalTempEntity *ClientsideProjectileCallback( const CEffectData &data, float flGravityBase, const char *pszParticleName )
{
	DevMsg("Baseclass Client Projectile Callback\n");
	// Create a nail temp ent, and give it an impact callback to use
	C_BaseEntity *pEnt = C_BaseEntity::Instance( data.m_hEntity );

	if ( !pEnt || pEnt->IsDormant() )
	{
		Assert( 0 );
		return NULL;
	}

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	//		C_BaseEntity *pViewModel = pLocalPlayer->GetViewModel();
	Vector vForward, vRight, vUp;
	pLocalPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	muzzlePoint = pLocalPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;
	

	trace_t trace;
	UTIL_TraceLine(muzzlePoint + vForward * -50, data.m_vOrigin, MASK_SOLID, pEnt, COLLISION_GROUP_NONE, &trace);
	Vector vecSrc = trace.endpos;

	Vector vecVelocity = (vForward * 1100.0f);



	return tempents->ClientProjectile(vecSrc, data.m_vStart, vForward, data.m_nMaterial, 6.0f, pEnt, NULL, NULL);
}

#else
//----------------------------------------------------------------------------
// Purpose: Specify what velocity we want to have on the client immediately.
//			Without this, the entity wouldn't have an interpolation history initially, so it would
//			sit still until it had gotten a few updates from the server.

void CHLRMPProjectileBase::SetupInitialTransmittedVelocity(const Vector &velocity)
{
	m_vecInitialVelocity = velocity;
}
#endif

//----------------------------------------------------------------------------
// Purpose: Keep track of when spawned
//----------------------------------------------------------------------------
void CHLRMPProjectileBase::Spawn()
{
	m_flSpawnTime = gpGlobals->curtime;

	BaseClass::Spawn();

	SetSolidFlags(FSOLID_NOT_STANDABLE);

	// New Group that everything "projectiles" do, and players -GreenMushy
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
}


CHLRMPProjectileBase *CHLRMPProjectileBase::Create(const char *pszClassName, const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pOwner, float flVelocity, float flTime, short iProjectileModel, const char *pszDispatchEffect)
{
	CHLRMPProjectileBase *pProjectile = NULL;
	Vector vecForward, vecRight, vecUp;
	AngleVectors(angAngles, &vecForward, &vecRight, &vecUp);
	CEffectData data;
	Vector vecVelocity = vecForward * flVelocity;
#ifdef GAME_DLL
	pProjectile = static_cast<CHLRMPProjectileBase*>(CBaseEntity::Create(pszClassName, vecOrigin, angAngles, pOwner));
	if (!pProjectile)
		return NULL;

	pProjectile->SetOwnerEntity(pOwner);
	pProjectile->Spawn();
	pProjectile->SetAbsVelocity(vecVelocity);
	QAngle angles;
	VectorAngles(vecVelocity, angles);
	pProjectile->SetAbsAngles(angles);

	pProjectile->AddEffects(EF_NODRAW);
	
#endif
	if (pszDispatchEffect)
	{
		trace_t tr;
		UTIL_TraceLine(vecOrigin, vecOrigin + vecForward * MAX_COORD_RANGE, (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE), pOwner, COLLISION_GROUP_NONE, &tr);
		bool bBroadcast = (UTIL_PointContents(vecOrigin) != UTIL_PointContents(tr.endpos));
		IRecipientFilter *pFilter;
		if ( bBroadcast )
		{
			// The projectile is going to cross content types 
			// (which will block PVS/PAS). Send to every client
			pFilter = new CReliableBroadcastRecipientFilter();
	}
		else
		{
			// just the PVS of where the projectile will hit.
			pFilter = new CPASFilter(tr.endpos);
		}
		
		data.m_vOrigin = vecOrigin;
		data.m_vStart = vecVelocity;
		data.m_fFlags = 6;
		data.m_flMagnitude = flTime;
#ifdef GAME_DLL
		data.m_nMaterial = pProjectile->GetModelIndex();
		data.m_nEntIndex = pOwner->entindex();
#else
		data.m_nMaterial = iProjectileModel;
		data.m_hEntity = ClientEntityList().EntIndexToHandle(pOwner->entindex());
		DispatchEffect(pszDispatchEffect,data);
#endif
		
		//DispatchEffect(pszDispatchEffect, data);
		DevMsg("Dispatching Client Projectile %s\n",pszDispatchEffect);
	}

	return pProjectile;
}
//----------------------------------------------------------------------------
// Purpose: Precache the bounce and flight sounds
//----------------------------------------------------------------------------
void CHLRMPProjectileBase::Precache()
{
}

//----------------------------------------------------------------------------
// Purpose: Client & server constructor
//----------------------------------------------------------------------------
CHLRMPProjectileBase::CHLRMPProjectileBase()
{
	m_flSpawnTime = 0.0f;
}