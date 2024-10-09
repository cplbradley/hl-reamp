#include "cbase.h"
#include "sprite.h"
#include "baseanimating.h"
#include "player.h"
#include "spritetrail.h"
#include "hlr_floorsprite.h"
#include "interpolatortypes.h"



#include "tier0/memdbgon.h"

#define SPRITE_MATERIAL "sprites/floorsprite.vmt"


LINK_ENTITY_TO_CLASS(hlr_floorsprite, CHLRFloorSprite);
PRECACHE_REGISTER(hlr_floorsprite);

BEGIN_DATADESC(CHLRFloorSprite)
DEFINE_FIELD(m_pSprite, FIELD_EHANDLE),
DEFINE_THINKFUNC(UpdateThink),
DEFINE_FUNCTION(DrawSprite),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CHLRFloorSprite,DT_FloorSprite)
END_SEND_TABLE()


void CHLRFloorSprite::Spawn(void)
{
	Precache();
	DrawSprite();
	SetThink(&CHLRFloorSprite::UpdateThink);
	SetNextThink(gpGlobals->curtime);
	DevMsg("floorsprite spawned\n");
}
void CHLRFloorSprite::Precache(void)
{
	PrecacheModel(SPRITE_MATERIAL);
}
bool CHLRFloorSprite::InitSprite(void)
{
	if (!m_pSprite)
	{
		m_pSprite = CSprite::SpriteCreate(SPRITE_MATERIAL, GetAbsOrigin(), false);
		m_pSprite->FollowEntity(this);
		m_pSprite->SetSpriteScale(0.5f);
		m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
		m_pSprite->SetGlowProxySize(16.0f);
	}
	return true;
}
void CHLRFloorSprite::DrawSprite(void)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;

	InitSprite();
}
void CHLRFloorSprite::UpdateThink(void)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	if (!pPlayer)
		return;

	if (!m_pSprite)
		return;
	if (pPlayer->GetGroundEntity() == NULL)
		spriteAlpha+= 4;
	else
		spriteAlpha-= 4;
	if (spriteAlpha < 1)
		spriteAlpha = 1;
	if (spriteAlpha > 64)
		spriteAlpha = 64;
	m_pSprite->SetRenderColorA(spriteAlpha);
	SetNextThink(gpGlobals->curtime);
}