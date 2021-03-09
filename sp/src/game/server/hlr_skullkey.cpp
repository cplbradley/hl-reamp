#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "convar.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "props.h"
#include "soundent.h"
#include "explode.h"
#include "physics_shared.h"
#include "game.h"
#include "movevars_shared.h"
#include "vstdlib/random.h"
#include "hl2_shareddefs.h"
#include "gamemovement.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"
#include "particle_parse.h"
#include "particle_system.h"

#include "tier0/memdbgon.h"

enum {
	KEYCOLOR_RED,
	KEYCOLOR_BLUE,
	KEYCOLOR_PURPLE,
};
class CHLRSkullkey : public CBaseAnimating
{
	DECLARE_CLASS(CHLRSkullkey, CBaseAnimating);
public:
	void Spawn(void);
	void Precache(void);
	void Pickup(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	COutputEvent m_OnPickup;
	int m_nColor;
	//void Touch(CBaseEntity *pOther);
	void CreateSprite(void);
	bool	CreateVPhysics(void);
	int ObjectCaps(void);
	CSprite *pSprite[2];

private:

	void DisplayHud(void);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_skullkey, CHLRSkullkey);

BEGIN_DATADESC(CHLRSkullkey)
DEFINE_FUNCTION(Pickup),
DEFINE_FUNCTION(Touch),
DEFINE_KEYFIELD(m_nColor, FIELD_INTEGER, "KeyColor"),
DEFINE_OUTPUT(m_OnPickup, "OnPickup"),
END_DATADESC();


void CHLRSkullkey::Spawn(void)
{
	Precache();
	SetModel("models/props/keys/skullkey/skullkey.mdl");
	UTIL_SetSize(this, -Vector(4.0f, 4.0f, 4.0f), Vector(4.0f, 4.0f, 4.0f));
	m_nSkin = m_nColor;
	SetSolid(SOLID_VPHYSICS);
	SetMoveType(MOVETYPE_NONE);
	SetUse(&CHLRSkullkey::Pickup);
	CreateSprite();
	//AddSpawnFlags(SF_PHYSPROP_ENABLE_PICKUP_OUTPUT);
	//CreateVPhysics();
	//SetTouch(&CHLRSkullkey::Touch);
	//BaseClass::Spawn();
}
int CHLRSkullkey::ObjectCaps(void)
{
	int flags = BaseClass::ObjectCaps();

	return (flags | FCAP_IMPULSE_USE);
}
void CHLRSkullkey::Precache(void)
{
	PrecacheModel("models/props/keys/skullkey/skullkey.mdl");
	PrecacheMaterial("sprites/animglow01.vmt");
}
void CHLRSkullkey::CreateSprite(void)
{
	for (int i = 0; i < 2; i++)
	{
		pSprite[i] = CSprite::SpriteCreate("sprites/animglow01.vmt", GetLocalOrigin(), false);
		if (pSprite[i] == NULL)
			return;

		if (i == 0)
		{
			int nAttachment = LookupAttachment("righteye");
			pSprite[i]->FollowEntity(this);
			pSprite[i]->SetAttachment(this, nAttachment);
			pSprite[i]->SetScale(0.1f);
			pSprite[i]->SetGlowProxySize(4.0f);
			pSprite[i]->Animate(15.0f);
		}
		if (i == 1)
		{
			int nAttachment = LookupAttachment("lefteye");
			pSprite[i]->FollowEntity(this);
			pSprite[i]->SetAttachment(this, nAttachment);
			pSprite[i]->SetScale(0.1f);
			pSprite[i]->SetGlowProxySize(4.0f);
			pSprite[i]->Animate(15.0f);
		}
		switch (m_nColor)
		{
		case KEYCOLOR_RED:
		{
			pSprite[i]->SetTransparency(kRenderGlow, 255, 0, 0, 150, kRenderFxPulseFast);

			break;
		}
		case KEYCOLOR_BLUE:
		{
			pSprite[i]->SetTransparency(kRenderGlow, 0, 0, 255, 150, kRenderFxPulseFast);
			break;
		}
		case KEYCOLOR_PURPLE:
			pSprite[i]->SetTransparency(kRenderGlow, 100, 0, 255, 150, kRenderFxPulseFast);
			break;

		default:
			break;
		}
	}
}
/*void CHLRSkullkey::Touch(CBaseEntity *pOther)
{
if (pOther->IsPlayer())
{
/*switch (m_nColor)
{
case KEYCOLOR_RED:
{
//engine->ClientCommand(pOther->edict(),"show_red 1");

break;
}
case KEYCOLOR_BLUE:
{
break;
}
case KEYCOLOR_PURPLE:
break;

default:
break;
}
m_OnPickup.FireOutput(pOther, this);
SetTouch(NULL);
SetModelName(NULL_STRING);
SetSolid(SOLID_NONE);
SetSolidFlags(FSOLID_NOT_SOLID);
DisplayHud();
}
}*/
void CHLRSkullkey::Pickup(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_OnPickup.FireOutput(pActivator, this);
	SetTouch(NULL);
	SetUse(NULL);
	SetModelName(NULL_STRING);
	SetSolid(SOLID_NONE);
	SetSolidFlags(FSOLID_NOT_SOLID);
	DisplayHud();
	for (int i = 0; i < 2; i++)
	{
		pSprite[i]->TurnOff();
		pSprite[i]->SetRenderColorA(0);
		pSprite[i] = NULL;
		UTIL_Remove(pSprite[i]);
	}
	RemoveDeferred();
}
bool CHLRSkullkey::CreateVPhysics(void)
{
	return BaseClass::CreateVPhysics();
}

void CHLRSkullkey::DisplayHud(void)
{
	switch (m_nColor)
	{
	case KEYCOLOR_RED:
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "RedSkullKey");
		WRITE_BOOL(true);
		MessageEnd();
		break;
	}
	case KEYCOLOR_BLUE:
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "BlueSkullKey");
		WRITE_BOOL(true);
		MessageEnd();
		break;
	}
	case KEYCOLOR_PURPLE:
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "PurpleSkullKey");
		WRITE_BOOL(true);
		MessageEnd();
		break;
	}
	default:
		break;
	}
}