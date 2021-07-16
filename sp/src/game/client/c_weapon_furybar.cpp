#include "cbase.h"
#include "c_basecombatweapon.h"
#include "particle_property.h"



class C_WeaponFurybar : public C_BaseCombatWeapon
{
	DECLARE_CLASS(C_WeaponFurybar, C_BaseCombatWeapon);
	DECLARE_CLIENTCLASS();
public:

	C_WeaponFurybar() {};
	bool m_bShouldDrawFlames;

	virtual void ClientThink();
	virtual void	OnDataChanged(DataUpdateType_t type);
	virtual void CheckFlames();

	CNewParticleEffect *m_pFlame;
};

IMPLEMENT_CLIENTCLASS_DT(C_WeaponFurybar, DT_WeaponFuryBar, CWeaponFurybar)
RecvPropBool(RECVINFO(m_bShouldDrawFlames))
END_RECV_TABLE();

void C_WeaponFurybar::OnDataChanged(DataUpdateType_t type)
{
	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}
void C_WeaponFurybar::ClientThink()
{
	CheckFlames();
}

void C_WeaponFurybar::CheckFlames()
{
	/*if (m_bShouldDrawFlames)
	{
		if (!m_pFlame)
		{
			CBasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
			int iAttachment = ParticleProp()->GetParticleAttachment(pPlayer->GetViewModel(), "0", "crowbar_burn");
			m_pFlame = ParticleProp()->Create("crowbar_burn", PATTACH_POINT_FOLLOW, iAttachment);
		}
	}
	else
	{
		if (m_pFlame)
		{
			m_pFlame->StopEmission();
		}
	}*/
}