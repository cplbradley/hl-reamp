#include "cbase.h"
/*#include "particle_parse.h"
#include "basecombatweapon.h"
#include "baseanimating.h"
#include "player.h"
#include "hl2_player.h"
#include "sprite.h"
#include "beam_shared.h"


#include "tier0/memdbgon.h"

enum {
	STATE_IDLE,
	STATE_START,
	STATE_FIRING
};

class CWeaponGluon : public CBaseCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponGluon, CBaseCombatWeapon);
	DECLARE_SERVERCLASS();
	CWeaponGluon(void);
	void Precache();
	void ItemPostFrame();
	void PrimaryAttack();

	CNetworkVar(bool, m_bFiring);
private:
	int m_iWeaponState;

};

IMPLEMENT_SERVERCLASS_ST(CWeaponGluon,DT_WeaponGluon)
END_SEND_TABLE()


LINK_ENTITY_TO_CLASS(weapon_gluon, CWeaponGluon)

BEGIN_DATADESC(CWeaponGluon)
DEFINE_FIELD(m_iWeaponState, FIELD_INTEGER),
DEFINE_FIELD(m_bFiring,FIELD_BOOLEAN),
END_DATADESC()




CWeaponGluon::CWeaponGluon(void)
{
}

void CWeaponGluon::Precache(void)
{

}

void CWeaponGluon::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	switch (m_iWeaponState)
	{
	case STATE_IDLE:
	{
		if (!HasAmmo())
			return;
		pPlayer->RemoveAmmo(1, GetPrimaryAmmoType());

		EmitSound("Gluon.Start");
		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		m_iWeaponState = STATE_START;
		break;
	}
	case STATE_START:
	{

	}
}*/