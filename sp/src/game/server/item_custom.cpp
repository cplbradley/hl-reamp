#include "cbase.h"
#include "items.h"
#include "player.h"
#include "hl2_player.h"

#include "tier0/memdbgon.h"

class CItemHealthCustom : public CItem
{
public:
	DECLARE_CLASS(CItemHealthCustom, CItem);
	DECLARE_DATADESC();
	void Precache(void);
	void Spawn(void);
	bool MyTouch(CBasePlayer *pPlayer);
	float m_fHealth;
};

LINK_ENTITY_TO_CLASS(item_health_custom, CItemHealthCustom);

BEGIN_DATADESC(CItemHealthCustom)
DEFINE_KEYFIELD(m_fHealth, FIELD_FLOAT, "CustomHealth"),
END_DATADESC()

PRECACHE_REGISTER(item_health_custom);


void CItemHealthCustom::Precache(void)
{
}
void CItemHealthCustom::Spawn(void)
{
	char *szModel = (char *)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		Warning("prop at %.0f %.0f %0.f missing modelname\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		UTIL_Remove(this);
		return;
	}
	PrecacheModel(szModel);
	Precache();
	SetModel(szModel);
	BaseClass::Spawn();
}

bool CItemHealthCustom::MyTouch(CBasePlayer *pPlayer)
{
	if (pPlayer->TakeHealth(m_fHealth, DMG_GENERIC))
	{
		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "ItemPickup");
		WRITE_STRING(GetClassname());
		MessageEnd();

		CPASAttenuationFilter filter(pPlayer, "HealthKit.Touch");
		EmitSound(filter, pPlayer->entindex(), "HealthKit.Touch");

		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}

		return true;
	}

	return false;
}

class CItemBatteryCustom : public CItem
{
public:
	DECLARE_CLASS(CItemBatteryCustom, CItem);
	DECLARE_DATADESC();
	void Precache(void);
	void Spawn(void);
	bool MyTouch(CBasePlayer *pPlayer);
	float m_fBattery;
};

LINK_ENTITY_TO_CLASS(item_battery_custom, CItemBatteryCustom);

BEGIN_DATADESC(CItemBatteryCustom)
DEFINE_KEYFIELD(m_fBattery, FIELD_FLOAT, "CustomBattery"),
END_DATADESC()

PRECACHE_REGISTER(item_battery_custom);



void CItemBatteryCustom::Precache(void)
{
}
void CItemBatteryCustom::Spawn(void)
{
	char *szModel = (char *)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		Warning("prop at %.0f %.0f %0.f missing modelname\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		UTIL_Remove(this);
		return;
	}
	PrecacheModel(szModel);
	Precache();
	SetModel(szModel);
	BaseClass::Spawn();
}

bool CItemBatteryCustom::MyTouch(CBasePlayer *pPlayer)
{
	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>(pPlayer);
	if ((pHL2Player->ArmorValue() < 150) && pHL2Player->IsSuitEquipped())
	{
		int pct;
		char szcharge[64];

		pHL2Player->IncrementArmorValue(m_fBattery, 150);

		CPASAttenuationFilter filter(this, "ItemBattery.Touch");
		EmitSound(filter, entindex(), "ItemBattery.Touch");

		CSingleUserRecipientFilter user(pHL2Player);
		user.MakeReliable();

		UserMessageBegin(user, "ItemPickup");
		WRITE_STRING("item_battery");
		MessageEnd();


		// Suit reports new power level
		// For some reason this wasn't working in release build -- round it.
		pct = (int)((float)(pHL2Player->ArmorValue() * 100.0) * (1.0 / 150) + 0.5);
		pct = (pct / 5);
		if (pct > 0)
			pct--;

		Q_snprintf(szcharge, sizeof(szcharge), "!HEV_%1dP", pct);
		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}

		//UTIL_EmitSoundSuit(edict(), szcharge);
		//SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
		return true;
	}
	return false;
}