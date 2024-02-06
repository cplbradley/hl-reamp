#include "cbase.h"
#include "iefx.h"
#include "dlight.h"

class C_StrobeLight : public C_BaseEntity
{
	DECLARE_CLASS(C_StrobeLight, C_BaseEntity);
	DECLARE_CLIENTCLASS();
public:
	virtual void OnDataChanged(DataUpdateType_t type);
	void UpdateLight();
	virtual void ClientThink();

private:
	bool m_bEnabled;
	float m_fFrequency;
	float fRadius;
	Vector vColor;
	int iPattern;
	int curtoken;
	float fNextUpdate;
	char* str;
};


IMPLEMENT_CLIENTCLASS_DT(C_StrobeLight,DT_StrobeLight,CStrobeLight)
RecvPropBool(RECVINFO(m_bEnabled)),
RecvPropFloat(RECVINFO(m_fFrequency)),
RecvPropFloat(RECVINFO(fRadius)),
RecvPropInt(RECVINFO(iPattern)),
RecvPropVector(RECVINFO(vColor)),
END_RECV_TABLE()


void C_StrobeLight::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
		DevMsg("Strobelight ClientSpawn\n");
		fNextUpdate = gpGlobals->curtime;
	}
}

void C_StrobeLight::ClientThink()
{
	if (m_bEnabled)
	{
		UpdateLight();
	}
}

void C_StrobeLight::UpdateLight()
{
	if (gpGlobals->curtime > fNextUpdate)
	{
		fNextUpdate = gpGlobals->curtime + m_fFrequency;
		Msg("Strobelight Creating light\n");
		dlight_t* dl = effects->CL_AllocDlight(index);
		dl->origin = GetAbsOrigin();
		dl->color.r = 255;
		dl->color.g = 255;
		dl->color.b = 255;
		dl->die = gpGlobals->curtime + (m_fFrequency * 0.5f);
		dl->radius = fRadius * RandomInt(0,1);
	}
}