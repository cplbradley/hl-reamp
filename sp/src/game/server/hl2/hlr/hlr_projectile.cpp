#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "explode.h"
#include "game.h"
#include "vstdlib/random.h"
#include "hl2_shareddefs.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"
#include "hlr_projectile.h"
#include "particle_parse.h"

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(hlr_vortprojectile, CHLRVortProjectile); //ooh i'm being assigned a class
BEGIN_DATADESC(CHLRVortProjectile) //let's store some data
// Function Pointers
DEFINE_FUNCTION(Touch), //i can touch stuff, i need outputs
END_DATADESC() //all done!
CHLRVortProjectile *CHLRVortProjectile::Create(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner) //dunno why this is here honestly it's useless and doesn't actually work how i want it to
{
	// Create a new entity with CHLRVortProjectile data
	CHLRVortProjectile *pVort = (CHLRVortProjectile *)CreateEntityByName("hlr_vortprojectile");
	UTIL_SetOrigin(pVort, vecOrigin);
	pVort->SetAbsAngles(angAngles);
	pVort->Spawn();
	pVort->SetOwnerEntity(pentOwner);

	return pVort;
}
bool CHLRVortProjectile::CreateVPhysics(void) //this is important i guess
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_SOLID, false);

	return true;
}
unsigned int CHLRVortProjectile::PhysicsSolidMaskForEntity() const //as is this
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CHLRVortProjectile::Spawn(void) //i have been born
{
	Precache(); //load the data
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM); //i can fly!
	UTIL_SetSize(this, -Vector(10.0f, 10.0f, 1.0f), Vector(10.0f, 10.0f, 10.0f)); //this is how big my bounding box is
	SetSolid(SOLID_BBOX); //check out my bounding box
	SetModel("models/spitball_small.mdl"); //this is what i look like 
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE); //i'm a projectile, don't hit me!
	DrawSprite(); //i'm a glowing bouncy boy
	DispatchParticleEffect("vortigaunt_projectile_core",PATTACH_ABSORIGIN_FOLLOW, this, "root", false); //i'm super gooey
	SetTouch(&CHLRVortProjectile::Touch); //i can touch stuff

}
void CHLRVortProjectile::Precache(void) //let's gather up some supplies
{
	PrecacheModel("sprites/greenglow1.vmt");//need this
	PrecacheModel("models/spitball_small.mdl"); //need this
	PrecacheParticleSystem("vortigaunt_projectile_core"); //need this
	PrecacheParticleSystem("vortigaunt_burst_core"); //definitely need this
}
bool CHLRVortProjectile::DrawSprite(void) //this is what i glow like
{
	m_pMainGlow = CSprite::SpriteCreate("sprites/greenglow1.vmt", GetLocalOrigin(), false); //i look like this
	if (m_pMainGlow != NULL)
	{
		m_pMainGlow->FollowEntity(this); //my glow sticks to me 
		m_pMainGlow->SetTransparency(kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation); //i'm a certain color
		m_pMainGlow->SetScale(1.5f); //this is how bing i am 
		m_pMainGlow->SetGlowProxySize(4.0f); //
	}
	return true;
}
void CHLRVortProjectile::Touch(CBaseEntity *pOther) //i touched something
{
	if (pOther->IsSolid()) //is what i touched solid?
	{
		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER)) //is it a trigger?
		{
			return; //carry on like nothing happened
		}
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
		{
			return; //i don't collide with other projectiles
		}

		if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
		{
			trace_t	tr, tr2; //initialize info
			tr = BaseClass::GetTouchTrace(); //trace touch
			Vector	vecNormalizedVel = GetAbsVelocity(); //this is how fast i'm going!

			ClearMultiDamage(); //gotta reset everything before i zap ya
			VectorNormalize(vecNormalizedVel); //i need to have a clear shot 
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), 15, DMG_SHOCK); //i'm loading my damage data
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel(); //how fair do i feel?
			dmgInfo.SetDamagePosition(tr.endpos); //gotcha, there's my target 
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr); //you're getting zapped!
			//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this);
			ApplyMultiDamage(); //zap!
		}
		SetMoveType(MOVETYPE_NONE); //stop moving completely
		SetSolid(SOLID_NONE); //get rid of my collision model
		SetSolidFlags(FSOLID_NOT_SOLID); //alert the game i'm not solid anymore
		DispatchParticleEffect("vortigaunt_burst_core", GetAbsOrigin(), GetAbsAngles(), this); //poof!
		SetThink(&CHLRVortProjectile::KillIt); //i'm gonna kill myself
		SetTouch(NULL); //i can't touch anything anymore
		SetNextThink(gpGlobals->curtime + 0.01f); //i'm killing myself in 0.01 seconds
	}
}
void CHLRVortProjectile::KillIt(void)
{
	SetThink(NULL); //i'm done thinking now 
	StopParticleEffects(this); //stop my poofs
	m_pGlowTrail == NULL; //no more trail
	m_pMainGlow == NULL; //no more sprite
	UTIL_Remove(this); //i have now killed myself
	
}
LINK_ENTITY_TO_CLASS(hlr_pistolprojectile, CHLRPistolProjectile);
BEGIN_DATADESC(CHLRPistolProjectile)
// Function Pointers
DEFINE_FUNCTION(Touch),
END_DATADESC()
CHLRPistolProjectile *CHLRPistolProjectile::Create(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner)
{
	// Create a new entity with CHLRVortProjectile data
	CHLRPistolProjectile *pPew = (CHLRPistolProjectile *)CreateEntityByName("hlr_pistolprojectile");
	UTIL_SetOrigin(pPew, vecOrigin);
	pPew->SetAbsAngles(angAngles);
	pPew->SetOwnerEntity(pentOwner);
	pPew->Spawn();


	return pPew;
}
bool CHLRPistolProjectile::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_SOLID, false);

	return true;
}
unsigned int CHLRPistolProjectile::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CHLRPistolProjectile::Spawn(void)
{
	Precache();
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(10.0f, 10.0f, 1.0f), Vector(10.0f, 10.0f, 10.0f));
	SetSolid(SOLID_BBOX);
	SetModel("models/spitball_small.mdl");
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	//SetSolidFlags(FSOLID_TRIGGER);
	SetRenderColor(255, 135, 115);
	CreateTrail();
	DrawSprite();
	SetNextThink(gpGlobals->curtime + 0.2f);
	SetThink(&CHLRPistolProjectile::EnableTouch);
	

}
void CHLRPistolProjectile::EnableTouch(void)
{
	SetTouch(&CHLRPistolProjectile::Touch);
}
void CHLRPistolProjectile::Precache(void)
{
	PrecacheModel("sprites/smoke.vmt");
	PrecacheModel("sprites/redglow2.vmt");
	PrecacheModel("models/spitball_small.mdl");
	PrecacheParticleSystem("pistol_core");
	BaseClass::Precache();
}
bool CHLRPistolProjectile::CreateTrail(void)
{
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/smoke.vmt", GetLocalOrigin(), false);
	if (m_pGlowTrail != NULL)
	{
		m_pGlowTrail->FollowEntity(this);
		m_pGlowTrail->SetStartWidth(6.0f);
		m_pGlowTrail->SetEndWidth(0.2f);
		m_pGlowTrail->SetLifeTime(0.05f);
		m_pGlowTrail->SetTransparency(kRenderTransAdd, 255, 45, 45, 100, kRenderFxNone);
	}
	return true;
}
bool CHLRPistolProjectile::DrawSprite(void)
{
	m_pMainGlow = CSprite::SpriteCreate("sprites/redglow2.vmt", GetLocalOrigin(), false);
	if (m_pMainGlow != NULL)
	{
		m_pMainGlow->FollowEntity(this);
		m_pMainGlow->SetTransparency(kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation);
		m_pMainGlow->SetScale(0.5f);
		m_pMainGlow->SetGlowProxySize(4.0f);
	}
	return true;
}
void CHLRPistolProjectile::Touch(CBaseEntity *pOther) //i touched something
{
	if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsPlayer())
	{
		return;
	}
	if (pOther->IsSolid()) //is what i touched solid?
	{
		if (pOther->IsSolidFlagSet(FSOLID_TRIGGER)) //is it a trigger?
		{
			return; //carry on like nothing happened
		}
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
		{
			return;
		}
		//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this); //poof effect!
		
		if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
		{
			trace_t	tr, tr2; //initialize info
			tr = BaseClass::GetTouchTrace(); //trace touch
			Vector	vecNormalizedVel = GetAbsVelocity();
			ClearMultiDamage();
			VectorNormalize(vecNormalizedVel);
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), 7, DMG_SHOCK);
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			//CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 1.0f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
			//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this);
			ApplyMultiDamage();
		}
		SetSolid(SOLID_NONE);
		SetSolidFlags(FSOLID_NOT_SOLID);
		SetMoveType(MOVETYPE_NONE);
		DispatchParticleEffect("pistol_core", GetAbsOrigin(), GetAbsAngles(), this);
		const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
		trace_t *pNewTrace = const_cast<trace_t*>(pTrace);
		UTIL_DecalTrace(pNewTrace, "SmallScorch");
		SetThink(&CHLRPistolProjectile::KillIt); //schedule remove command
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime + 0.01f); //execute remove command after 0.01 seconds
	}
}
void CHLRPistolProjectile::KillIt(void)
{
	SetThink(NULL);
	m_pGlowTrail == NULL;
	m_pMainGlow == NULL;
	UTIL_Remove(this); //remove it
}

LINK_ENTITY_TO_CLASS(hlr_fireball, CHLRFireball);
BEGIN_DATADESC(CHLRFireball)
// Function Pointers
DEFINE_FUNCTION(Touch),
END_DATADESC()


bool CHLRFireball::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_SOLID, false);

	return true;
}
void CHLRFireball::Spawn(void)
{
	Precache();
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetGravity(1.0);
	SetFriction(0.8f);
	UTIL_SetSize(this, -Vector(32.0f, 32.0f, 32.0f), Vector(32.0f, 32.0f, 32.0f));
	SetSolid(SOLID_BBOX);
	SetModel("models/spitball_small.mdl");

	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetTouch(&CHLRFireball::Touch);
	SetNextThink(gpGlobals->curtime + 0.1f);
	DispatchParticleEffect("antlionwarrior_fireball_core", PATTACH_ABSORIGIN_FOLLOW, this, "root", false);

}
void CHLRFireball::Precache(void)
{
	PrecacheModel("models/spitball_small.mdl");
	PrecacheParticleSystem("antlionwarrior_fireball_core");
	UTIL_PrecacheOther("hlr_fireballtemp");
}
void CHLRFireball::Touch(CBaseEntity *pOther)
{
	if (pOther == NULL)
		return;
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	// Don't hit other spit
	if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
		return;
	if (GetOwnerEntity() && GetOwnerEntity() == pOther)
		return;
	/*if (pOther->IsPlayer() == true)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		pPlayer->IgniteLifetime(2.5);
	}*/
	Detonate();
}
void CHLRFireball::Detonate(void)
{
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	trace_t *pNewTrace = const_cast<trace_t*>( pTrace );
	UTIL_DecalTrace( pNewTrace, "Scorch" );
	CBaseEntity *pOwner = GetOwnerEntity();
	QAngle angCent = QAngle(0, 0, 0);
	SetAbsAngles(angCent);
	RadiusDamage(CTakeDamageInfo(this, pOwner, 40, DMG_BURN), GetAbsOrigin(), 128, CLASS_ANTLION, NULL);
	CHLRFireballTemp *pTemp = (CHLRFireballTemp*)CreateEntityByName("hlr_fireballtemp");
	pTemp->SetAbsOrigin(this->GetAbsOrigin());
	pTemp->SetAbsAngles(this->GetAbsAngles());
	pTemp->Spawn();
	UTIL_Remove(this);

}
LINK_ENTITY_TO_CLASS(hlr_fireballtemp, CHLRFireballTemp);
void CHLRFireballTemp::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 32.0f), Vector(64.0f, 64.0f, 64.0f));
	SetModel("models/props/null.mdl");
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	DispatchParticleEffect("antlionwarrior_fireball_groundfire_sheet", GetAbsOrigin(), GetAbsAngles(), this);
	SetNextThink(gpGlobals->curtime + 8.0f);
	SetThink(&CHLRFireballTemp::Kill);
}
void CHLRFireballTemp::Precache(void)
{
	PrecacheParticleSystem("antlionwarrior_fireball_groundfire_sheet");
	PrecacheModel("models/props/null.mdl");
}
void CHLRFireballTemp::Kill(void)
{
	StopParticleEffects(this);
	UTIL_Remove(this);
}