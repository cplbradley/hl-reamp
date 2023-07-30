#include "cbase.h"
#include "items.h"
#include "hl2_player.h"
#include "player.h"
#include "baseanimating.h"
#include "Sprite.h"

#include "memdbgon.h"


ConVar sk_armor_boots("sk_armor_boots", "25");
ConVar sk_armor_helmet("sk_armor_helmet", "50");
ConVar sk_armor_chest("sk_armor_chest", "75");


#define BOOTS_MODEL "models/items/armor_boots.mdl"
#define HELMET_MODEL "models/items/armor_helmet.mdl"
#define CHEST_MODEL "models/items/armor_chest.mdl"
#define ARMOR_SPRITE "sprites/armor.vmt"

///
/// Boots Armor
///
class CItemArmorBoots : public CItem
{
	DECLARE_CLASS(CItemArmorBoots, CItem);
	DECLARE_DATADESC();

public:
	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer* pPlayer);
	bool EnableEffects();

private:
	CHandle<CSprite> m_pSprite;
};

LINK_ENTITY_TO_CLASS(item_armor_boots, CItemArmorBoots);
PRECACHE_REGISTER(item_armor_boots);

BEGIN_DATADESC(CItemArmorBoots)
DEFINE_FIELD(m_pSprite,FIELD_EHANDLE)
END_DATADESC()

void CItemArmorBoots::Spawn(void)
{
	Precache();
	SetModel(BOOTS_MODEL);
	EnableEffects();
	BaseClass::Spawn();
}

bool CItemArmorBoots::EnableEffects()
{
	m_pSprite = CSprite::SpriteCreate(ARMOR_SPRITE, GetAbsOrigin(), false);
	if (m_pSprite != NULL)
	{
		m_pSprite->FollowEntity(this);
		m_pSprite->SetLocalOrigin(vec3_origin + Vector(0, 0, 16));
		m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxHologram);
		m_pSprite->SetScale(0.4f);
		m_pSprite->SetGlowProxySize(16.0f);
	}

	return true;
}
void CItemArmorBoots::Precache(void)
{
	PrecacheModel(BOOTS_MODEL);
	PrecacheMaterial(ARMOR_SPRITE);
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
	DECLARE_DATADESC();
public:
	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer* pPlayer);
	bool EnableEffects();

private:
	CHandle<CSprite> m_pSprite;
};

LINK_ENTITY_TO_CLASS(item_armor_helmet, CItemArmorHelmet);
PRECACHE_REGISTER(item_armor_helmet);
BEGIN_DATADESC(CItemArmorHelmet)
DEFINE_FIELD(m_pSprite, FIELD_EHANDLE)
END_DATADESC()

void CItemArmorHelmet::Spawn(void)
{
	Precache();
	SetModel(HELMET_MODEL);
	EnableEffects();
	BaseClass::Spawn();
}
bool CItemArmorHelmet::EnableEffects()
{
	m_pSprite = CSprite::SpriteCreate(ARMOR_SPRITE, GetAbsOrigin(), false);
	if (m_pSprite != NULL)
	{
		m_pSprite->FollowEntity(this);
		m_pSprite->SetLocalOrigin(vec3_origin + Vector(0, 0, 16));
		m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxHologram);
		m_pSprite->SetScale(0.4f);
		m_pSprite->SetGlowProxySize(16.0f);
	}

	return true;
}
void CItemArmorHelmet::Precache(void)
{
	PrecacheModel(HELMET_MODEL);
	PrecacheMaterial(ARMOR_SPRITE);
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
	DECLARE_DATADESC();
public:
	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer* pPlayer);
	bool EnableEffects();

private:
	CHandle<CSprite> m_pSprite;
};

LINK_ENTITY_TO_CLASS(item_armor_chest, CItemArmorChest);
LINK_ENTITY_TO_CLASS(item_armor, CItemArmorChest);
PRECACHE_REGISTER(item_armor_chest);

BEGIN_DATADESC(CItemArmorChest)
DEFINE_FIELD(m_pSprite, FIELD_EHANDLE)
END_DATADESC()

void CItemArmorChest::Spawn(void)
{
	Precache();
	EnableEffects();
	SetModel(CHEST_MODEL);
	BaseClass::Spawn();
}
bool CItemArmorChest::EnableEffects()
{
	m_pSprite = CSprite::SpriteCreate(ARMOR_SPRITE, GetAbsOrigin(), false);
	if (m_pSprite != NULL)
	{
		m_pSprite->FollowEntity(this);
		m_pSprite->SetLocalOrigin(GetAbsOrigin() + Vector(0, 0, 32));
		m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxHologram);
		m_pSprite->SetScale(0.4f);
		m_pSprite->SetGlowProxySize(16.0f);
	}

	return true;
}
void CItemArmorChest::Precache(void)
{
	PrecacheModel(CHEST_MODEL);
	PrecacheMaterial(ARMOR_SPRITE);
}

bool CItemArmorChest::MyTouch(CBasePlayer* pPlayer)
{
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	return (pHL2Player && pHL2Player->ApplyArmor(sk_armor_chest.GetFloat()));
}