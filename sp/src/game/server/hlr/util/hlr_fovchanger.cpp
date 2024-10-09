#include "cbase.h"
#include "player.h"




#include "tier0/memdbgon.h"



class CHLRFOVChanger : public CPointEntity
{
	DECLARE_CLASS(CHLRFOVChanger, CPointEntity);
	DECLARE_DATADESC();

public:
	void Spawn();
	void ChangeFOV(int iDestFOV, float fLerpTime);
	void ResetFOV();
	float m_fLerpTime;

	void InputSetFOV(inputdata_t& inputdata);
	void InputSetLerp(inputdata_t& inputdata);
};

LINK_ENTITY_TO_CLASS(hlr_fovchanger, CHLRFOVChanger);

BEGIN_DATADESC(CHLRFOVChanger)
DEFINE_INPUTFUNC(FIELD_FLOAT,"ChangeFOV",InputSetFOV),
END_DATADESC()


void CHLRFOVChanger::Spawn()
{

}

void CHLRFOVChanger::ChangeFOV(int iDestFOV, float fLerpTime)
{
	CBasePlayer* player = UTIL_GetLocalPlayer();
	ConVarRef fov_desired("fov_desired");
	if (!player)
		return;
	int fov = iDestFOV;
	if (fov == 0)
	{
		fov = fov_desired.GetFloat();
		SetThink(&CHLRFOVChanger::ResetFOV);
		SetNextThink(gpGlobals->curtime + fLerpTime);
	}

	player->SetFOV(this, (int)fov, fLerpTime);
}

void CHLRFOVChanger::InputSetFOV(inputdata_t& inputdata)
{
	
	float destFOV = inputdata.value.Int();

	ChangeFOV(destFOV, m_fLerpTime);
}

void CHLRFOVChanger::InputSetLerp(inputdata_t& inputdata)
{
	m_fLerpTime = inputdata.value.Float();
}

void CHLRFOVChanger::ResetFOV()
{
	CBasePlayer* player = UTIL_GetLocalPlayer();
	if (!player)
		return;

	player->SetFOV(this, 0, 0.f);
}