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
	void TraceThink(void);
	void SeekThink(void);
	bool MyTouch(CBasePlayer *pPlayer);
protected:
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);
BEGIN_DATADESC(CItemBattery)
DEFINE_THINKFUNC(SeekThink),
DEFINE_THINKFUNC(TraceThink),
END_DATADESC()
PRECACHE_REGISTER(item_battery);

	
	void CItemBattery::Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/battery.mdl" );
		DrawTrail( );
		DrawGlowSprite();
		SetThink(&CItemBattery::TraceThink);
		BaseClass::Spawn();
		
	}
	void CItemBattery::Precache(void)
	{
		PrecacheModel ("models/items/battery.mdl");
		PrecacheModel ("sprites/bluelaser1.vmt");
		PrecacheModel ("sprites/animglow01.vmt");
		PrecacheScriptSound( "ItemBattery.Touch" );
	}
	void CItemBattery::DrawTrail(void)
	{
		m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/bluelaser1.vmt", GetLocalOrigin(), false);

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
	void CItemBattery::TraceThink(void)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		float flNearest = (ITEM_MAGNET_RADIUS * ITEM_MAGNET_RADIUS);
		if (pPlayer)
		{
			float flDist = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();

			if (flDist < flNearest)
			{
				SetThink(&CItemBattery::SeekThink);
				SetNextThink(gpGlobals->curtime + 0.05f);
				SetMoveType(MOVETYPE_FLY);
			}
			return;
		}
		return;
	}
	/*Vector GetSteerVector(const Vector &vecForward)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		Vector vecSteer = vec3_origin;
		Vector vecRight, vecUp;
		VectorVectors(vecForward, vecRight, vecUp);

		// Use two probes fanned out a head of us
		Vector vecProbe;
		float flSpeed = GetAbsVelocity().Length();

		// Try right 
		vecProbe = vecForward + vecRight;
		vecProbe *= flSpeed;

		// We ignore multiple targets
		CTraceFilterSimpleList filterSkip(COLLISION_GROUP_NONE);
		filterSkip.AddEntityToIgnore(this);
		filterSkip.AddEntityToIgnore(GetOwnerEntity());
		filterSkip.AddEntityToIgnore(pPlayer);

		trace_t tr;
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecProbe, MASK_SHOT, &filterSkip, &tr);
		vecSteer -= vecRight * 100.0f * (1.0f - tr.fraction);

		// Try left
		vecProbe = vecForward - vecRight;
		vecProbe *= flSpeed;

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecProbe, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		vecSteer += vecRight * 100.0f * (1.0f - tr.fraction);

		return vecSteer;
	}*/
	void CItemBattery::SeekThink(void)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		Vector vecDir = (pPlayer->WorldSpaceCenter() - GetAbsOrigin());
		VectorNormalize(vecDir);

		/*if (flSpeed < ITEM_MAGNET_SPEED)
		{
			// Accelerate by the desired amount	
			flSpeed += (ITEM_MAGNET_SPEED * flDelta);
			if (flSpeed > ITEM_MAGNET_SPEED)
			{
				flSpeed = ITEM_MAGNET_SPEED;
			}
		}

		// Steer!
		Vector vecRight, vecUp;
		VectorVectors(vecDir, vecRight, vecUp);
		Vector vecOffset = vec3_origin;
		vecOffset += vecUp * cos(gpGlobals->curtime * 20.0f) * 200.0f * gpGlobals->frametime;
		vecOffset += vecRight * sin(gpGlobals->curtime * 15.0f) * 200.0f * gpGlobals->frametime;

		vecOffset += GetSteerVector(vecDir);
		*/
		SetAbsVelocity(vecDir * ITEM_MAGNET_SPEED);
		SetNextThink(gpGlobals->curtime + 0.05f);
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
protected:
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	void Spawn(void)
	{
		Precache();
		SetModel("models/items/battery.mdl");
		DrawTrail();
		DrawGlowSprite();
		SetThink(&CItemBatteryDrop::DelayedKill);
		SetNextThink(gpGlobals->curtime + 10.0f);
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/battery.mdl");
		PrecacheModel("sprites/bluelaser1.vmt");
		PrecacheModel("sprites/animglow01.vmt");
		PrecacheScriptSound("ItemBattery.Touch");
	}
	void DrawTrail(void)
	{
		m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/bluelaser1.vmt", GetLocalOrigin(), false);

		if (m_pGlowTrail != NULL)
		{
			m_pGlowTrail->FollowEntity(this);
			m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 155, 255, 255, kRenderFxNone);
			m_pGlowTrail->SetStartWidth(14.0f);
			m_pGlowTrail->SetEndWidth(1.0f);
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
	bool MyTouch(CBasePlayer *pPlayer)
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>(pPlayer);
		return (pHL2Player && pHL2Player->ApplyBattery());
	}
};

LINK_ENTITY_TO_CLASS(item_batterydrop, CItemBatteryDrop);
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
