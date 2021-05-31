//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the suit batteries.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"
#include "SpriteTrail.h"
#include "Sprite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ITEM_MAGNET_RADIUS 256.0f
#define ITEM_MAGNET_SPEED 800.0f

class CItemBattery : public CItem
{
public:
	DECLARE_CLASS(CItemBattery, CItem);
	DECLARE_DATADESC();
	void Spawn(void);
	void Precache(void);
	void DrawTrail(void);
	void DrawGlowSprite(void);
	bool MyTouch(CBasePlayer *pPlayer);
protected:
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);
BEGIN_DATADESC(CItemBattery)
END_DATADESC()
PRECACHE_REGISTER(item_battery);

	
	void CItemBattery::Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/battery.mdl" );
		DrawTrail( );
		DrawGlowSprite();
		BaseClass::Spawn();
		
	}
	void CItemBattery::Precache(void)
	{
		PrecacheModel ("models/items/battery.mdl");
		PrecacheModel ("sprites/laser.vmt");
		PrecacheModel ("sprites/animglow01.vmt");
		PrecacheScriptSound( "ItemBattery.Touch" );
	}
	void CItemBattery::DrawTrail(void)
	{
		m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/laser.vmt", GetLocalOrigin(), false);

		if (m_pGlowTrail != NULL)
		{
			m_pGlowTrail->FollowEntity(this);
			m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 155, 255, 255, kRenderFxNone);
			m_pGlowTrail->SetStartWidth(14.0f);
			m_pGlowTrail->SetEndWidth(1.0f);
			m_pGlowTrail->SetLifeTime(1.0f);
		}
	}
	void CItemBattery::DrawGlowSprite(void)
	{
		m_pMainGlow = CSprite::SpriteCreate("sprites/animglow01.vmt", GetLocalOrigin(), false);


		if (m_pMainGlow != NULL)
		{
			m_pMainGlow->FollowEntity(this);
			m_pMainGlow->SetTransparency(kRenderGlow, 0, 155, 255, 200, kRenderFxNoDissipation);
			m_pMainGlow->SetScale(0.45f);
			m_pMainGlow->SetGlowProxySize(4.0f);
		}

	}
	bool CItemBattery::MyTouch(CBasePlayer *pPlayer)
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>( pPlayer );
		return ( pHL2Player && pHL2Player->ApplyBattery() );
	}


class CItemBatteryDrop : public CItem
{
public:
	DECLARE_CLASS(CItemBatteryDrop, CItem);
	DECLARE_DATADESC();
	void Spawn(void)
	{
		Precache();
		SetModel("models/items/resdrop.mdl");
		SetRenderColor(0, 150, 255);
		DrawTrail();
		DrawGlowSprite();
		SetThink(&CItemBatteryDrop::DelayedKill);
		SetNextThink(gpGlobals->curtime + 10.0f);

		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/resdrop.mdl");
		PrecacheModel("sprites/laser.vmt");
		PrecacheModel("sprites/animglow01.vmt");
		PrecacheScriptSound("ItemBattery.Touch");
	}
	void DrawTrail(void)
	{
		m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/laser.vmt", GetLocalOrigin(), false);

		if (m_pGlowTrail != NULL)
		{
			m_pGlowTrail->FollowEntity(this);
			m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 155, 255, 255, kRenderFxNone);
			m_pGlowTrail->SetStartWidth(14.0f);
			m_pGlowTrail->SetEndWidth(0.0f);
			m_pGlowTrail->SetLifeTime(1.0f);
		}
	}
	void DrawGlowSprite(void)
	{
		m_pMainGlow = CSprite::SpriteCreate("sprites/animglow01.vmt", GetLocalOrigin(), false);


		if (m_pMainGlow != NULL)
		{
			m_pMainGlow->FollowEntity(this);
			m_pMainGlow->SetTransparency(kRenderGlow, 0, 155, 255, 200, kRenderFxNoDissipation);
			m_pMainGlow->SetScale(0.45f);
			m_pMainGlow->SetGlowProxySize(4.0f);
		}

	}
	void DelayedKill(void)
	{
		UTIL_Remove(this);
	}
	void CheckQuantity(void)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		int ArmorCount = pPlayer->ArmorValue();
		if (ArmorCount < 100)
		{
			m_bShouldSeek = true;
		}
		else
		{
			m_bShouldSeek = false;
		}

	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>(pPlayer);
		return (pHL2Player && pHL2Player->ApplyBattery());
	}
protected:
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
};

LINK_ENTITY_TO_CLASS(item_batterydrop, CItemBatteryDrop);
BEGIN_DATADESC(CItemBatteryDrop)
DEFINE_THINKFUNC(DelayedKill),
END_DATADESC()
PRECACHE_REGISTER(item_batterydrop);

class CItemArmor : public CItem
{
public:
	DECLARE_CLASS(CItemArmor, CItem);

	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer *pPlayer);
};

LINK_ENTITY_TO_CLASS(item_armor, CItemArmor);
PRECACHE_REGISTER(item_armor);


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItemArmor::Spawn(void)
{
	Precache();
	SetModel("models/items/armor.mdl");
	BaseClass::Spawn();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItemArmor::Precache(void)
{
	PrecacheModel("models/items/armor.mdl");
	PrecacheScriptSound("ItemBattery.Touch");
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : 
//-----------------------------------------------------------------------------
bool CItemArmor::MyTouch(CBasePlayer *pPlayer)
{
	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>(pPlayer);
	return (pHL2Player && pHL2Player->ApplyArmor());
}
