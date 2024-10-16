//#pragma once
#ifndef FMOD_DYNAMIC_PLAYER_H
#define FMOD_DYNAMIC_PLAYER_H

#include "cbase.h"
#include "KeyValues.h"
#include "string_t.h"
#ifdef CLIENT_DLL
#include "c_baseentity.h"
#else
#include "baseentity.h"
#endif

#ifdef CLIENT_DLL
#define CFMODDynamicPlayer C_FMODDynamicPlayer
#endif

struct musicscript_t
{
	char szMusicType[MAX_PATH];
	char szMusicPath[MAX_PATH];
};

enum MusicType
{
	MUSICTYPE_NONE = 0,
	MUSICTYPE_LIGHT,
	MUSICTYPE_MEDIUM,
	MUSICTYPE_HEAVY,
	MUSICTYPE_TRANSITION_LIGHT,
	MUSICTYPE_TRANSITION_MEDIUM,
	MUSICTYPE_TRANSITION_HEAVY
};

enum
{
	SIGNAL_RESET,
	SIGNAL_PRINT,
	SIGNAL_PARSE,
	SIGNAL_TRANSITION,
	SIGNAL_STOP,
	SIGNAL_PLAY,
	SIGNAL_RANDOMPATH,
};

class CFMODDynamicPlayer : public CBaseEntity
{
	DECLARE_CLASS(CFMODDynamicPlayer, CBaseEntity);
	DECLARE_NETWORKCLASS();
	

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
public:
	void Spawn(void);
	int GetCurrentMusicType() { return m_iMusicType; }
	void TransitionToMusicType(int musictype);
	void PickRandomMusicStem();
	void PlaySpecificSound();
	

	void PrintToConsole(inputdata_t &inputdata);
	void RandomMusicPath(inputdata_t& inputdata);
	void InputPlaySpecificSound(inputdata_t &inputdata);
	void InputTransitionToHeavyMusic(inputdata_t &inputdata);
	void InputTransitionToMediumMusic(inputdata_t &inputdata);
	void InputTransitionToLightMusic(inputdata_t &inputdata);
	void InputImmediateHeavyMusic(inputdata_t &inputdata);
	void InputImmediateMediumMusic(inputdata_t& inputdata);
	void EnablePlayer(inputdata_t &inputdata);
	void ParseScript(inputdata_t& inputdata);
	void PrintScriptToConsole();
	void CheckMusicFile();
	void fuckit();
#ifdef GAME_DLL
	int UpdateTransmitState();
	void TransmitMessage(int message);
	
#endif

#ifdef CLIENT_DLL
	void ReceiveMessage(int classID, bf_read& msg);
	void ParseMusicScript(const char* pKeyName);
	void ReadMusicDirectory(KeyValues* KeyValue);
	const char* GetFixedPathToMusic(const char* path);
	void PickRandomLightMusic();
	void PickRandomMediumMusic();
	void PickRandomHeavyMusic();
	void PickLightTransition();
	void PickMediumTransition();
	void PickHeavyTransition();
	void PostDataUpdate(DataUpdateType_t updateType);

	void PrintRandomPath();

	
	
	const char* GetPathToMusic();
	const char* GetMusicTypeForScriptParse();

	virtual void ClientThink();

	int m_iPrevMusicType;
#endif

	CNetworkString(scriptpath, 512);
	CNetworkVar(int, m_iMusicType);

	
	
private:
	

#ifdef CLIENT_DLL
	int lastrand;
	const char* musicpath[512];
	CUtlVector<musicscript_t> m_musiclist;
#endif
};

extern CFMODDynamicPlayer* FMODPlayer();

#endif //FMOD_DYNAMIC_PLAYER_H