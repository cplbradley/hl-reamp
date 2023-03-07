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
		MessageEnd();
		Msg("enabled\n");
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
		MessageEnd();
		Msg("enabled\n");
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
		MessageEnd();
		Msg("enabled\n");
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
				MessageEnd();
				SetThink(NULL);
				return;
			}
		}
		break;
		}
	}
	/*

	

	if (!pPlayer->IsAlive())
	{
		bActive = false;
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "EnemyHealth1");
		WRITE_FLOAT(1.0f);
		WRITE_FLOAT(1.0f);
		WRITE_BOOL(bActive);
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
			UserMessageBegin(user, "EnemyHealth1");
			WRITE_FLOAT(gHealth);
			WRITE_FLOAT(gMaxHealth);
			WRITE_BOOL(bActive);
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
			MessageEnd();
			SetThink(NULL);
			return;
		}
	}*/
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

////////////////////////
////////////////////////
/*


class CHLRHudHealthbar2 : public CLogicalEntity
{
	DECLARE_CLASS(CHLRHudHealthbar2, CLogicalEntity);

public:
	void Spawn(void);
	void Enable(void);
	void InputEnable(inputdata_t &inputdata);
	void TransmitData(void);
	void ClearHud(void);
	const char* targetent1;
	CBaseEntity *Ent;
	float gHealth;
	float gMaxHealth;
	bool bActive;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_hud_healthbar2, CHLRHudHealthbar2);

BEGIN_DATADESC(CHLRHudHealthbar2)
DEFINE_THINKFUNC(TransmitData),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_KEYFIELD(targetent1, FIELD_STRING, "targetent1"),
DEFINE_FIELD(bActive, FIELD_BOOLEAN),
DEFINE_FIELD(Ent, FIELD_CLASSPTR),
END_DATADESC()
void CHLRHudHealthbar2::Spawn(void)
{
	Ent = gEntList.FindEntityByName(NULL, targetent1);
	ClearHud();
	SetThink(&CHLRHudHealthbar2::TransmitData);
	SetNextThink(gpGlobals->curtime);
}
void CHLRHudHealthbar2::InputEnable(inputdata_t &inputdata)
{
	Enable();
}

void CHLRHudHealthbar2::Enable(void)
{
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
	MessageEnd();
	Msg("enabled\n");
	SetThink(&CHLRHudHealthbar2::TransmitData);
	SetNextThink(gpGlobals->curtime);

}
void CHLRHudHealthbar2::TransmitData(void)
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
			MessageEnd();
			SetThink(NULL);
			return;
		}
	}
	SetNextThink(gpGlobals->curtime + 0.1f);
}
void CHLRHudHealthbar2::ClearHud(void)
{
	return;
}

/////////////////////////////////
/////////////////////////////////

class CHLRHudHealthbar3 : public CLogicalEntity
{
	DECLARE_CLASS(CHLRHudHealthbar3, CLogicalEntity);

public:
	void Spawn(void);
	void Enable(void);
	void InputEnable(inputdata_t &inputdata);
	void TransmitData(void);
	void ClearHud(void);
	const char* targetent1;
	CBaseEntity *Ent;
	float gHealth;
	float gMaxHealth;
	bool bActive;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_hud_healthbar3, CHLRHudHealthbar3);

BEGIN_DATADESC(CHLRHudHealthbar3)
DEFINE_THINKFUNC(TransmitData),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_KEYFIELD(targetent1, FIELD_STRING, "targetent1"),
DEFINE_FIELD(bActive, FIELD_BOOLEAN),
DEFINE_FIELD(Ent, FIELD_CLASSPTR),
END_DATADESC()
void CHLRHudHealthbar3::Spawn(void)
{
	Ent = gEntList.FindEntityByName(NULL, targetent1);
	ClearHud();
	SetThink(&CHLRHudHealthbar3::TransmitData);
	SetNextThink(gpGlobals->curtime);
}
void CHLRHudHealthbar3::InputEnable(inputdata_t &inputdata)
{
	Enable();
}

void CHLRHudHealthbar3::Enable(void)
{
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
	MessageEnd();
	Msg("enabled\n");
	SetThink(&CHLRHudHealthbar3::TransmitData);
	SetNextThink(gpGlobals->curtime);

}
void CHLRHudHealthbar3::TransmitData(void)
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
			MessageEnd();
			SetThink(NULL);
			return;
		}
	}
	SetNextThink(gpGlobals->curtime + 0.1f);
}
void CHLRHudHealthbar3::ClearHud(void)
{
	return;
}*/