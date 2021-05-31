#include "cbase.h"
#include "sprite.h"


#pragma once

class CHLRFloorSprite : public CBaseEntity
{

	DECLARE_CLASS(CHLRFloorSprite, CBaseEntity);

	DECLARE_DATADESC();

public:
	void Spawn();
	void Precache();
	bool InitSprite();
	void UpdatePos();
	void UpdateThink();

	CHandle<CSprite> m_pSprite;
};