#include "cbase.h"
#include "player.h"
#include "triggers.h"
#include "baseentity.h"


//you know the rules
#include "tier0/memdbgon.h"



class CHLRSpeedMod : public CPointEntity
{
	DECLARE_CLASS(CHLRSpeedMod, CPointEntity);
	DECLARE_DATADESC();

public:
	void Spawn();
	void SetPlayerSpeed(float speed);
	void InputSetMaxSpeed(inputdata_t &data);
};

LINK_ENTITY_TO_CLASS(hlr_speedmod, CHLRSpeedMod);
BEGIN_DATADESC(CHLRSpeedMod)
DEFINE_INPUTFUNC(FIELD_FLOAT,"SetPlayerSpeed",InputSetMaxSpeed),
END_DATADESC();


void CHLRSpeedMod::Spawn()
{
	BaseClass::Spawn();
}

void CHLRSpeedMod::InputSetMaxSpeed(inputdata_t &data)
{
	SetPlayerSpeed(data.value.Float());
	Msg("SpeedMod: %f\n", data.value.Float());
}

void CHLRSpeedMod::SetPlayerSpeed(float speed)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
		return;

	ConVarRef hl2_normspeed("hl2_normspeed");
	float normspeed = hl2_normspeed.GetFloat();
	pPlayer->SetMaxSpeed(speed * normspeed);
}




