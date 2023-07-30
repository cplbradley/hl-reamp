#include "cbase.h"

class C_ActualBullet : public C_BaseEntity
{
	DECLARE_CLASS(C_ActualBullet, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

public: 
	C_ActualBullet() {};

	int m_iBulletType;

	virtual void OnDataChanged(DataUpdateType_t type);
	void Touch(CBaseEntity *pOther);
	//virtual int DrawModel(int flags);
};

BEGIN_DATADESC(C_ActualBullet)
END_DATADESC()

IMPLEMENT_CLIENTCLASS_DT(C_ActualBullet,DT_ActualBullet,CActualBullet)
RecvPropInt(RECVINFO(m_iBulletType))
END_RECV_TABLE()



void C_ActualBullet::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
		SetTouch(&C_ActualBullet::Touch);
	}
}

void C_ActualBullet::Touch(CBaseEntity *pOther)
{
	if (!pOther)
		return;
	if (!pOther->IsSolid())
		return;
	if (GetOwnerEntity() && GetOwnerEntity() == pOther)
		return;
	if (pOther->GetSolidFlags() & (FSOLID_TRIGGER | FSOLID_NOT_SOLID))
		return;

	Vector vecDir = GetAbsVelocity().Normalized();
	SetTouch(NULL);
	SetSolid(SOLID_NONE);
	SetSolidFlags(FSOLID_NOT_SOLID);
	/*const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	trace_t *pNewTrace = const_cast<trace_t*>(pTrace);*/
	DevMsg("bullet touched");
	FireBulletsInfo_t info;
	info.m_iAmmoType = m_iBulletType;
	info.m_vecSrc = (GetAbsOrigin() - (vecDir * 64));
	info.m_iShots = 1;
	info.m_iTracerFreq = 0;
	info.m_vecDirShooting = vecDir;
	FireBullets(info);
}
/*int C_ActualBullet::DrawModel(int flags)
{
	return 0;
}*/