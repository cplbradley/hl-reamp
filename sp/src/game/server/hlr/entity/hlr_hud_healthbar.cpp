#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include "string_t.h"
#include "mathlib/mathlib.h"
#include "globalstate.h"
#include "ndebugoverlay.h"
#include "saverestore_utlvector.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "gameinterface.h"

#include "tier0/memdbgon.h"

enum
{
	HEALTHBAR_SLOT1,
	HEALTHBAR_SLOT2,
	HEALTHBAR_SLOT3,
};

class CHLRHudHealthbar : public CLogicalEntity
{
	DECLARE_CLASS(CHLRHudHealthbar, CLogicalEntity);
	
public:
	void Spawn(void);
	void Enable(void);
	void InputEnable(inputdata_t &inputdata);
	void TransmitData(void);
	void ClearHud(void);
	void OnRestore();
	const char* targetent1;
	CBaseEntity *Ent;

	int m_iSlot;
	float gHealth;
	float gMaxHealth;
	bool bActive;
	string_t szName;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_hud_healthbar, CHLRHudHealthbar);

BEGIN_DATADESC(CHLRHudHealthbar)
DEFINE_THINKFUNC(TransmitData),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_KEYFIELD(targetent1, FIELD_STRING, "targetent1"),
DEFINE_FIELD(bActive,FIELD_BOOLEAN),
DEFINE_FIELD(Ent, FIELD_CLASSPTR),
DEFINE_KEYFIELD(m_iSlot, FIELD_INTEGER, "HealthBarSlot"),
DEFINE_KEYFIELD(szName,FIELD_STRING,"NameLabel"),
END_DATADESC()
void CHLRHudHealthbar::Spawn(void)
{
	//Ent = gEntList.FindEntityByName(NULL, targetent1);
	ClearHud();
	//SetThink(&CHLRHudHealthbar::TransmitData);
	//SetNextThink(gpGlobals->curtime);
	bActive = false;
}
void CHLRHudHealthbar::InputEnable(inputdata_t &inputdata)
{
	Enable();
}

void CHLRHudHealthbar::Enable(void)
{
	switch (m_iSlot)
	{
	case HEALTHBAR_SLOT1:
	{
		Ent = gEntList.FindEntityByName(NULL, targetent1);
		if (!Ent)
			return;
		bActive = true;
		gHealth = Ent->GetHealth();
		gMaxHealth = Ent->GetMaxHealth();
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "EnemyHealth1");
		WRITE_FLOAT(gHealth);
		WRITE_FLOAT(gMaxHealth);
		WRITE_BOOL(bActive);
		WRITE_STRING(STRING(szName));
		MessageEnd();
		Msg("enabled, sending name %s\n",STRING(szName));
		break;
	}
	case HEALTHBAR_SLOT2:
	{
		Ent = gEntList.FindEntityByName(NULL, targetent1);
		if (!Ent)
			return;
		bActive = true;
		gHealth = Ent->GetHealth();
		gMaxHealth = Ent->GetMaxHealth();
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "EnemyHealth2");
		WRITE_FLOAT(gHealth);
		WRITE_FLOAT(gMaxHealth);
		WRITE_BOOL(bActive);
		WRITE_STRING(STRING(szName));
		MessageEnd();
		Msg("enabled, sending name %s\n", STRING(szName));
		break;
	}
	case HEALTHBAR_SLOT3:
	{
		Ent = gEntList.FindEntityByName(NULL, targetent1);
		if (!Ent)
			return;
		bActive = true;
		gHealth = Ent->GetHealth();
		gMaxHealth = Ent->GetMaxHealth();
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "EnemyHealth3");
		WRITE_FLOAT(gHealth);
		WRITE_FLOAT(gMaxHealth);
		WRITE_BOOL(bActive);
		WRITE_STRING(STRING(szName));
		MessageEnd();
		Msg("enabled, sending name %s\n", STRING(szName));
		break;
	}
	}
	
	SetThink(&CHLRHudHealthbar::TransmitData);
	SetNextThink(gpGlobals->curtime);
	
}
void CHLRHudHealthbar::TransmitData(void)
{
	switch (m_iSlot)
	{
	case HEALTHBAR_SLOT1:
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		Ent = gEntList.FindEntityByName(NULL, targetent1);
		if (!Ent)
		{
			bActive = false;
			CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
			user.MakeReliable();
			UserMessageBegin(user, "EnemyHealth1");
			WRITE_FLOAT(1.0f);
			WRITE_FLOAT(1.0f);
			WRITE_BOOL(bActive);
			WRITE_STRING(STRING(szName));
			MessageEnd();
			SetThink(NULL);
			return;
		}
		if (!pPlayer->IsAlive())
		{
			bActive = false;
			CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
			user.MakeReliable();
			UserMessageBegin(user, "EnemyHealth1");
			WRITE_FLOAT(1.0f);
			WRITE_FLOAT(1.0f);
			WRITE_BOOL(bActive);
			WRITE_STRING(STRING(szName));
			MessageEnd();
			SetThink(NULL);
			return;
		}
		gMaxHealth = Ent->GetMaxHealth();
		gHealth = Ent->GetHealth();
		if (gHealth != gMaxHealth)
		{
			if (gHealth > 0)
			{
				bActive = true;
				CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
				user.MakeReliable();
				UserMessageBegin(user, "EnemyHealth1");
				WRITE_FLOAT(gHealth);
				WRITE_FLOAT(gMaxHealth);
				WRITE_BOOL(bActive);
				WRITE_STRING(STRING(szName));
				MessageEnd();
			}
			else
			{
				bActive = false;
				CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
				user.MakeReliable();
				UserMessageBegin(user, "EnemyHealth1");
				WRITE_FLOAT(gHealth);
				WRITE_FLOAT(gMaxHealth);
				WRITE_BOOL(bActive);
				WRITE_STRING(STRING(szName));
				MessageEnd();
				SetThink(NULL);
				return;
			}

		}
		break;
	}
	case HEALTHBAR_SLOT2:
	{
		Ent = gEntList.FindEntityByName(NULL, targetent1);
		if (!Ent)
		{
			bActive = false;
			CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
			user.MakeReliable();
			UserMessageBegin(user, "EnemyHealth2");
			WRITE_FLOAT(1.0f);
			WRITE_FLOAT(1.0f);
			WRITE_BOOL(bActive);
			WRITE_STRING(STRING(szName));
			MessageEnd();
			SetThink(NULL);
			return;
		}

		gMaxHealth = Ent->GetMaxHealth();
		gHealth = Ent->GetHealth();
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

		if (!pPlayer->IsAlive())
		{
			bActive = false;
			CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
			user.MakeReliable();
			UserMessageBegin(user, "EnemyHealth2");
			WRITE_FLOAT(1.0f);
			WRITE_FLOAT(1.0f);
			WRITE_BOOL(bActive);
			WRITE_STRING(STRING(szName));
			MessageEnd();
			SetThink(NULL);
			return;
		}
		if (gHealth != gMaxHealth)
		{
			if (gHealth > 0)
			{
				bActive = true;
				CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
				user.MakeReliable();
				UserMessageBegin(user, "EnemyHealth2");
				WRITE_FLOAT(gHealth);
				WRITE_FLOAT(gMaxHealth);
				WRITE_BOOL(bActive);
				WRITE_STRING(STRING(szName));
				MessageEnd();
			}
			else
			{
				bActive = false;
				CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
				user.MakeReliable();
				UserMessageBegin(user, "EnemyHealth2");
				WRITE_FLOAT(gHealth);
				WRITE_FLOAT(gMaxHealth);
				WRITE_BOOL(bActive);
				WRITE_STRING(STRING(szName));
				MessageEnd();
				SetThink(NULL);
				return;
			}
		}
		break;
	}
	case HEALTHBAR_SLOT3:
	{
		Ent = gEntList.FindEntityByName(NULL, targetent1);
		if (!Ent)
		{
			bActive = false;
			CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
			user.MakeReliable();
			UserMessageBegin(user, "EnemyHealth3");
			WRITE_FLOAT(1.0f);
			WRITE_FLOAT(1.0f);
			WRITE_BOOL(bActive);
			WRITE_STRING(STRING(szName));
			MessageEnd();
			SetThink(NULL);
			return;
		}
		gMaxHealth = Ent->GetMaxHealth();
		gHealth = Ent->GetHealth();
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

		if (!pPlayer->IsAlive())
		{
			bActive = false;
			CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
			user.MakeReliable();
			UserMessageBegin(user, "EnemyHealth3");
			WRITE_FLOAT(1.0f);
			WRITE_FLOAT(1.0f);
			WRITE_BOOL(bActive);
			WRITE_STRING(STRING(szName));
			MessageEnd();
			SetThink(NULL);
			return;
		}
		if (gHealth != gMaxHealth)
		{
			if (gHealth > 0)
			{
				bActive = true;
				CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
				user.MakeReliable();
				UserMessageBegin(user, "EnemyHealth3");
				WRITE_FLOAT(gHealth);
				WRITE_FLOAT(gMaxHealth);
				WRITE_BOOL(bActive);
				WRITE_STRING(STRING(szName));
				MessageEnd();
			}
			else
			{
				bActive = false;
				CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
				user.MakeReliable();
				UserMessageBegin(user, "EnemyHealth3");
				WRITE_FLOAT(gHealth);
				WRITE_FLOAT(gMaxHealth);
				WRITE_BOOL(bActive);
				WRITE_STRING(STRING(szName));
				MessageEnd();
				SetThink(NULL);
				return;
			}
		}
		break;
		}
	}
	SetNextThink(gpGlobals->curtime + 0.1f);
}
void CHLRHudHealthbar::ClearHud(void)
{
	return;
}

void CHLRHudHealthbar::OnRestore()
{
	TransmitData();
}