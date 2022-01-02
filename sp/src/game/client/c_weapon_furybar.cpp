#include "cbase.h"
#include "c_basecombatweapon.h"
#include "particle_property.h"
#include "c_basehlcombatweapon.h"
#include "particles_new.h"


class C_WeaponFurybar : public C_BaseHLBludgeonWeapon
{
	DECLARE_CLASS(C_WeaponFurybar, C_BaseHLBludgeonWeapon);
	DECLARE_CLIENTCLASS();
public:

	C_WeaponFurybar() {};
	bool m_bShouldDrawFlames;

	virtual void ClientThink();
	virtual void	OnDataChanged(DataUpdateType_t type);
	virtual void CheckFlames();

	CNewParticleEffect *m_pFlame;
};

IMPLEMENT_CLIENTCLASS_DT(C_WeaponFurybar,DT_WeaponFurybar,CWeaponFurybar)
RecvPropBool(RECVINFO(m_bShouldDrawFlames)),
END_RECV_TABLE()

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
	if (m_bShouldDrawFlames)
	{
		/*CBasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();
		C_BaseViewModel *pvm = pPlayer->GetViewModel();
		Vector vStart, vEnd;
		int iAttachment1 = pvm->LookupAttachment("0");
		int iAttachment2 = pvm->LookupAttachment("1");
		pvm->GetAttachment(iAttachment1, vStart);
		pvm->GetAttachment(iAttachment2, vEnd);

		if (!m_pFlame)
		{
			m_pFlame = ParticleProp()->Create("furybar_burn", PATTACH_CUSTOMORIGIN, 0);
		}
		m_pFlame->SetControlPoint(0, vStart);
		m_pFlame->SetControlPoint(1, vEnd);*/

	}
	else
	{
		if (m_pFlame)
		{
			//m_pFlame->StopEmission();
		}
	}
}