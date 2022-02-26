//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "grenade_frag.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "particle_parse.h"
#include "soundent.h"
#include "explode.h"
#include "hl2_gamestats.h"
#include "physics_saverestore.h"
#include "vphysics/constraints.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FRAG_GRENADE_BLIP_FREQUENCY			1.0f
#define FRAG_GRENADE_BLIP_FAST_FREQUENCY	0.3f

#define FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP 1.5f
#define FRAG_GRENADE_WARN_TIME 1.5f

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

ConVar sk_plr_dmg_fraggrenade	( "sk_plr_dmg_fraggrenade","0");
ConVar sk_npc_dmg_fraggrenade	( "sk_npc_dmg_fraggrenade","0");
ConVar sk_fraggrenade_radius	( "sk_fraggrenade_radius", "0");

#define GRENADE_MODEL "models/Weapons/w_grenade.mdl"
extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

LINK_ENTITY_TO_CLASS( npc_grenade_frag, CGrenadeFrag );

BEGIN_DATADESC( CGrenadeFrag )

	// Fields
	DEFINE_FIELD( m_pMainGlow, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_combineSpawned, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_punted, FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_THINKFUNC( DelayThink ),
	DEFINE_ENTITYFUNC (NadeTouch),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTimer", InputSetTimer ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeFrag::~CGrenadeFrag( void )
{
}

void CGrenadeFrag::Spawn( void )
{
	Precache( );
	
	SetModel( GRENADE_MODEL );

	if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() )
	{
		m_flDamage		= sk_plr_dmg_fraggrenade.GetFloat();
		m_DmgRadius		= sk_fraggrenade_radius.GetFloat();
	}
	else
	{
		m_flDamage		= sk_npc_dmg_fraggrenade.GetFloat();
		m_DmgRadius		= sk_fraggrenade_radius.GetFloat();
	}


	AddEffects(EF_NOINTERP);
	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;
	iBounces		= 1;
	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	SetCollisionBoundsFromModel();
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	//CreateVPhysics();
	

	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;

	SetTouch(&CGrenadeFrag::NadeTouch);

	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetSolid(SOLID_BBOX);

	m_combineSpawned	= false;
	m_punted			= false;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeFrag::OnRestore( void )
{
	// If we were primed and ready to detonate, put FX on us.
	if (m_flDetonateTime > 0)
		CreateEffects();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeFrag::CreateEffects( void )
{
	// Start up the eye glow
	m_pMainGlow = CSprite::SpriteCreate( "sprites/redglow1.vmt", GetLocalOrigin(), false );

	int	nAttachment = LookupAttachment( "fuse" );

	if ( m_pMainGlow != NULL )
	{
		m_pMainGlow->FollowEntity( this );
		m_pMainGlow->SetAttachment( this, nAttachment );
		m_pMainGlow->SetTransparency( kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation );
		m_pMainGlow->SetScale( 0.2f );
		m_pMainGlow->SetGlowProxySize( 4.0f );
	}

	// Start up the eye trail
	m_pGlowTrail	= CSpriteTrail::SpriteTrailCreate( "sprites/bluelaser1.vmt", GetLocalOrigin(), false );

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		//m_pGlowTrail->SetAttachment( this, nAttachment );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 0, 0, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 16.0f );
		m_pGlowTrail->SetEndWidth( 1.0f );
		m_pGlowTrail->SetLifeTime( 3.0f );
	}
}

/*bool CGrenadeFrag::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);
	return true;
}*/

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
/*class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );
	
	CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( pEntity )
		{
			if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
				return false;
			if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};*/

/*void CGrenadeFrag::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity( &vel, &angVel );
	
	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
	UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
	if ( tr.startsolid )
	{
		if ( !m_inSolid )
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity( &vel, NULL );
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if ( tr.DidHit() )
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
		tr.m_pEnt->TakeDamage( info );

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;
		
		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity( &vel, &angVel );
	}
}*/


void CGrenadeFrag::Precache( void )
{
	PrecacheModel( GRENADE_MODEL );

	PrecacheScriptSound( "Grenade.Blip" );
	PrecacheScriptSound("Grenade.ImpactHard");

	PrecacheModel( "sprites/redglow1.vmt" );
	PrecacheModel( "sprites/bluelaser1.vmt" );

	BaseClass::Precache();
}

void CGrenadeFrag::SetTimer( float detonateDelay, float warnDelay )
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink( &CGrenadeFrag::DelayThink );
	SetNextThink( gpGlobals->curtime );

	CreateEffects();
}
void CGrenadeFrag::NadeTouch(CBaseEntity *pOther)
{
	Assert(pOther);
	CBaseEntity *pOwner = GetOwnerEntity();

	if (!pOwner)
		return;
	Msg("collided\n");
	if (!pOther->GetSolid() == SOLID_NONE)
	{
		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER))
			return;
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		if (pOther->GetMoveType() == MOVETYPE_NONE && (tr.surface.flags & SURF_SKY))
		{
			UTIL_Remove(this);
		}
		// See if we struck the world
		/*if (pOther->GetMoveType() == MOVETYPE_NONE && !(tr.surface.flags & SURF_SKY))
		{
			
		}*/

		if (pOwner->IsPlayer() && pOther->IsNPC())
		{
			Detonate();
		}
		if (pOwner->IsNPC() && pOther->IsPlayer())
		{
			Detonate();
		}
		Vector vecDir = GetAbsVelocity();
		float speed = VectorNormalize(vecDir);
		float hitDot = DotProduct(tr.plane.normal, -vecDir);
		iBounces += 0.5f;
		Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;
		SetAbsVelocity(vReflection * (speed * 1/iBounces));
		EmitSound("Grenade.ImpactHard");
		if (iBounces > 2.0f)
		{
			SetLocalAngularVelocity(QAngle(0, 0, 0));
		}
		if (speed < 1)
		{
			SetMoveType(MOVETYPE_NONE);
			SetAbsVelocity(Vector(0, 0, 0));
		}
		//Msg("bounce\n");
		SetGravity(1.0f);
		//return;
	}
}
/*void CGrenadeFrag::Detonate(void)
{

	SetTouch(NULL);

	SetModelName(NULL_STRING);//invisible
	AddSolidFlags(FSOLID_NOT_SOLID);
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), 0, 0,
		SF_ENVEXPLOSION_NOFIREBALL, 0.0f, this);
	RadiusDamage(CTakeDamageInfo(this, GetThrower(), 75, DMG_BLAST), GetAbsOrigin(), 128, CLASS_NONE, NULL);
	SetThink(&CBaseGrenade::SUB_Remove);
}*/
void CGrenadeFrag::Explode(trace_t *pTrace, int bitsDamageType)
{
#if !defined( CLIENT_DLL )

	SetModelName(NULL_STRING);//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents(vecAbsOrigin);

#if defined( TF_DLL )
	// Since this code only runs on the server, make sure it shows the tempents it creates.
	// This solves a problem with remote detonating the pipebombs (client wasn't seeing the explosion effect)
	CDisablePredictionFiltering disabler;
#endif

	if (pTrace->fraction != 1.0)
	{
		Vector vecNormal = pTrace->plane.normal;
		surfacedata_t *pdata = physprops->GetSurfaceData(pTrace->surface.surfaceProps);
		CPASFilter filter(vecAbsOrigin);

		te->Explosion(filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NOFIREBALL,
			m_DmgRadius,
			m_flDamage,
			&vecNormal,
			(char)pdata->game.material);
	}
	else
	{
		CPASFilter filter(vecAbsOrigin);
		te->Explosion(filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NOFIREBALL,
			m_DmgRadius,
			m_flDamage);
	}

#if !defined( CLIENT_DLL )
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);
#endif

	// Use the thrower's position as the reported position
	Vector vecReported = m_hThrower ? m_hThrower->GetAbsOrigin() : vec3_origin;

	CTakeDamageInfo info(this, m_hThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported);

	RadiusDamage(info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL);

	UTIL_DecalTrace(pTrace, "Scorch");

	EmitSound("BaseGrenade.Explode");

	SetThink(&CBaseGrenade::SUB_Remove);
	SetTouch(NULL);
	SetSolid(SOLID_NONE);

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);

#if HL2_EPISODIC
	// Because the grenade is zipped out of the world instantly, the EXPLOSION sound that it makes for
	// the AI is also immediately destroyed. For this reason, we now make the grenade entity inert and
	// throw it away in 1/10th of a second instead of right away. Removing the grenade instantly causes
	// intermittent bugs with env_microphones who are listening for explosions. They will 'randomly' not
	// hear explosion sounds when the grenade is removed and the SoundEnt thinks (and removes the sound)
	// before the env_microphone thinks and hears the sound.
	SetNextThink(gpGlobals->curtime + 0.1);
#else
	SetNextThink(gpGlobals->curtime);
#endif//HL2_EPISODIC

#if defined( HL2_DLL )
	CBasePlayer *pPlayer = ToBasePlayer(m_hThrower.Get());
	if (pPlayer)
	{
		gamestats->Event_WeaponHit(pPlayer, true, "weapon_frag", info);
	}
#endif

#endif
}
void CGrenadeFrag::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	SetThrower( pPhysGunUser );

#ifdef HL2MP
	SetTimer( FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP, FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP / 2);

	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
	m_bHasWarnedAI = true;
#else
	if( IsX360() )
	{
		// Give 'em a couple of seconds to aim and throw. 
		SetTimer( 2.0f, 1.0f);
		BlipSound();
		m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
	}
#endif

#ifdef HL2_EPISODIC
	SetPunted( true );
#endif

	BaseClass::OnPhysGunPickup( pPhysGunUser, reason );
}

void CGrenadeFrag::DelayThink() 
{
	if( gpGlobals->curtime > m_flDetonateTime )
	{
		Detonate();
		return;
	}

	if( !m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime )
	{
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this );
#endif
		m_bHasWarnedAI = true;
	}
	
	if( gpGlobals->curtime > m_flNextBlipTime )
	{
		BlipSound();
		
		if( m_bHasWarnedAI )
		{
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
		}
		else
		{
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;
		}
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeFrag::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CGrenadeFrag::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
extern int	g_interactionBarnacleVictimGrab; ///< usually declared in ai_interactions.h but no reason to haul all of that in here.
extern int g_interactionBarnacleVictimBite;
extern int g_interactionBarnacleVictimReleased;
bool CGrenadeFrag::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	// allow fragnades to be grabbed by barnacles. 
	if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		// give the grenade another five seconds seconds so the player can have the satisfaction of blowing up the barnacle with it
		float timer = m_flDetonateTime - gpGlobals->curtime + 5.0f;
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );

		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimBite )
	{
		// detonate the grenade immediately 
		SetTimer( 0, 0 );
		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimReleased )
	{
		// take the five seconds back off the timer.
		float timer = MAX(m_flDetonateTime - gpGlobals->curtime - 5.0f,0.0f);
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
		return true;
	}
	else
	{
		return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
	}
}
#endif

void CGrenadeFrag::InputSetTimer( inputdata_t &inputdata )
{
	SetTimer( inputdata.value.Float(), inputdata.value.Float() - FRAG_GRENADE_WARN_TIME );
}

/////
/////////////////////////////////////
/////////////////////////////////////
///////////

LINK_ENTITY_TO_CLASS(grenade_gas, CGrenadeGas);

BEGIN_DATADESC(CGrenadeGas)

// Fields
DEFINE_FIELD(m_pMainGlow, FIELD_EHANDLE),
DEFINE_FIELD(m_pGlowTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_flNextBlipTime, FIELD_TIME),
DEFINE_FIELD(m_inSolid, FIELD_BOOLEAN),

// Function Pointers
DEFINE_THINKFUNC(DelayThink),
DEFINE_ENTITYFUNC(NadeTouch),

// Inputs
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetTimer", InputSetTimer),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeGas::~CGrenadeGas(void)
{
}

void CGrenadeGas::Spawn(void)
{
	Precache();

	SetModel(GRENADE_MODEL);
	UTIL_SetSize(this, -Vector(10.0f, 10.0f, 10.0f), Vector(10.0f, 10.0f, 10.0f));
	if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
	{
		m_flDamage = sk_plr_dmg_fraggrenade.GetFloat();
		m_DmgRadius = sk_fraggrenade_radius.GetFloat();
	}
	else
	{
		m_flDamage = sk_npc_dmg_fraggrenade.GetFloat();
		m_DmgRadius = sk_fraggrenade_radius.GetFloat();
	}

	m_takedamage = DAMAGE_YES;
	m_iHealth = 1;

	SetSize(-Vector(4, 4, 4), Vector(4, 4, 4));
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	//CreateVPhysics();

	iBounces = 1;
	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;

	CreateEffects();
	SetTouch(&CGrenadeFrag::NadeTouch);

	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetSolid(SOLID_BBOX);

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeGas::OnRestore(void)
{
	// If we were primed and ready to detonate, put FX on us.

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeGas::CreateEffects(void)
{
	// Start up the eye glow
	m_pMainGlow = CSprite::SpriteCreate("sprites/redglow1.vmt", GetLocalOrigin(), false);

	int	nAttachment = LookupAttachment("fuse");

	if (m_pMainGlow != NULL)
	{
		m_pMainGlow->FollowEntity(this);
		m_pMainGlow->SetAttachment(this, nAttachment);
		m_pMainGlow->SetTransparency(kRenderGlow, 0, 255, 0, 200, kRenderFxNoDissipation);
		m_pMainGlow->SetScale(0.2f);
		m_pMainGlow->SetGlowProxySize(4.0f);
	}

	// Start up the eye trail
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/bluelaser1.vmt", GetLocalOrigin(), false);

	if (m_pGlowTrail != NULL)
	{
		m_pGlowTrail->FollowEntity(this);
		//m_pGlowTrail->SetAttachment( this, nAttachment );
		m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 255, 0, 255, kRenderFxNone);
		m_pGlowTrail->SetStartWidth(8.0f);
		m_pGlowTrail->SetEndWidth(1.0f);
		m_pGlowTrail->SetLifeTime(0.5f);
	}
}

/*bool CGrenadeGas::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);
	return true;
}*/

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked

/*void CGrenadeGas::VPhysicsUpdate(IPhysicsObject *pPhysics)
{
	BaseClass::VPhysicsUpdate(pPhysics);
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity(&vel, &angVel);

	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter(this, GetCollisionGroup(), COLLISION_GROUP_NONE);
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull(start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX | CONTENTS_MONSTER | CONTENTS_SOLID, &filter, &tr);
#else
	UTIL_TraceLine(start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX | CONTENTS_MONSTER | CONTENTS_SOLID, &filter, &tr);
#endif
	if (tr.startsolid)
	{
		if (!m_inSolid)
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity(&vel, NULL);
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if (tr.DidHit())
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info(this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH);
		tr.m_pEnt->TakeDamage(info);

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel, tr.plane.normal) + vel;

		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity(&vel, &angVel);
	}
}
*/

void CGrenadeGas::Precache(void)
{
	PrecacheModel(GRENADE_MODEL);

	PrecacheScriptSound("Grenade.Blip");
	PrecacheScriptSound("Grenade.ImpactHard");
	PrecacheScriptSound("Gasbomb.Explosion");

	PrecacheModel("sprites/redglow1.vmt");
	PrecacheModel("sprites/bluelaser1.vmt");
	PrecacheParticleSystem("hlr_gas_cloud_fog");

	BaseClass::Precache();
}

void CGrenadeGas::SetTimer(float detonateDelay, float warnDelay)
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink(&CGrenadeFrag::DelayThink);
	SetNextThink(gpGlobals->curtime);
}
void CGrenadeGas::NadeTouch(CBaseEntity *pOther)
{
	Assert(pOther);
	CBaseEntity *pOwner = GetOwnerEntity();

	if (!pOwner)
		return;
	Msg("collided\n");
	if (!pOther->GetSolid() == SOLID_NONE)
	{
		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER))
			return;
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		if (pOther->GetMoveType() == MOVETYPE_NONE && (tr.surface.flags & SURF_SKY))
		{
			UTIL_Remove(this);
		}
		// See if we struck the world
		/*if (pOther->GetMoveType() == MOVETYPE_NONE && !(tr.surface.flags & SURF_SKY))
		{

		}*/

		if (pOwner->IsPlayer() && pOther->IsNPC())
		{
			Detonate();
		}
		if (pOwner->IsNPC() && pOther->IsPlayer())
		{
			Detonate();
		}
		Vector vecDir = GetAbsVelocity();
		float speed = VectorNormalize(vecDir);
		float hitDot = DotProduct(tr.plane.normal, -vecDir);
		iBounces += 0.5f;
		Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;
		SetAbsVelocity(vReflection * (speed * 1 / iBounces));
		EmitSound("Grenade.ImpactHard");
		if (iBounces > 2.0f)
		{
			SetLocalAngularVelocity(QAngle(0, 0, 0));
		}
		if (speed < 1)
		{
			SetMoveType(MOVETYPE_NONE);
		}
		Msg("bounce\n");
		SetGravity(1.0f);
		//return;
	}

}
/*void CGrenadeFrag::Detonate(void)
{

SetTouch(NULL);

SetModelName(NULL_STRING);//invisible
AddSolidFlags(FSOLID_NOT_SOLID);
ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), 0, 0,
SF_ENVEXPLOSION_NOFIREBALL, 0.0f, this);

SetThink(&CBaseGrenade::SUB_Remove);
}*/
void CGrenadeGas::Explode(trace_t *pTrace, int bitsDamageType)
{
#if !defined( CLIENT_DLL )

	
	AddSolidFlags(FSOLID_NOT_SOLID);

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	Vector vecAbsOrigin = GetAbsOrigin();
	//int contents = UTIL_PointContents(vecAbsOrigin);

	if (pTrace->fraction != 1.0)
	{
		Vector vecNormal = pTrace->plane.normal;
		//surfacedata_t *pdata = physprops->GetSurfaceData(pTrace->surface.surfaceProps);
		CPASFilter filter(vecAbsOrigin);
		//RadiusDamage(CTakeDamageInfo(this, GetThrower(), 75, DMG_BLAST), GetAbsOrigin(), 128, CLASS_NONE, NULL);
		DispatchParticleEffect("hlr_gas_cloud_fog", GetAbsOrigin(), GetAbsAngles(), this);
		EmitSound("Gasbomb.Explosion");
		/*te->Explosion(filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NOFIREBALL,
			m_DmgRadius,
			m_flDamage,
			&vecNormal,
			(char)pdata->game.material);*/
	}
	else
	{
		CPASFilter filter(vecAbsOrigin);
		//RadiusDamage(CTakeDamageInfo(this, GetThrower(), 75, DMG_BLAST), GetAbsOrigin(), 128, CLASS_NONE, NULL);
		DispatchParticleEffect("hlr_gas_cloud_fog", GetAbsOrigin(), GetAbsAngles(), this);
		EmitSound("Gasbomb.Explosion");
	}

#if !defined( CLIENT_DLL )
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);
#endif

	// Use the thrower's position as the reported position
	Vector vecReported = m_hThrower ? m_hThrower->GetAbsOrigin() : vec3_origin;
	CTakeDamageInfo info(this, m_hThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported);

	//RadiusDamage(info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL);

	UTIL_DecalTrace(pTrace, "Scorch");

	EmitSound("BaseGrenade.Explode");
	CBaseEntity *pTemp = (CBaseEntity*)CreateEntityByName("hlr_gas_tempent");
	UTIL_SetOrigin(pTemp, this->GetAbsOrigin());
	pTemp->Spawn();
	SetThink(&CBaseGrenade::SUB_Remove);
	SetTouch(NULL);
	SetSolid(SOLID_NONE);

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);

	SetNextThink(gpGlobals->curtime + 0.1f);
	//SetModelName(NULL_STRING);//invisible
#if defined( HL2_DLL )
	CBasePlayer *pPlayer = ToBasePlayer(m_hThrower.Get());
	if (pPlayer)
	{
		gamestats->Event_WeaponHit(pPlayer, true, "weapon_frag", info);
	}
#endif
#endif

}

void CGrenadeGas::DelayThink()
{
	if (gpGlobals->curtime > m_flDetonateTime)
	{
		Detonate();
		return;
	}

	if (!m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime)
	{
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this);
#endif
		m_bHasWarnedAI = true;
	}

	if (gpGlobals->curtime > m_flNextBlipTime)
	{
		BlipSound();

		if (m_bHasWarnedAI)
		{
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
		}
		else
		{
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;
		}
	}

	SetNextThink(gpGlobals->curtime + 0.1);
}

void CGrenadeGas::SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity)
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if (pPhysicsObject)
	{
		pPhysicsObject->AddVelocity(&velocity, &angVelocity);
	}
}

int CGrenadeGas::OnTakeDamage(const CTakeDamageInfo &inputInfo)
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage(inputInfo);

	// Grenades only suffer blast damage and burn damage.
	if (!(inputInfo.GetDamageType() & (DMG_BLAST | DMG_BURN)))
		return 0;

	return BaseClass::OnTakeDamage(inputInfo);
}

void CGrenadeGas::InputSetTimer(inputdata_t &inputdata)
{
	SetTimer(inputdata.value.Float(), inputdata.value.Float() - FRAG_GRENADE_WARN_TIME);
}
/////GAS TEMPENT


LINK_ENTITY_TO_CLASS(hlr_gas_tempent, CGasTemp);

BEGIN_DATADESC(CGasTemp)

DEFINE_THINKFUNC(EmitDamage),

END_DATADESC()


void CGasTemp::Spawn(void)
{
	BaseClass::Spawn();
	SetThink(&CGasTemp::EmitDamage);
	SetNextThink(gpGlobals->curtime);

	m_fLastDamage = 0;
	m_fNextDamage = 0;
	m_fFinalDamage = (gpGlobals->curtime + 15.0f);
}
void CGasTemp::EmitDamage(void)
{
	m_fLastDamage = (gpGlobals->curtime);
	if (m_fLastDamage < m_fFinalDamage)
	{
		RadiusDamage(CTakeDamageInfo(this, GetOwnerEntity(), 5, DMG_NERVEGAS), GetAbsOrigin(), 200, CLASS_NONE, NULL);
	}
	else
	{
		UTIL_Remove(this);
		return;
	}
	SetNextThink(gpGlobals->curtime + 0.5f);
}

/////////////////////////////////////////
///// STICKY GRENADE  ///////////////////
/////////////////////////////////////////

#define STICKY_MODEL "models/weapons/w_stickygrenade.mdl"

LINK_ENTITY_TO_CLASS(grenade_sticky, CStickyGrenade);
BEGIN_DATADESC(CStickyGrenade)
DEFINE_PHYSPTR(m_pConstraint),
DEFINE_FIELD(m_hConstrainedEntity, FIELD_EHANDLE),
END_DATADESC()


void CStickyGrenade::Precache(void)
{
	PrecacheScriptSound("Weapon_Crossbow.Buildup");
	PrecacheModel(STICKY_MODEL);
	PrecacheModel("sprites/redglow1.vmt");
}

void CStickyGrenade::Spawn(void)
{
	Precache();
	SetModel(STICKY_MODEL);
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetSolid(SOLID_VPHYSICS);
	SetSolidFlags(FSOLID_NOT_STANDABLE | FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	SetGravity(1.0f);
	SetTouch(&CStickyGrenade::OnTouch);

}
void CStickyGrenade::OnTouch(CBaseEntity *pOther)
{
	if (!pOther)
		return;
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
			// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
				return;
	}
	if (GetOwnerEntity() && GetOwnerEntity() == pOther)
		return;
	if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
	{
		return;
	}

	trace_t tr; 
	tr = BaseClass::GetTouchTrace();
	if (pOther->IsSolid())
	{
		if ((pOther->GetMoveType() == MOVETYPE_NONE) && tr.surface.flags && SURF_SKY)
		{
			SUB_Remove();
		}

		if (pOther->IsWorld())
		{
			SetMoveType(MOVETYPE_NONE);
			StartTimer();
		}

		if (pOther->IsNPC())
		{
			SetMoveType(MOVETYPE_VPHYSICS);
			VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);
			Stick(pOther);
		}
		StartTimer();
	}
}

void CStickyGrenade::StartTimer(void)
{
	SetThink(&CStickyGrenade::Explode);
	SetNextThink(gpGlobals->curtime + 1.3f);
	EmitSound("Weapon_Crossbow.Buildup");
}

void CStickyGrenade::Explode(void)
{
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), 0, 0,
		SF_ENVEXPLOSION_NOFIREBALL, 0.0f, this);
	RadiusDamage(CTakeDamageInfo(this, GetThrower(), 150, DMG_BLAST), GetAbsOrigin(), 64, CLASS_NONE, NULL);
	RemoveDeferred();
}

bool CStickyGrenade::Stick(CBaseEntity *pOther)
{
	if (m_pConstraint != NULL)
	{
		// Should we destroy the constraint and make a new one at this point?
		Assert(0);
		return false;
	}

	if (pOther == NULL)
		return false;

	IPhysicsObject *pPhysObject = pOther->VPhysicsGetObject();
	if (pPhysObject == NULL)
		return false;

	IPhysicsObject *pMyPhysObject = VPhysicsGetObject();
	if (pPhysObject == NULL)
		return false;

	// Create the fixed constraint
	constraint_fixedparams_t fixedConstraint;
	fixedConstraint.Defaults();
	fixedConstraint.InitWithCurrentObjectState(pPhysObject, pMyPhysObject);

	IPhysicsConstraint *pConstraint = physenv->CreateFixedConstraint(pPhysObject, pMyPhysObject, NULL, fixedConstraint);
	if (pConstraint == NULL)
		return false;

	// Hold on to us
	m_pConstraint = pConstraint;
	pConstraint->SetGameData((void *)this);
	m_hConstrainedEntity = pOther->GetOwnerEntity();;

	// Disable collisions between the two ents
	PhysDisableObjectCollisions(pPhysObject, pMyPhysObject);

	return true;
}


//////SPAWN FUNCTIONS

CBaseGrenade *Fraggrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown-------------------------------------
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::Create( "npc_grenade_frag", position, angles, pOwner );
	//pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY);
	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetAbsVelocity(velocity);
	pGrenade->ApplyLocalAngularVelocityImpulse(angVelocity);
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->SetGravity(1.0f);
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );

	return pGrenade;
}
CBaseGrenade *Gasgrenade_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer)
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown-------------------------------------
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::Create("grenade_gas", position, angles, pOwner);
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY,MOVECOLLIDE_FLY_CUSTOM);
	pGrenade->SetTimer(timer, timer - FRAG_GRENADE_WARN_TIME);
	pGrenade->SetAbsVelocity(velocity);
	pGrenade->ApplyLocalAngularVelocityImpulse(angVelocity);
	pGrenade->SetThrower(ToBaseCombatCharacter(pOwner));
	pGrenade->SetGravity(1.0f);
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	//pGrenade->SetCombineSpawned(combineSpawned);
	pGrenade->SetElasticity(0.1f);

	return pGrenade;
}

CBaseGrenade *Stickynade_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner)
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown-------------------------------------
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::Create("grenade_sticky", position, angles, pOwner);
	pGrenade->SetAbsOrigin(position);
	pGrenade->SetAbsVelocity(velocity);
	pGrenade->ApplyLocalAngularVelocityImpulse(angVelocity);
	pGrenade->SetThrower(ToBaseCombatCharacter(pOwner));
	pGrenade->SetGravity(1.0f);
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;

	return pGrenade;
}

bool Fraggrenade_WasPunted( const CBaseEntity *pEntity )
{
	const CGrenadeFrag *pFrag = dynamic_cast<const CGrenadeFrag *>( pEntity );
	if ( pFrag )
	{
		return pFrag->WasPunted();
	}

	return false;
}

bool Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity )
{
	const CGrenadeFrag *pFrag = dynamic_cast<const CGrenadeFrag *>( pEntity );
	if ( pFrag )
	{
		return pFrag->IsCombineSpawned();
	}

	return false;
}