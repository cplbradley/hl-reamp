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
class CHLRKeycard : public CBaseAnimating
{
	DECLARE_CLASS(CHLRKeycard, CBaseAnimating);
	DECLARE_SERVERCLASS();
public:
	int UpdateTransmitState()	// always send to all clients
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}

	void Spawn(void);
	void Precache(void);
	void Pickup(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	COutputEvent m_OnPickup;
	CNetworkVar(int, m_nColor);
	CNetworkVar(bool, m_bDrawHud);
	//void Touch(CBaseEntity *pOther);
	void CreateSprite(void);
	bool	CreateVPhysics(void);
	int ObjectCaps(void);
	CHandle<CSprite> pSprite;
	//void OnRestore();

private:

	//void DisplayHud(void);

	DECLARE_DATADESC();
};