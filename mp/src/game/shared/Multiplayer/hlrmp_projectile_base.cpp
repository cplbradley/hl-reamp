


#include "cbase.h"
#include "hlrmp_projectile_base.h"

#ifdef GAME_DLL
#include "soundent.h"
#include "util.h"
#include "baseanimating.h"
#else
#include "iinput.h"
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
	}
}
int CHLRMPProjectileBase::DrawModel(int flags)
{
	// During the first 0.2 seconds of our life, don't draw ourselves.
	if (gpGlobals->curtime - m_flSpawnTime < 0.1f)
		return 0;

	return BaseClass::DrawModel(flags);
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