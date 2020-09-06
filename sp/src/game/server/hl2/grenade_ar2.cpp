//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "grenade_ar2.h"
#include "weapon_ar2.h"
#include "soundent.h"
#include "decals.h"
#include "shake.h"
#include "smoke_trail.h"
#include "ar2_explosion.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "world.h"
#include "SpriteTrail.h"
#include "explode.h"
#include "particle_system.h"
#include "particle_parse.h"

#ifdef PORTAL
	#include "portal_util_shared.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define AR2_GRENADE_MAX_DANGER_RADIUS	300

extern short	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud

// Moved to HL2_SharedGameRules because these are referenced by shared AmmoDef functions
extern ConVar    sk_plr_dmg_smg1_grenade;
extern ConVar    sk_npc_dmg_smg1_grenade;
extern ConVar    sk_max_smg1_grenade;

ConVar	  sk_smg1_grenade_radius		( "sk_smg1_grenade_radius","0");

ConVar g_CV_SmokeTrail("smoke_trail", "1", 0); // temporary dust explosion switch

BEGIN_DATADESC( CGrenadeAR2 )

	DEFINE_FIELD( m_hSmokeTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_fSpawnTime, FIELD_TIME ),
	DEFINE_FIELD( m_fDangerRadius, FIELD_FLOAT ),

	// Function pointers
	DEFINE_ENTITYFUNC( GrenadeAR2Touch ),
	DEFINE_THINKFUNC( GrenadeAR2Think ),
	DEFINE_THINKFUNC( Kill ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( grenade_ar2, CGrenadeAR2 );

void CGrenadeAR2::Spawn( void )
{
	Precache( );
	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	// Hits everything but debris
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	SetModel( "models/Weapons/ar2_grenade.mdl");
	UTIL_SetSize(this, Vector(-3, -3, -3), Vector(3, 3, 3));
//	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));

	SetUse( &CGrenadeAR2::DetonateUse );
	SetTouch( &CGrenadeAR2::GrenadeAR2Touch );
	SetThink( &CGrenadeAR2::GrenadeAR2Think );
	SetNextThink( gpGlobals->curtime + 0.1f );

	if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() )
	{
		m_flDamage = sk_plr_dmg_smg1_grenade.GetFloat();
	}
	else
	{
		m_flDamage = sk_npc_dmg_smg1_grenade.GetFloat();
	}

	m_DmgRadius		= sk_smg1_grenade_radius.GetFloat();
	m_takedamage	= DAMAGE_YES;
	m_bIsLive		= true;
	m_iHealth		= 1;

	SetGravity( UTIL_ScaleForGravity( 800 ) );	// use a lower gravity for grenades to make them easier to see
	SetFriction( 0.8 );
	SetSequence( 0 );

	m_fDangerRadius = 100;

	m_fSpawnTime = gpGlobals->curtime;

	// -------------
	// Smoke trail.
	// -------------
	if( g_CV_SmokeTrail.GetInt() && !IsXbox() )
	{
		m_hSmokeTrail = CSpriteTrail::SpriteTrailCreate("sprites/lgtning.vmt", GetLocalOrigin(), false);

		if (m_hSmokeTrail != NULL)
		{
			m_hSmokeTrail->FollowEntity(this);
			m_hSmokeTrail->SetTransparency(kRenderTransAdd, 255, 150, 150, 200, kRenderFxNone);
			m_hSmokeTrail->SetStartWidth(4.0f);
			m_hSmokeTrail->SetEndWidth(0.2f);
			m_hSmokeTrail->SetLifeTime(0.35f);
		}
	}
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("effect_name", "hlr_base_explosion2");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
	}
}

//-----------------------------------------------------------------------------
// Purpose:  The grenade has a slight delay before it goes live.  That way the
//			 person firing it can bounce it off a nearby wall.  However if it
//			 hits another character it blows up immediately
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CGrenadeAR2::GrenadeAR2Think( void )
{
	SetNextThink( gpGlobals->curtime + 0.05f );

	if (!m_bIsLive)
	{
		// Go live after a short delay
		if (m_fSpawnTime + MAX_AR2_NO_COLLIDE_TIME < gpGlobals->curtime)
		{
			m_bIsLive  = true;
		}
	}
	
	// If I just went solid and my velocity is zero, it means I'm resting on
	// the floor already when I went solid so blow up
	if (m_bIsLive)
	{
		if (GetAbsVelocity().Length() == 0.0 ||
			GetGroundEntity() != NULL )
		{
			Detonate();
		}
	}

	// The old way of making danger sounds would scare the crap out of EVERYONE between you and where the grenade
	// was going to hit. The radius of the danger sound now 'blossoms' over the grenade's lifetime, making it seem
	// dangerous to a larger area downrange than it does from where it was fired.
	if( m_fDangerRadius <= AR2_GRENADE_MAX_DANGER_RADIUS )
	{
		m_fDangerRadius += ( AR2_GRENADE_MAX_DANGER_RADIUS * 0.05 );
	}

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, m_fDangerRadius, 0.2, this, SOUNDENT_CHANNEL_REPEATED_DANGER );
}

void CGrenadeAR2::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );
}
void CGrenadeAR2::Kill(void)
{
	UTIL_Remove(this);
}

void CGrenadeAR2::GrenadeAR2Touch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() )
		return;

	// If I'm live go ahead and blow up
	if (m_bIsLive)
	{
		Detonate();
	}
	else
	{
		// If I'm not live, only blow up if I'm hitting an chacter that
		// is not the owner of the weapon
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( pOther );
		if (pBCC && GetThrower() != pBCC)
		{
			m_bIsLive = true;
			Detonate();
		}
	}
}

void CGrenadeAR2::Detonate(void)
{
	if (!m_bIsLive)
	{
		return;
	}

	m_bIsLive		= false;
	m_takedamage	= DAMAGE_NO;	

	if(m_hSmokeTrail)
	{
		UTIL_Remove(m_hSmokeTrail);
		m_hSmokeTrail = NULL;
	}
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), 0, 0,
		SF_ENVEXPLOSION_NOPARTICLES, 0.0f, this);
	DispatchParticleEffect("hlr_base_explosion2", GetAbsOrigin(), GetAbsAngles(), this);
	Vector vecForward = GetAbsVelocity();
	VectorNormalize(vecForward);
	trace_t		tr;
	UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + 60*vecForward, MASK_SHOT, 
		this, COLLISION_GROUP_NONE, &tr);


	if ((tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0))
	{
		// non-world needs smaller decals
		if( tr.m_pEnt && !tr.m_pEnt->IsNPC() )
		{
			UTIL_DecalTrace( &tr, "SmallScorch" );
		}
	}
	else
	{
		UTIL_DecalTrace( &tr, "Scorch" );
	}

	UTIL_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 0.5, 500, SHAKE_START );
	

	RadiusDamage(CTakeDamageInfo(this, GetThrower(), 150, DMG_BLAST), GetAbsOrigin(), 32, CLASS_PLAYER, NULL);
	SetThink(&CGrenadeAR2::Kill);
	SetNextThink(gpGlobals->curtime + 0.01f);
}

void CGrenadeAR2::Precache( void )
{
	PrecacheModel("models/Weapons/ar2_grenade.mdl"); 
	PrecacheModel("sprites/lgtning.vmt");
	PrecacheParticleSystem("hlr_base_explosion2");
}


CGrenadeAR2::CGrenadeAR2(void)
{
	m_hSmokeTrail  = NULL;
}
