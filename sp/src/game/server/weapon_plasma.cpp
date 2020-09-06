
#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "gamestats.h"
#include "beam_shared.h"

//#define PLASMA_MODEL "models/spitball_medium.mdl"
//#define PLASMA_SPEED 4500
#include "tier0/memdbgon.h"

/*
class CPlasma : public CBaseAnimating
{
	DECLARE_CLASS(CPlasma, CBaseAnimating);

	void	Spawn(void);
	void	Precache(void);
	void	PlasmaTouch(CBaseEntity *pOther);
	bool	CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;

	static CPlasma *Create(const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner);
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(plasma_ball, CPlasma);
BEGIN_DATADESC(CPlasma)
// Function Pointers
DEFINE_FUNCTION(PlasmaTouch),
END_DATADESC()
CPlasma *CPlasma::Create(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner)
{
	// Create a new entity with CCrossbowBolt private data
	CPlasma *pBall = (CCrossbowBolt *)CreateEntityByName("plasma_ball");
	UTIL_SetOrigin(pBall, vecOrigin);
	pBall->SetAbsAngles(angAngles);
	pBall->Spawn();
	pBall->SetOwnerEntity(pentOwner);

	return pBall;
}
bool CPlasma::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}
unsigned int CPlasma::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
CPlasma::Spawn(void)
{
	Precache();
	SetModel("models/crossbow_bolt.mdl");
	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(0.3f, 0.3f, 0.3f), Vector(0.3f, 0.3f, 0.3f));
	SetSolid(SOLID_BBOX);

	SetTouch(&CPlasma::PlasmaTouch);

}
CPlasma::Precache(void)
{
	PrecacheModel(PLASMA_MODEL);
}
void CPlasma::PlasmaTouch(CBaseEntity *pOther)
{
	UTIL_Remove(this);
}
class CWeaponPlasma : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponPlasma, CBaseHLCombatWeapon);
public:

	CWeaponPlasma(void);
	void PrimaryAttack(void);
	void ItemPostFrame(void);
	void ItemBusyFrame(void);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

};
LINK_ENTITY_TO_CLASS(weapon_plasma, CWeaponPlasma);
PRECACHE_WEAPON_REGISTER(weapon_plasma);

IMPLEMENT_SERVERCLASS_ST(CWeaponPlasma, DT_WeaponPlasma)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponPlasma)
END_DATADESC()

CWeaponPlasma::PrimaryAttack(void)
{
	CPlasma::Create (GetOwner()->Weapon_ShootPosition(), GetOwner()->EyeAngles(), GetOwnerEntity());
}
CWeaponPlasma::ItemPostFrame(void)
{
}
CWeaponPlasma::ItemBusyFrame(void)
{
}
*/