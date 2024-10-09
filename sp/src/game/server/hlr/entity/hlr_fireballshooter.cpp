#include "cbase.h"
#include "baseanimating.h"
#include "player.h"
#include "particle_parse.h"


#include "tier0/memdbgon.h"


#define SF_FIREBALL_SHOOTER_BREAK_ON_DMG 0x0001
#define SF_FIREBALL_SHOOTER_SILENT 0x0002
#define SF_FIREBALL_NO_DLIGHT 0x0004

enum FireballType {
	FB_SMALL,
	FB_MED,
	FB_LARGE
};

class CShooterFireball : public CBaseAnimating
{
	DECLARE_CLASS(CShooterFireball, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	void Precache();
	void Spawn();
	void FireballTouch(CBaseEntity* pOther);
	void StartDeath();
	void Die();
	void SetFireballType();

	virtual int UpdateTransmitState() { return SetTransmitState(FL_EDICT_ALWAYS); }

	FireballType iFireballType;
	float fDamage;
	bool bDestroyOnDamage;

	CNetworkVar(bool, bUseDLight);

	static CShooterFireball* CreateFireball(const Vector& vecOrigin, FireballType type, float damage, bool destroyondmg, bool dlight);
};

IMPLEMENT_SERVERCLASS_ST(CShooterFireball, DT_Fireball)
SendPropBool(SENDINFO(bUseDLight)),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(fireball, CShooterFireball);

BEGIN_DATADESC(CShooterFireball)
	DEFINE_FIELD(iFireballType,FIELD_INTEGER),
	DEFINE_FIELD(fDamage,FIELD_FLOAT),
	DEFINE_FIELD(bDestroyOnDamage,FIELD_BOOLEAN),
	DEFINE_FUNCTION(FireballTouch),
	DEFINE_FIELD(bUseDLight,FIELD_BOOLEAN),
END_DATADESC()

CShooterFireball* CShooterFireball::CreateFireball(const Vector& vecOrigin, FireballType type, float damage, bool destroyondmg, bool dlight)
{
	CShooterFireball* pBall = (CShooterFireball*)CreateEntityByName("fireball");
	UTIL_SetOrigin(pBall, vecOrigin);
	pBall->iFireballType = type;
	pBall->fDamage = damage;
	pBall->bDestroyOnDamage = destroyondmg;
	pBall->bUseDLight = dlight;
	pBall->Spawn();

	return pBall;

}

void CShooterFireball::Spawn()
{
	Precache();
	SetModel("models/spitball_small.mdl");
	SetFireballType();
	SetMoveType(MOVETYPE_FLY,MOVECOLLIDE_FLY_CUSTOM);
	SetSolid(SOLID_CUSTOM);
	SetSolidFlags(FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetTouch(&CShooterFireball::FireballTouch);
}

void CShooterFireball::Precache()
{
	PrecacheModel("models/spitball_small.mdl");
	PrecacheParticleSystem("fireball_shooter_large");
	PrecacheParticleSystem("fireball_shooter_medium");
	PrecacheParticleSystem("mechubus_missile_core");
}
void CShooterFireball::SetFireballType()
{
	switch (iFireballType)
	{
	case FB_LARGE:
		UTIL_SetSize(this, Vector(-48, -48, -48), Vector(48, 48, 48));
		DispatchParticleEffect("fireball_shooter_large", PATTACH_ABSORIGIN_FOLLOW, this);
		break;
	case FB_MED:
		UTIL_SetSize(this, Vector(-24, -24, -24), Vector(24, 24, 24));
		DispatchParticleEffect("fireball_shooter_medium", PATTACH_ABSORIGIN_FOLLOW, this);
		break;
	case FB_SMALL:
	default:
		UTIL_SetSize(this, Vector(-12, -12, -12), Vector(12, 12, 12));
		DispatchParticleEffect("mechubus_missile_core", PATTACH_ABSORIGIN_FOLLOW, this);
		break;
	}
}

void CShooterFireball::FireballTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	

	if (pOther->IsSolidFlagSet(FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE)
			return;
	};

	if (pOther->m_takedamage != DAMAGE_NO)
	{
		pOther->TakeDamage(CTakeDamageInfo(this, this, fDamage, DMG_BURN));
	}

	if (pOther->IsWorld() || bDestroyOnDamage)
		StartDeath();
}

void CShooterFireball::StartDeath()
{
	SetTouch(NULL);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);
	SetThink(&CShooterFireball::Die);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CShooterFireball::Die()
{
	StopParticleEffects(this);
	UTIL_Remove(this);
}


class CFireballShooter : public CBaseEntity
{
	DECLARE_CLASS(CFireballShooter, CBaseEntity);
public:
	void Spawn();
	void Precache();
	void ShootFireball();
	

	void InputShootFireball(inputdata_t& inputdata);
	void InputSetFireballSpeed(inputdata_t& inputdata);
	void InputSetFireballType(inputdata_t& inputdata);
	void InputSetFireballDamage(inputdata_t& inputdata);

	COutputEvent m_OnShoot;

	DECLARE_DATADESC();

private:

	FireballType iFireballType;
	float fDamage;
	float fSpeed;
	const char* szTargetName;
	string_t m_iszSound;

	bool m_bBreakOnDmg;
	bool m_bSilent;

	bool bUseDLight;
};

LINK_ENTITY_TO_CLASS(fireball_shooter, CFireballShooter);

BEGIN_DATADESC(CFireballShooter)
	DEFINE_KEYFIELD(fDamage, FIELD_FLOAT, "damage"),
	DEFINE_KEYFIELD(fSpeed, FIELD_FLOAT, "speed"),
	DEFINE_KEYFIELD(iFireballType, FIELD_INTEGER, "type"),
	DEFINE_KEYFIELD(szTargetName, FIELD_STRING, "target"),
	DEFINE_KEYFIELD(m_iszSound, FIELD_STRING, "shootsound"),

	DEFINE_FIELD(m_bBreakOnDmg,FIELD_BOOLEAN),
	DEFINE_FIELD(bUseDLight,FIELD_BOOLEAN),
	
	DEFINE_INPUTFUNC(FIELD_VOID,"ShootFireball",InputShootFireball),
	DEFINE_INPUTFUNC(FIELD_FLOAT,"SetFireballSpeed",InputSetFireballSpeed),
	DEFINE_INPUTFUNC(FIELD_FLOAT,"SetFireballDamage",InputSetFireballDamage),
	DEFINE_INPUTFUNC(FIELD_INTEGER,"SetFireballSize",InputSetFireballType),

	DEFINE_OUTPUT(m_OnShoot,"OnShoot"),

END_DATADESC()


void CFireballShooter::Spawn()
{
	Precache();

	m_bSilent = HasSpawnFlags(SF_FIREBALL_SHOOTER_SILENT);
	m_bBreakOnDmg = HasSpawnFlags(SF_FIREBALL_SHOOTER_BREAK_ON_DMG);

	if (HasSpawnFlags(SF_FIREBALL_NO_DLIGHT))
		bUseDLight = false;
	else
		bUseDLight = true;
}

void CFireballShooter::Precache()
{
	if (!m_bSilent)
	{
		char* szSoundFile = (char*)STRING(m_iszSound);
		if (m_iszSound != NULL_STRING && strlen(szSoundFile) > 1)
		{
			if (*szSoundFile != '!')
			{
				PrecacheScriptSound(szSoundFile);
			}
		}
	}

	UTIL_PrecacheOther("fireball");
}

void CFireballShooter::ShootFireball()
{
	char* szSoundFile = (char*)STRING(m_iszSound);
	CBaseEntity* targetEntity = gEntList.FindEntityByName(NULL, szTargetName);

	Vector vecShootDir;

	if (targetEntity)
	{
		if (targetEntity->IsPlayer())
		{
			vecShootDir = (targetEntity->WorldSpaceCenter() - GetAbsOrigin().Normalized());
		}
		else
			vecShootDir = (targetEntity->GetAbsOrigin() - GetAbsOrigin()).Normalized();
	}
	else
	{
		AngleVectors(GetAbsAngles(), &vecShootDir);
	}

	CShooterFireball* pBall = CShooterFireball::CreateFireball(GetAbsOrigin(), iFireballType, fDamage, m_bBreakOnDmg, bUseDLight);

	if (pBall)
	{
		pBall->SetAbsVelocity(vecShootDir * fSpeed);
		EmitSound(szSoundFile);
	}

	m_OnShoot.FireOutput(this, this);
}

void CFireballShooter::InputShootFireball(inputdata_t& inputdata)
{
	ShootFireball();
}

void CFireballShooter::InputSetFireballDamage(inputdata_t& inputdata)
{
	fDamage = inputdata.value.Float();
}
void CFireballShooter::InputSetFireballType(inputdata_t& inputdata)
{
	iFireballType = (FireballType)inputdata.value.Int();
}
void CFireballShooter::InputSetFireballSpeed(inputdata_t& inputdata)
{
	fSpeed = inputdata.value.Float();
}