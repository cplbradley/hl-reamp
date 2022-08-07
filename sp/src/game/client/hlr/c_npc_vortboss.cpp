#include "cbase.h"
#include "c_ai_basenpc.h"
#include "takedamageinfo.h"
#include "beam_shared.h"


#define VORTBOSS_EYE_ATTACHMENT 1
#define VORTBOSS_HAND_ATTACHMENT 2
#define VORTBOSS_MAX_LASER_RANGE 3600


class C_NPC_VortBoss : public C_AI_BaseNPC
{
public:
	C_NPC_VortBoss() {}
	DECLARE_CLASS(C_NPC_VortBoss, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

private:
	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t type);

	//CNetworkVector(vDmgPos);
	CBeam *pSpinBeam;
	bool bDrawSpinBeam;
	bool bBleeding;

	CNewParticleEffect *m_pBleedingFX;

	C_NPC_VortBoss(const C_NPC_VortBoss &);
};

BEGIN_DATADESC(C_NPC_VortBoss)
END_DATADESC()
IMPLEMENT_CLIENTCLASS_DT(C_NPC_VortBoss, DT_VortBoss, CNPC_VortBoss)
RecvPropBool(RECVINFO(bDrawSpinBeam)),
RecvPropBool(RECVINFO(bBleeding)),
END_RECV_TABLE()


void C_NPC_VortBoss::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if ((type == DATA_UPDATE_CREATED))
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

void C_NPC_VortBoss::ClientThink()
{
	/*if (bDrawSpinBeam)
	{
		Vector vecHandPos, m_vLaserDir;
		QAngle angHandAng;
		GetAttachment(VORTBOSS_HAND_ATTACHMENT, vecHandPos, angHandAng);
		AngleVectors(angHandAng, &m_vLaserDir);
		trace_t tr;
		m_vLaserDir[ROLL] = 0;
		UTIL_TraceLine(vecHandPos, vecHandPos + m_vLaserDir * VORTBOSS_MAX_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		if (!pSpinBeam)
		{
			pSpinBeam = CBeam::BeamCreate("sprites/laser.vmt", 16.0);
			pSpinBeam->SetRenderColor(255, 0, 30);
		}
		pSpinBeam->PointEntInit(tr.endpos, this);
		pSpinBeam->SetEndAttachment(VORTBOSS_HAND_ATTACHMENT);
		pSpinBeam->SetBrightness(255);
		pSpinBeam->SetNoise(0);
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(tr.m_pEnt);
		//DebugDrawLine(tr.startpos, tr.endpos, 0, 150, 255, false, 0.1);
		if (pBCC)
		{
			if (pBCC->IsPlayer())
			{
				//ClearMultiDamage();
				CTakeDamageInfo info(this, this, 20, DMG_SHOCK);
				//info.AdjustPlayerDamageTakenForSkillLevel();
				//info.SetDamagePosition(tr.endpos);
				//CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
				pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
				//ApplyMultiDamage();
				engine->ClientCmd_Unrestricted("vorthurt");
				DevMsg("client hit\n");
			}
		}
	}
	else
	{
		if (pSpinBeam)
		{	
			pSpinBeam->SetRenderColorA(0);
			pSpinBeam = NULL;
		}
	}*/
	if (bBleeding)
	{
		CParticleProperty * pProp = ParticleProp();
		if (!m_pBleedingFX)
		{
			m_pBleedingFX = pProp->Create("blood_vortboss_bleeding", PATTACH_ABSORIGIN_FOLLOW);
			pProp->AddControlPoint(m_pBleedingFX, 1, this, PATTACH_ABSORIGIN_FOLLOW);
		}
	}
	else if (m_pBleedingFX)
	{
		m_pBleedingFX->StopEmission();
		m_pBleedingFX = NULL;
	}
	BaseClass::ClientThink();
}