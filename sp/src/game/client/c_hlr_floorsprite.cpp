#include "cbase.h"


class C_HLRFloorSprite : public C_BaseEntity
{
public: 
	C_HLRFloorSprite() {};

	DECLARE_CLASS(C_HLRFloorSprite, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink();
	virtual void UpdatePos();

private:
	C_HLRFloorSprite(const C_HLRFloorSprite &);
};


BEGIN_DATADESC(C_HLRFloorSprite)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_HLRFloorSprite,DT_FloorSprite,CHLRFloorSprite)
END_RECV_TABLE()


void C_HLRFloorSprite::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if ((type == DATA_UPDATE_CREATED))
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

void C_HLRFloorSprite::ClientThink()
{
	UpdatePos();
}
void C_HLRFloorSprite::UpdatePos()
{
	CBasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
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



