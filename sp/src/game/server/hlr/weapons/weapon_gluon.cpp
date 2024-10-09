#include "cbase.h"
#include "player.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "particle_parse.h"
#include "in_buttons.h"
#include "basehlcombatweapon.h"


#include "tier0/memdbgon.h"



ConVar sk_plr_dmg_gluon("sk_plr_dmg_gluon", "0",FCVAR_GAMEDLL);

class CGluonTarget : public CBaseEntity
{
	DECLARE_CLASS(CGluonTarget, CBaseEntity);
	DECLARE_SERVERCLASS();
public:
	void Spawn();
	void Precache();
	void UpdateDirection();
	virtual int UpdateTransmitState() { return SetTransmitState(FL_EDICT_ALWAYS); }
	CNetworkVector(m_vecDestination);
	CNetworkVar(bool, m_bOverdrive);
};

LINK_ENTITY_TO_CLASS(gluon_target, CGluonTarget);

IMPLEMENT_SERVERCLASS_ST(CGluonTarget,DT_GluonTarget)
SendPropVector(SENDINFO(m_vecDestination)),
END_SEND_TABLE()

void CGluonTarget::Spawn()
{
	BaseClass::Spawn();
	SetMoveType(MOVETYPE_NOCLIP);
	SetSolid(SOLID_NONE);
	SetSolidFlags(FSOLID_NOT_SOLID);
}

void CGluonTarget::Precache()
{
}

void CGluonTarget::UpdateDirection()
{
	Vector dirvec = m_vecDestination - GetAbsOrigin();
	Vector vecDir = dirvec.Normalized();
	float dist = dirvec.Length();
	float speedmod = 5.f;

	if (m_bOverdrive)
		speedmod = 30.f;

	SetAbsVelocity(vecDir * (dist * speedmod));
}

enum GluonState_t
{
	STATE_IDLE,
	STATE_STARTING,
	STATE_FIRING
};

class CWeaponGluon : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponGluon, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

public:
	CWeaponGluon();
	void Precache();
	virtual void ItemPostFrame();
	virtual void PrimaryAttack();
	virtual bool Holster(CBaseCombatWeapon* pSwitchingTo);
	virtual bool Deploy();

	void StartGluon();
	void FireGluon();
	void StopGluon();

	GluonState_t GetState() { return m_state; }
	void SetState(GluonState_t state) { m_state = state; }

private:
	CNetworkVector(m_vecDamagePoint);
	CNetworkVar(bool, m_bFiring);
	CNetworkVar(bool, m_bOverdrive);
	CNetworkVar(bool, m_bDrawCross);

	float m_fStartTime;
	float m_fNextRemoveAmmo;
	float m_fShakeScale;


	GluonState_t m_state;


	CNetworkHandle(CGluonTarget, pTarget);
};

LINK_ENTITY_TO_CLASS(weapon_gluon, CWeaponGluon)

IMPLEMENT_SERVERCLASS_ST(CWeaponGluon, DT_WeaponGluon)
SendPropBool(SENDINFO(m_bFiring)),
SendPropVector(SENDINFO(m_vecDamagePoint)),
SendPropEHandle(SENDINFO(pTarget)),
SendPropBool(SENDINFO(m_bOverdrive)),
SendPropBool(SENDINFO(m_bDrawCross)),
END_SEND_TABLE()

PRECACHE_WEAPON_REGISTER(weapon_gluon);

BEGIN_DATADESC(CWeaponGluon)
END_DATADESC()



acttable_t	CWeaponGluon::m_acttable[] =
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_PHYSGUN,					false },
};

IMPLEMENT_ACTTABLE(CWeaponGluon);



CWeaponGluon::CWeaponGluon()
{
	m_vecDamagePoint.Init();
	m_bFiring = false;
	m_bDrawCross = false;
}

void CWeaponGluon::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound("Gluon.Start");
	PrecacheScriptSound("Gluon.Stop");
	PrecacheParticleSystem("gluon_start");
	PrecacheParticleSystem("gluon_impact");
	PrecacheParticleSystem("gluon_impact_overdrive");
}

void CWeaponGluon::PrimaryAttack()
{

}

void CWeaponGluon::ItemPostFrame()
{
	CBasePlayer* player = ToBasePlayer(GetOwner());
	if (!player)
		return;

	m_bOverdrive = player->HasOverdrive();

	if (m_bOverdrive && player->GetViewModel()->m_nSkin != 1)
		player->GetViewModel()->m_nSkin = 1;
	else if (!m_bOverdrive && player->GetViewModel()->m_nSkin != 0)
		player->GetViewModel()->m_nSkin = 0;

	//engine->Con_NPrintf(0, "Gluon State: %i", m_state);
	if (player->m_nButtons & IN_ATTACK)
	{
		if (GetState() == STATE_IDLE)
		{
			StartGluon();
		}

		if (GetState() == STATE_STARTING && m_fStartTime < gpGlobals->curtime)
		{
			SetState(STATE_FIRING);
		}

		if (GetState() == STATE_FIRING)
		{
			FireGluon();
		}
	}
	else
	{
		if (GetState() != STATE_IDLE)
			StopGluon();
	}

	BaseClass::ItemPostFrame();
}


void CWeaponGluon::StartGluon()
{
	CBasePlayer* player = ToBasePlayer(GetOwner());
	if (!player)
		return;

	SetState(STATE_STARTING);
	m_fStartTime = gpGlobals->curtime + 1.f;
	EmitSound("Gluon.Start");
	ConVarRef thirdperson("g_thirdperson");

	
	thirdperson.GetBool() ? DispatchParticleEffect("gluon_start", PATTACH_POINT_FOLLOW, this, "muzzle") : DispatchParticleEffect("gluon_start", PATTACH_POINT_FOLLOW, player->GetViewModel(), "muzzle");
}

void CWeaponGluon::FireGluon()
{
	CBasePlayer* player = ToBasePlayer(GetOwner());
	if (!player)
		return;

	if (!m_bFiring)
		m_bFiring = true;


	Vector vecSrc, vecEnd, vecDir;
	trace_t tr;
	QAngle eyeAng;

	eyeAng = player->EyeAngles();
	AngleVectors(eyeAng, &vecDir);
	VectorNormalize(vecDir);
	vecSrc = player->Weapon_ShootPosition();
	vecEnd = vecSrc + (vecDir * MAX_TRACE_LENGTH);

	UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT, player, COLLISION_GROUP_NONE, &tr);
	if (!pTarget)
	{
		pTarget = (CGluonTarget*)CreateEntityByName("gluon_target");
		UTIL_SetOrigin(pTarget, tr.endpos);
		pTarget->Spawn();
		pTarget->m_bOverdrive = m_bOverdrive;
	}
	pTarget->m_vecDestination = tr.endpos;
	pTarget->UpdateDirection();
	m_vecDamagePoint = pTarget->GetAbsOrigin();

	Vector vecMidPoint = (vecSrc + tr.endpos) * 0.5f;
	Vector vecQuarter = (vecMidPoint + player->WorldSpaceCenter()) * 0.5f;
	Vector vecThreeQuarter = (vecMidPoint + pTarget->GetAbsOrigin()) * 0.5f;
	Vector vecMidMid = (vecQuarter + vecThreeQuarter) * 0.5f;
	Vector vecMidMidMid = (vecMidPoint + vecMidMid) * 0.5f;

	trace_t trx;

	float damage = g_pGameRules->SkillAdjustValueInverted(sk_plr_dmg_gluon.GetFloat());

	if (m_bOverdrive)
		damage *= 2;

	CTakeDamageInfo dmgInfo(this, player, damage, DMG_ENERGYBEAM);

	RadiusDamage(dmgInfo, m_vecDamagePoint, 32, CLASS_PLAYER, GetOwnerEntity());
	const char* particle = "gluon_impact";

	if (m_bOverdrive)
		particle = "gluon_impact_overdrive";
	DispatchParticleEffect(particle, PATTACH_ABSORIGIN_FOLLOW, pTarget);

	for (int i = 0; i <= 3; i++)
	{
		switch (i)
		{
		case 0:
		{
			UTIL_TraceHull(vecSrc, vecQuarter, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SHOT, player, COLLISION_GROUP_NONE, &trx);
			if (trx.m_pEnt && trx.m_pEnt->IsNPC())
			{
				ClearMultiDamage();
				trx.m_pEnt->TakeDamage(dmgInfo);
				trx.m_pEnt->DispatchTraceAttack(dmgInfo, (trx.endpos - trx.startpos).Normalized(), &trx);
				ApplyMultiDamage();
				DispatchParticleEffect(particle, trx.endpos, vec3_angle);
			}
		}
		case 1:
		{
			UTIL_TraceHull(vecQuarter, vecMidMidMid, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SHOT, player, COLLISION_GROUP_NONE, &trx);
			if (trx.m_pEnt && trx.m_pEnt->IsNPC())
			{
				ClearMultiDamage();
				trx.m_pEnt->TakeDamage(dmgInfo);
				trx.m_pEnt->DispatchTraceAttack(dmgInfo, (trx.endpos - trx.startpos).Normalized(), &trx);
				ApplyMultiDamage();
				DispatchParticleEffect(particle, trx.endpos, vec3_angle);
			}
		}
		case 2:
		{
			UTIL_TraceHull(vecMidMidMid, vecThreeQuarter, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SHOT, player, COLLISION_GROUP_NONE, &trx);
			if (trx.m_pEnt && trx.m_pEnt->IsNPC())
			{
				ClearMultiDamage();
				trx.m_pEnt->TakeDamage(dmgInfo);
				trx.m_pEnt->DispatchTraceAttack(dmgInfo, (trx.endpos - trx.startpos).Normalized(), &trx);
				ApplyMultiDamage();
				DispatchParticleEffect(particle, trx.endpos, vec3_angle);
			}
		}
		case 3:
		{
			UTIL_TraceHull(vecThreeQuarter, m_vecDamagePoint, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SHOT, player, COLLISION_GROUP_NONE, &trx);
			if (trx.m_pEnt && trx.m_pEnt->IsNPC())
			{
				ClearMultiDamage();
				trx.m_pEnt->TakeDamage(dmgInfo);
				trx.m_pEnt->DispatchTraceAttack(dmgInfo, (trx.endpos - trx.startpos).Normalized(), &trx);
				ApplyMultiDamage();
				DispatchParticleEffect(particle, trx.endpos, vec3_angle);
			}
		}
		}
	}

	
	m_fShakeScale = Clamp(m_fShakeScale, 0.3f, 0.7f);
	if (player->HasOverdrive())
		m_fShakeScale = 2.f;

	player->ViewPunch(QAngle(RandomFloat(-m_fShakeScale, m_fShakeScale), RandomFloat(-m_fShakeScale, m_fShakeScale), RandomFloat(-m_fShakeScale, m_fShakeScale)));

	if (gpGlobals->curtime > m_fNextRemoveAmmo)
	{
		m_fNextRemoveAmmo = gpGlobals->curtime + 0.02f;
		m_fShakeScale += 0.002f;
		player->RemoveAmmo(1, m_iPrimaryAmmoType);
	}
	
}

void CWeaponGluon::StopGluon()
{
	m_fShakeScale = 0.3f;
	m_bFiring = false;
	if(GetState() != STATE_IDLE)
		EmitSound("Gluon.Stop");
	SetState(STATE_IDLE);

	UTIL_Remove(pTarget);
	pTarget = NULL;
}

bool CWeaponGluon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	StopGluon();
	m_bDrawCross = false;
	Msg("holster\n");
	return BaseClass::Holster(pSwitchingTo);
}

bool CWeaponGluon::Deploy()
{
	m_bDrawCross = true;
	return BaseClass::Deploy();
}