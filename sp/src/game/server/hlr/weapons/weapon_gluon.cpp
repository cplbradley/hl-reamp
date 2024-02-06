#include "cbase.h"
#include "player.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
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

	SetAbsVelocity(vecDir * (dist * 5.f));
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

public:
	CWeaponGluon();
	void Precache();
	virtual void ItemPostFrame();
	virtual void PrimaryAttack();
	virtual bool Holster(CBaseCombatWeapon* pSwitchingTo);

	void StartGluon();
	void FireGluon();
	void StopGluon();

	GluonState_t GetState() { return m_state; }
	void SetState(GluonState_t state) { m_state = state; }

private:
	CNetworkVector(m_vecDamagePoint);
	CNetworkVar(bool, m_bFiring);

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
END_SEND_TABLE()

PRECACHE_WEAPON_REGISTER(weapon_gluon);

BEGIN_DATADESC(CWeaponGluon)
END_DATADESC()


CWeaponGluon::CWeaponGluon()
{
	m_vecDamagePoint.Init();
	m_bFiring = false;
}

void CWeaponGluon::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound("Gluon.Start");
	PrecacheScriptSound("Gluon.Stop");
	PrecacheParticleSystem("gluon_start");
}

void CWeaponGluon::PrimaryAttack()
{

}

void CWeaponGluon::ItemPostFrame()
{

	CBasePlayer* player = ToBasePlayer(GetOwner());
	if (!player)
		return;

	engine->Con_NPrintf(0, "Gluon State: %i", m_state);
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
	DispatchParticleEffect("gluon_start", PATTACH_POINT_FOLLOW, player->GetViewModel(), "muzzle");
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

	UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID_BRUSHONLY, player, COLLISION_GROUP_NONE, &tr);
	if (!pTarget)
	{
		pTarget = (CGluonTarget*)CreateEntityByName("gluon_target");
		UTIL_SetOrigin(pTarget, tr.endpos);
		pTarget->Spawn();
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

	CTakeDamageInfo dmgInfo(this, player, damage, DMG_ENERGYBEAM);

	for (int i = 0; i < 3; i++)
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
			}
		}
		}
	}

	
	m_fShakeScale = Clamp(m_fShakeScale, 0.3f, 0.7f);
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
	return BaseClass::Holster(pSwitchingTo);
}