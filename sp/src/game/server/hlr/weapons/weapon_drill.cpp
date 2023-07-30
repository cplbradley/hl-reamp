#include "cbase.h"
#include "basehlcombatweapon.h"
#include "hl2_player.h"
#include "in_buttons.h"
#include "baseanimating.h"
#include "game.h"
#include "player.h"



#include "tier0/memdbgon.h"


ConVar sk_plr_dmg_drill("sk_plr_dmg_drill", "4", FCVAR_GAMEDLL);

class CWeaponDrill : public CBaseHLCombatWeapon
{

	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponDrill, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();
	void Precache(void);
	void PrimaryAttack(void);
	//void SecondaryAttack(void);
	//void Impact(void);
private:
	//float m_fImpactDamage;
	//float m_fImpactTime;

};
IMPLEMENT_SERVERCLASS_ST(CWeaponDrill, DT_WeaponDrill)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_drill, CWeaponDrill);
PRECACHE_WEAPON_REGISTER(weapon_drill);

BEGIN_DATADESC(CWeaponDrill)
END_DATADESC()

void CWeaponDrill::Precache(void)
{
	BaseClass::Precache();

}

void CWeaponDrill::PrimaryAttack(void)
{
	m_flNextPrimaryAttack = gpGlobals->curtime;
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecDir;
	AngleVectors(pPlayer->EyeAngles(), &vecDir);
	Vector vecEnd = (vecSrc + (vecDir * 64.0f));
	trace_t tr;
	UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT, pPlayer,COLLISION_GROUP_NONE, &tr);

	if (tr.fraction != 1.0f)
	{
		CBaseEntity *pEnt = tr.m_pEnt;
		ClearMultiDamage();
		CTakeDamageInfo dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_drill.GetFloat(), DMG_CLUB);
		CalculateMeleeDamageForce(&dmgInfo, vecDir, tr.endpos);
		pEnt->DispatchTraceAttack(dmgInfo, vecDir, &tr);
		ApplyMultiDamage();
	}
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	DevMsg("fuck\n");
}