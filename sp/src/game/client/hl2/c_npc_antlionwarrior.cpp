//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Client side antlion guard. Used to create dlight for the cave guard.
//
//=============================================================================

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "dlight.h"
#include "iefx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// When enabled, add code to have the antlion bleed profusely as it is badly injured.
#define ANTLIONWARRIOR_BLOOD_EFFECTS 2



class C_NPC_AntlionWarrior : public C_AI_BaseNPC
{
public:
	C_NPC_AntlionWarrior() {}

	DECLARE_CLASS(C_NPC_AntlionWarrior, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink();
	//virtual void DispatchFlame();

private:

	bool m_bCavernBreed;
	bool m_bInCavern;


	unsigned char m_iBleedingLevel; //< the version coming from the server
	unsigned char m_iPerformingBleedingLevel; //< the version we're currently performing (for comparison to one above)
	CNewParticleEffect *m_pBleedingFX;

	CNewParticleEffect *m_pFlame;

	/// update the hemorrhage particle effect
	virtual void UpdateBleedingPerformance(void);

	C_NPC_AntlionWarrior(const C_NPC_AntlionWarrior &);
};


//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC(C_NPC_AntlionWarrior)
END_DATADESC()


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT(C_NPC_AntlionWarrior, DT_NPC_AntlionWarrior, CNPC_AntlionWarrior)
RecvPropBool(RECVINFO(m_bCavernBreed)),
RecvPropBool(RECVINFO(m_bInCavern)),
RecvPropInt(RECVINFO(m_iBleedingLevel)),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_AntlionWarrior::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}



	if (m_iBleedingLevel != m_iPerformingBleedingLevel)
	{
		UpdateBleedingPerformance();
	}


}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_AntlionWarrior::UpdateBleedingPerformance()
{
	// get my particles
	CParticleProperty * pProp = ParticleProp();

	// squelch the prior effect if it exists
	if (m_pBleedingFX)
	{
		pProp->StopEmission(m_pBleedingFX);
		m_pBleedingFX = NULL;
	}

	// kick off a new effect
	switch (m_iBleedingLevel)
	{
	case 1: // light bleeding
	{
		m_pBleedingFX = pProp->Create("blood_antlionguard_injured_light", PATTACH_ABSORIGIN_FOLLOW);
		AssertMsg1(m_pBleedingFX, "Particle system couldn't make %s", "blood_antlionguard_injured_light");
		if (m_pBleedingFX)
		{
			pProp->AddControlPoint(m_pBleedingFX, 1, this, PATTACH_ABSORIGIN_FOLLOW);
		}
	}
	break;

	case 2: // severe bleeding
	{
		m_pBleedingFX = pProp->Create("blood_antlionguard_injured_heavy", PATTACH_ABSORIGIN_FOLLOW);
		AssertMsg1(m_pBleedingFX, "Particle system couldn't make %s", "blood_antlionguard_injured_heavy");
		if (m_pBleedingFX)
		{
			pProp->AddControlPoint(m_pBleedingFX, 1, this, PATTACH_ABSORIGIN_FOLLOW);
		}

	}
	break;
	}

	m_iPerformingBleedingLevel = m_iBleedingLevel;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_AntlionWarrior::ClientThink()
{
	if (!m_pFlame)
	{
		m_pFlame = ParticleProp()->Create("antlionwarrior_burn", PATTACH_ABSORIGIN_FOLLOW);
	}
	
	
	BaseClass::ClientThink();
}
