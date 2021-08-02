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
#include "hlr_keycard.h"
#include "tier0/memdbgon.h"



LINK_ENTITY_TO_CLASS(hlr_keycard, CHLRKeycard);

BEGIN_DATADESC(CHLRKeycard)
DEFINE_FUNCTION(Pickup),
DEFINE_FUNCTION(Touch),
DEFINE_KEYFIELD(m_nColor, FIELD_INTEGER, "KeyColor"),
DEFINE_FIELD(m_bDrawHud, FIELD_BOOLEAN),
DEFINE_OUTPUT(m_OnPickup,"OnPickup"),
END_DATADESC();

IMPLEMENT_SERVERCLASS_ST(CHLRKeycard, DT_Keycard)
SendPropBool(SENDINFO(m_bDrawHud)),
SendPropInt(SENDINFO(m_nColor)),
END_SEND_TABLE()


void CHLRKeycard::Spawn(void)
{
	Precache();
	SetModel("models/props/keys/keycard/keycard.mdl");
	UTIL_SetSize(this, -Vector(4.0f, 4.0f, 4.0f), Vector(4.0f, 4.0f, 4.0f));
	m_nSkin = m_nColor;
	SetSolid(SOLID_VPHYSICS);
	SetMoveType(MOVETYPE_NONE);
	SetUse(&CHLRKeycard::Pickup);
	CreateSprite();
	m_bDrawHud = false;
	//AddSpawnFlags(SF_PHYSPROP_ENABLE_PICKUP_OUTPUT);
	//CreateVPhysics();
	//SetTouch(&CHLRKeycard::Touch);
	//BaseClass::Spawn();
}
int CHLRKeycard::ObjectCaps(void)
{
	int flags = BaseClass::ObjectCaps();

	return (flags | FCAP_IMPULSE_USE);
}
void CHLRKeycard::Precache(void)
{
	PrecacheModel("models/props/keys/keycard/keycard.mdl");
	PrecacheMaterial("sprites/animglow01.vmt");
}
void CHLRKeycard::CreateSprite(void)
{
	pSprite = CSprite::SpriteCreate("sprites/animglow01.vmt", GetLocalOrigin(), false);
	if (pSprite != NULL)
	{
		int nAttachment = LookupAttachment("sprite");
		pSprite->FollowEntity(this);
		pSprite->SetAttachment(this, nAttachment);
		pSprite->SetScale(0.1f);
		pSprite->SetGlowProxySize(4.0f);
		pSprite->Animate(15.0f);
		switch (m_nColor)
		{
		case KEYCOLOR_RED:
		{
			pSprite->SetTransparency(kRenderGlow, 255, 0, 0, 150, kRenderFxPulseFast);

			break;
		}
		case KEYCOLOR_BLUE:
		{
			pSprite->SetTransparency(kRenderGlow, 0, 0, 255, 150, kRenderFxPulseFast);
			break;
		}
		case KEYCOLOR_PURPLE:
			pSprite->SetTransparency(kRenderGlow, 100, 0, 255, 150, kRenderFxPulseFast);
			break;

		default:
			break;
		}
	}
}
/*void CHLRKeycard::Touch(CBaseEntity *pOther)
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
void CHLRKeycard::Pickup(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_OnPickup.FireOutput(pActivator, this);
	m_bDrawHud = true;
	SetTouch(NULL);
	SetUse(NULL);
	SetModelName(NULL_STRING);
	SetSolid(SOLID_NONE);
	SetSolidFlags(FSOLID_NOT_SOLID);
	//DisplayHud();
	//pSprite->TurnOff();
	//pSprite->SetRenderColorA(0);
	pSprite = NULL;
	UTIL_Remove(pSprite);
	//RemoveDeferred();
}
bool CHLRKeycard::CreateVPhysics(void)
{
	return BaseClass::CreateVPhysics();
}

/*void CHLRKeycard::DisplayHud(void)
{
	switch (m_nColor)
	{
	case KEYCOLOR_RED:
	{
		/*CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "RedKey");
		WRITE_BOOL(m_bDrawHud);
		MessageEnd();
		m_bDrawHud = true;
		break;
	}
	case KEYCOLOR_BLUE:
	{
		/*CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "RedKey");
		WRITE_BOOL(m_bDrawHud);
		MessageEnd();
		m_bDrawHud = true;
		break;
	}
	case KEYCOLOR_PURPLE:
	{
		/*CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		user.MakeReliable();
		UserMessageBegin(user, "RedKey");
		WRITE_BOOL(m_bDrawHud);
		MessageEnd();
		m_bDrawHud = true;
		break;
	}
	default:
		break;
	}
}*/

/*void CHLRKeycard::OnRestore()
{
	BaseClass::OnRestore();
	DisplayHud();
}*/
