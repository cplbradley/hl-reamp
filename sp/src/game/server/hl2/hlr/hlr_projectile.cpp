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
#include "smoke_trail.h"
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
DEFINE_FUNCTION(Touch),
DEFINE_THINKFUNC(KillIt),//i can touch stuff, i need outputs
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
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
			return;
	}

		if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
		{
			trace_t	tr; //initialize info
			tr = BaseClass::GetTouchTrace(); //trace touch
			Vector	vecNormalizedVel = GetAbsVelocity(); //this is how fast i'm going!

			ClearMultiDamage(); //gotta reset everything before i zap ya
			VectorNormalize(vecNormalizedVel); //convert the vector into a direction
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), 15, DMG_SHOCK); //i'm setting up some damage info
			dmgInfo.AdjustPlayerDamageTakenForSkillLevel(); //how fair do i feel?
			dmgInfo.SetDamagePosition(tr.endpos); //gotta attack at the exact point of impact
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
void CHLRVortProjectile::KillIt(void)
{
	SetThink(NULL); //i'm done thinking now 
	StopParticleEffects(this); //stop my poofs
	m_pGlowTrail == NULL; //no more trail
	m_pMainGlow == NULL; //no more sprite
	RemoveDeferred(); //i have now killed myself
	
}
LINK_ENTITY_TO_CLASS(hlr_pistolprojectile, CHLRPistolProjectile);
BEGIN_DATADESC(CHLRPistolProjectile)
// Function Pointers
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(CreateTrail),
DEFINE_FUNCTION(DrawSprite),
DEFINE_THINKFUNC(KillIt),
DEFINE_THINKFUNC(TargetTrackThink),
DEFINE_THINKFUNC(EnableTouch),
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
	SetSolid(SOLID_NONE);
	SetModel("models/spitball_small.mdl");
	//SetSolidFlags(FSOLID_TRIGGER);
	SetRenderColor(255, 135, 115);
	CreateTrail();
	DrawSprite();
	SetThink(&CHLRPistolProjectile::EnableTouch);
	SetNextThink(gpGlobals->curtime + 0.01f);
}
void CHLRPistolProjectile::Precache(void)
{
	PrecacheModel("sprites/smoke.vmt");
	PrecacheModel("sprites/redglow2.vmt");
	PrecacheModel("models/spitball_small.mdl");
	PrecacheParticleSystem("pistol_core");
	BaseClass::Precache();
}

void CHLRPistolProjectile::EnableTouch(void)
{
	SetTouch(&CHLRPistolProjectile::Touch);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID || FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
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
void CHLRPistolProjectile::SetTargetPos(const Vector &vecTargetPos, const float &fVelocity)
{
	vecTarget = vecTargetPos;
	flVelocity = fVelocity;
	SetThink(&CHLRPistolProjectile::TargetTrackThink);
	SetNextThink(gpGlobals->curtime);

}
void CHLRPistolProjectile::TargetTrackThink(void)
{
	Vector vecDir = (vecTarget - GetAbsOrigin());
	VectorNormalize(vecDir);
	SetAbsVelocity(vecDir * flVelocity);
	//SetNextThink(gpGlobals->curtime + 0.01f);
}
void CHLRPistolProjectile::Touch(CBaseEntity *pOther) //i touched something
{
	if (GetOwnerEntity() && GetOwnerEntity() == pOther)
	{
		return;
	}
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
			return;
	}
		if (pOther->m_takedamage != DAMAGE_NO) //can what i hit take damage?
		{
			SetThink(NULL);
			trace_t	tr; //initialize a trace
			tr = BaseClass::GetTouchTrace(); //assing the trace information from the touch
			Vector	vecNormalizedVel = GetAbsVelocity(); //save the current velocity for use later
			ClearMultiDamage(); //clear out any potential unwanted damage calls
			VectorNormalize(vecNormalizedVel); //convert the velocity into a direction
			CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), 12, DMG_CLUB); //initialize the damage information
			if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
			{
				dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			}
			else
			{
				dmgInfo.AdjustPlayerDamageTakenForSkillLevel();
			}
			dmgInfo.SetDamagePosition(tr.endpos); //set the damage point at the end of the trace
			//CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 1.0f);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr); //set off the effects
			//DispatchParticleEffect("smg_plasmaball_core", GetAbsOrigin(), GetAbsAngles(), this);
			ApplyMultiDamage();
		}
		SetSolid(SOLID_NONE); //no longer solid at all
		SetSolidFlags(FSOLID_NOT_SOLID); //same thing
		SetMoveType(MOVETYPE_NONE); //stop moving
		DispatchParticleEffect("pistol_core", GetAbsOrigin(), GetAbsAngles(), this);
		const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
		trace_t *pNewTrace = const_cast<trace_t*>(pTrace);
		UTIL_DecalTrace(pNewTrace, "FadingScorch");
		SetThink(&CHLRPistolProjectile::KillIt); //schedule remove command
		SetTouch(NULL);
		RemoveDeferred();
		SetNextThink(gpGlobals->curtime + 0.01f); //execute remove command after 0.01 seconds
}
void CHLRPistolProjectile::KillIt(void)
{
	UTIL_RemoveImmediate(this);
}

LINK_ENTITY_TO_CLASS(hlr_fireball, CHLRFireball);
BEGIN_DATADESC(CHLRFireball)
// Function Pointers
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(Detonate),
DEFINE_THINKFUNC(Kill),
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
	CFireTrail *pFireTrail = CFireTrail::CreateFireTrail();

	if (pFireTrail == NULL)
		return;

	pFireTrail->FollowEntity(this, "");
	pFireTrail->SetParent(this, 0);
	pFireTrail->SetLocalOrigin(vec3_origin);
	pFireTrail->SetMoveType(MOVETYPE_NONE);
	pFireTrail->SetLifetime(-1);

}
void CHLRFireball::Precache(void)
{
	PrecacheModel("models/spitball_small.mdl");
	PrecacheParticleSystem("antlionwarrior_fireball_core");
	PrecacheParticleSystem("hlr_base_explosion2");
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
	SetTouch(NULL);
	SetMoveType(MOVETYPE_NONE);
	SetSolidFlags(FSOLID_NOT_SOLID);
	SetSolid(SOLID_NONE);
	Detonate();
	return;
}
void CHLRFireball::Detonate(void)
{
	DispatchParticleEffect("hlr_base_explosion2", GetAbsOrigin(), GetAbsAngles(), this);
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	trace_t *pNewTrace = const_cast<trace_t*>( pTrace );
	UTIL_DecalTrace( pNewTrace, "Scorch" );
	CBaseEntity *pOwner = GetOwnerEntity();
	RadiusDamage(CTakeDamageInfo(this, pOwner, 40, DMG_BURN), GetAbsOrigin(), 128, CLASS_ANTLION, NULL);

	trace_t tr;
	Vector Startpos = GetAbsOrigin();
	Vector Endpos = Startpos + (Vector(0, 0, -1) * MAX_TRACE_LENGTH);
	UTIL_TraceLine(Startpos, Endpos, MASK_PLAYERSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	CHLRFireballTemp *pTemp = (CHLRFireballTemp*)CreateEntityByName("hlr_fireballtemp");
	pTemp->SetAbsOrigin(tr.endpos);
	pTemp->SetAbsAngles(this->GetAbsAngles());
	pTemp->Spawn();
	SetMoveType(MOVETYPE_NONE);

	SetThink(&CHLRFireball::Kill);
	SetNextThink(gpGlobals->curtime + 0.01f);
}
void CHLRFireball::Kill(void)
{
	RemoveDeferred();
}
LINK_ENTITY_TO_CLASS(hlr_fireballtemp, CHLRFireballTemp);
BEGIN_DATADESC(CHLRFireballTemp)
DEFINE_FUNCTION(Touch),
DEFINE_THINKFUNC(Kill),
END_DATADESC()
void CHLRFireballTemp::Spawn(void)
{

	Precache();
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 32.0f), Vector(64.0f, 64.0f, 64.0f));
	SetModel("models/props/null.mdl");
	SetSolid(SOLID_BBOX);
	SetSolidFlags(FSOLID_VOLUME_CONTENTS && FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_NONE);
	DispatchParticleEffect("antlionwarrior_fireball_groundfire_sheet", GetAbsOrigin(), GetAbsAngles(), this);
	SetNextThink(gpGlobals->curtime + 8.0f);
	SetThink(&CHLRFireballTemp::Kill);
	m_flNextDamage = gpGlobals->curtime;


}
void CHLRFireballTemp::Precache(void)
{
	PrecacheParticleSystem("antlionwarrior_fireball_groundfire_sheet");
	PrecacheModel("models/props/null.mdl");
}
void CHLRFireballTemp::Touch(CBaseEntity *pOther)
{
	if (pOther && pOther->m_takedamage && (m_flNextDamage < gpGlobals->curtime))
	{
		pOther->TakeDamage(CTakeDamageInfo(this, this, 1, (DMG_BULLET | DMG_BURN)));
		m_flNextDamage = gpGlobals->curtime + 0.5f;
	}
}
void CHLRFireballTemp::Kill(void)
{
	StopParticleEffects(this);
	UTIL_Remove(this);
}


LINK_ENTITY_TO_CLASS(hlr_scannerprojectile, CHLRScannerProjectile);
BEGIN_DATADESC(CHLRScannerProjectile)
// Function Pointers
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(DispatchEffects),
DEFINE_THINKFUNC(Kill),
END_DATADESC()
void CHLRScannerProjectile::Spawn(void)
{
	Precache();
	SetModel("models/spitball_large.mdl");
	SetMoveType(MOVETYPE_FLY);
	SetSolid(SOLID_BBOX);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetTouch(&CHLRScannerProjectile::Touch);
	DispatchEffects();
}
void CHLRScannerProjectile::DispatchEffects(void)
{
	DispatchParticleEffect("scanner_projectile_core", PATTACH_ABSORIGIN_FOLLOW, this, "root", false);
}
void CHLRScannerProjectile::Precache(void)
{
	PrecacheModel("models/spitball_large.mdl");
	PrecacheParticleSystem("scanner_projectile_core");
	PrecacheParticleSystem("scanner_projectile_poof");
}
void CHLRScannerProjectile::Touch(CBaseEntity *pOther)
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

	SetMoveType(MOVETYPE_NONE);
	RadiusDamage(CTakeDamageInfo(this, this->GetOwnerEntity(), 30, DMG_ENERGYBEAM, 0), GetAbsOrigin(), 64, CLASS_NONE, NULL);
	DispatchParticleEffect("scanner_projectile_poof", GetAbsOrigin(), GetAbsAngles(), this);
	SetThink(&CHLRScannerProjectile::Kill);
	SetNextThink(gpGlobals->curtime + 0.1f);

}
void CHLRScannerProjectile::Kill(void)
{
	RemoveDeferred();
}


LINK_ENTITY_TO_CLASS(hlr_mechubusmissile, CHLRMechubusMissile);
BEGIN_DATADESC(CHLRMechubusMissile)
DEFINE_FUNCTION(Touch),
DEFINE_FUNCTION(Explode),
DEFINE_FUNCTION(DispatchEffects),
DEFINE_THINKFUNC(EnableTouch),
END_DATADESC()

void CHLRMechubusMissile::Spawn(void)
{
	Precache();
	SetModel("models/spitball_large.mdl");
	//SetModelName(NULL_STRING);
	SetSolid(SOLID_NONE);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	DispatchEffects();
	SetThink(&CHLRMechubusMissile::EnableTouch);
	SetNextThink(gpGlobals->curtime + 0.01f); //to avoid accidental self-collisions, we delay the ability to contact by 0.1 seconds
}

void CHLRMechubusMissile::Precache(void)
{
	PrecacheModel("models/spitball_large.mdl");
	PrecacheParticleSystem("mechubus_missile_core");
}
bool CHLRMechubusMissile::DispatchEffects(void)
{
	DispatchParticleEffect("mechubus_missile_core", PATTACH_ABSORIGIN_FOLLOW, this, "root", false);
	SetRenderColor(255, 128, 0);
	return true;
}

void CHLRMechubusMissile::EnableTouch(void)
{
	SetSolid(SOLID_BBOX);
	SetSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	
	SetTouch(&CHLRMechubusMissile::Touch);
}
void CHLRMechubusMissile::Touch(CBaseEntity *pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
			return;
	}

	if (GetOwnerEntity() && GetOwnerEntity() == pOther)
		return;

	string_t str = pOther->m_iClassname;
	DevMsg("Hit Object %s\n", str);
	SetTouch(NULL);
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	SetSolidFlags(FSOLID_NOT_SOLID);
	
	Explode();
}

void CHLRMechubusMissile::Explode(void)
{
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), 20, 120,
		SF_ENVEXPLOSION_NOFIREBALL, 0.0f, this);
	RemoveDeferred();
}