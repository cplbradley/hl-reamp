#include "cbase.h"
#include "sprite.h"
#include "baseanimating.h"
#include "player.h"
#include "hlr_floorsprite.h"




#include "tier0/memdbgon.h"

#define SPRITE_MATERIAL "sprites/floorsprite.vmt"

LINK_ENTITY_TO_CLASS(hlr_floorsprite, CHLRFloorSprite);

BEGIN_DATADESC(CHLRFloorSprite)
DEFINE_THINKFUNC(UpdateThink),
DEFINE_FUNCTION(UpdatePos),
END_DATADESC()


void CHLRFloorSprite::Spawn(void)
{
	Precache();
	InitSprite();
	SetThink(&CHLRFloorSprite::UpdateThink);
	SetNextThink(gpGlobals->curtime);
	DevMsg("floorsprite spawned\n");
}
void CHLRFloorSprite::Precache(void)
{
	PrecacheMaterial(SPRITE_MATERIAL);
}
bool CHLRFloorSprite::InitSprite(void)
{
	m_pSprite = CSprite::SpriteCreate(SPRITE_MATERIAL, GetAbsOrigin(), false);
	m_pSprite->FollowEntity(this);
	m_pSprite->SetSpriteScale(1.0f);
	m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 64, kRenderFxNoDissipation);
	m_pSprite->SetGlowProxySize(16.0f);
	return true;
}
void CHLRFloorSprite::UpdatePos(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	Vector vecPlayerPos = pPlayer->GetAbsOrigin();
	trace_t tr;
	UTIL_TraceLine(vecPlayerPos, vecPlayerPos + (Vector(0, 0, -1) * MAX_TRACE_LENGTH), MASK_PLAYERSOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_PLAYER_MOVEMENT, &tr);
	Vector vecTgtPos = Vector(vecPlayerPos.x, vecPlayerPos.y, tr.endpos.z);
	SetAbsOrigin(vecTgtPos + Vector(0, 0, 4));
	Vector vecDown = Vector(0, 0, 1);
	QAngle angDown;
	VectorAngles(vecDown, angDown);
	SetAbsAngles(angDown);
}
void CHLRFloorSprite::UpdateThink(void)
{
	UpdatePos();
	SetNextThink(gpGlobals->curtime);
}