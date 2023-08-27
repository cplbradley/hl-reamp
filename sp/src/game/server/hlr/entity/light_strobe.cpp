#include "cbase.h"


#include "tier0/memdbgon.h"


class CStrobeLight : public CBaseEntity
{
	DECLARE_CLASS(CStrobeLight, CBaseEntity);
	DECLARE_SERVERCLASS();

public:
	void Spawn();
	void InputEnable(inputdata_t& data);
	void InputDisable(inputdata_t& data);

private:
	CNetworkVar(bool, m_bEnabled);
	CNetworkVar(float, m_fFrequency);
	CNetworkVar(int, iPattern);
	CNetworkVector(vColor);
	CNetworkVar(float, fRadius);

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(light_strobe, CStrobeLight);
BEGIN_DATADESC(CStrobeLight)
DEFINE_KEYFIELD(m_fFrequency,FIELD_FLOAT,"StrobeFrequency"),
DEFINE_KEYFIELD(vColor,FIELD_COLOR32,"LightColor"),
DEFINE_KEYFIELD(fRadius,FIELD_FLOAT,"LightRadius"),
DEFINE_FIELD(m_bEnabled,FIELD_BOOLEAN),
DEFINE_INPUTFUNC(FIELD_VOID,"Enable",InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID,"Disable",InputDisable),
DEFINE_KEYFIELD(iPattern,FIELD_STRING,"LightPattern"),
END_DATADESC()


IMPLEMENT_SERVERCLASS_ST(CStrobeLight,DT_StrobeLight)
SendPropBool(SENDINFO(m_bEnabled)),
SendPropFloat(SENDINFO(m_fFrequency)),
SendPropFloat(SENDINFO(fRadius)),
SendPropInt(SENDINFO(iPattern)),
SendPropVector(SENDINFO(vColor)),
END_SEND_TABLE()


void CStrobeLight::Spawn()
{
	m_bEnabled = false;
	SetTransmitState(FL_EDICT_ALWAYS);
	Msg("ServerSpawn\n");
}

void CStrobeLight::InputEnable(inputdata_t& data)
{
	Msg("enabling\n");
	m_bEnabled = true;
}

void CStrobeLight::InputDisable(inputdata_t& data)
{
	m_bEnabled = false;
}