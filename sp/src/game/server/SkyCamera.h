//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Resource collection entity
//
// $NoKeywords: $
//=============================================================================//

#ifndef SKYCAMERA_H
#define SKYCAMERA_H

#ifdef _WIN32
#pragma once
#endif

class CSkyCamera;
#define SF_SKY_MASTER (1 << 0)
#define SF_SKY_START_UPDATING (1 << 1)
//=============================================================================
//
// Sky Camera Class
//
class CSkyCamera : public CBaseEntity
{
	DECLARE_CLASS( CSkyCamera, CBaseEntity );
	DECLARE_SERVERCLASS();

public:

	DECLARE_DATADESC();
	CSkyCamera();
	~CSkyCamera();
	virtual void Spawn( void );
	virtual void Activate();
	virtual int UpdateTransmitState() { return SetTransmitState(FL_EDICT_ALWAYS); }
	bool AcceptInput(const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID);
	void Update();
	void InputForceUpdate(inputdata_t &inputdata);
	void InputStartUpdating(inputdata_t &inputdata);
	void InputStopUpdating(inputdata_t &inputdata);
	void InputActivateSkybox(inputdata_t &inputdata);
	void InputDeactivateSkybox(inputdata_t &inputdata);
	void InputSetFogStartDist(inputdata_t &data);
	void InputSetFogEndDist(inputdata_t &data);
	void InputTurnOnFog(inputdata_t &data);
	void InputTurnOffFog(inputdata_t &data);
	void InputSetFogColor(inputdata_t &data);
	void InputSetFogColorSecondary(inputdata_t &data);
	void InputSetFogMaxDensity(inputdata_t &inputdata);
	void InputSetFarZ(inputdata_t &data);

public:
	sky3dparams_t	m_skyboxData;
	bool			m_bUseAngles;
	bool			m_bUseAnglesForSky;
	CSkyCamera		*m_pNext;

	CNetworkVar(bool, bAmActiveSkyCamera);
	CNetworkVar(bool, bParented);
	CNetworkVar(bool, bUseAngles);
};


//-----------------------------------------------------------------------------
// Retrives the current skycamera
//-----------------------------------------------------------------------------
CSkyCamera*		GetCurrentSkyCamera();
CSkyCamera*		GetSkyCameraList();


#endif // SKYCAMERA_H
