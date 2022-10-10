#include "cbase.h"
#include "items.h"
#include "hl2_player.h"
#include "player.h"
#include "baseanimating.h"


#include "memdbgon.h"


ConVar sk_armor_boots("sk_armor_boots", "25");
ConVar sk_armor_helmet("sk_armor_helmet", "50");
ConVar sk_armor_chest("sk_armor_chest", "75");


#define BOOTS_MODEL "models/items/armor_boots.mdl"
#define HELMET_MODEL "models/items/armor_helmet.mdl"
#define CHEST_MODEL "models/items/armor_chest.mdl"

///
/// Boots Armor
///
class CItemArmorBoots : public CItem
{
	DECLARE_CLASS(CItemArmorBoots, CItem);

public:
	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer* pPlayer);
};

LINK_ENTITY_TO_CLASS(item_armor_boots, CItemArmorBoots);
PRECACHE_REGISTER(item_armor_boots);


void CItemArmorBoots::Spawn(void)
{
	Precache();
	SetModel(BOOTS_MODEL);
	BaseClass::Spawn();
}

void CItemArmorBoots::Precache(void)
{
	PrecacheModel(BOOTS_MODEL);
}

bool CItemArmorBoots::MyTouch(CBasePlayer* pPlayer)
{
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	return (pHL2Player && pHL2Player->ApplyArmor(sk_armor_boots.GetFloat()));
}

///
/// Helmet Armor
///
class CItemArmorHelmet : public CItem
{
	DECLARE_CLASS(CItemArmorHelmet, CItem);

public:
	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer* pPlayer);
};

LINK_ENTITY_TO_CLASS(item_armor_helmet, CItemArmorHelmet);
PRECACHE_REGISTER(item_armor_helmet);


void CItemArmorHelmet::Spawn(void)
{
	Precache();
	SetModel(HELMET_MODEL);
	BaseClass::Spawn();
}

void CItemArmorHelmet::Precache(void)
{
	PrecacheModel(HELMET_MODEL);
}

bool CItemArmorHelmet::MyTouch(CBasePlayer* pPlayer)
{
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	return (pHL2Player && pHL2Player->ApplyArmor(sk_armor_helmet.GetFloat()));
}

///
/// Chest Armor
/// 

class CItemArmorChest : public CItem
{
	DECLARE_CLASS(CItemArmorChest, CItem);

public:
	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer* pPlayer);
};

LINK_ENTITY_TO_CLASS(item_armor_chest, CItemArmorChest);
LINK_ENTITY_TO_CLASS(item_armor, CItemArmorChest);
PRECACHE_REGISTER(item_armor_chest);


void CItemArmorChest::Spawn(void)
{
	Precache();
	SetModel(CHEST_MODEL);
	BaseClass::Spawn();
}

void CItemArmorChest::Precache(void)
{
	PrecacheModel(CHEST_MODEL);
}

bool CItemArmorChest::MyTouch(CBasePlayer* pPlayer)
{
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	return (pHL2Player && pHL2Player->ApplyArmor(sk_armor_chest.GetFloat()));
}