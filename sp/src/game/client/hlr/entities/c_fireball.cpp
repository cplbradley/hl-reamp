#include "cbase.h"
#include "dlight.h"
#include "iefx.h"
#include "c_baseanimating.h"

#include "tier0/memdbgon.h"


class C_ShooterFireball : public C_BaseAnimating
{
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS(C_ShooterFireball, C_BaseAnimating);
	DECLARE_DATADESC();

public:
	C_ShooterFireball() {}
	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink();
private:
	bool bUseDLight;
	dlight_t* dlight;
};

IMPLEMENT_CLIENTCLASS_DT(C_ShooterFireball,DT_Fireball,CShooterFireball)
RecvPropBool(RECVINFO(bUseDLight)),
END_RECV_TABLE()

BEGIN_DATADESC(C_ShooterFireball)
END_DATADESC()


void C_ShooterFireball::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

void C_ShooterFireball::ClientThink()
{
	if (bUseDLight)
	{
		if (!dlight)
		{
			dlight = effects->CL_AllocDlight(index);
			dlight->color.r = 255;
			dlight->color.g = 140;
			dlight->color.b = 0;
			dlight->color.exponent = 2;
			dlight->radius = 512.f;
		}
		dlight->radius = RandomFloat(450.f, 600.f);
		dlight->origin = GetAbsOrigin();
		dlight->die = gpGlobals->curtime + 0.1f;
	}

	BaseClass::ClientThink();
}