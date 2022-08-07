#include "cbase.h"
#include "fmod/fmod_dynamic_player.h"
#ifdef CLIENT_DLL
#include "fmod/fmod_system.h"
#endif
#include "filesystem.h"
#include "KeyValues.h"
#include "string_t.h"

#include "memdbgon.h"

CFMODDynamicPlayer fmodPlayer;
CFMODDynamicPlayer* FMODPlayer() {
	return &fmodPlayer;
}
IMPLEMENT_NETWORKCLASS_ALIASED(FMODDynamicPlayer, DT_FMODDynamicPlayer)

BEGIN_NETWORK_TABLE(CFMODDynamicPlayer, DT_FMODDynamicPlayer)
#ifndef CLIENT_DLL
SendPropStringT(SENDINFO(scriptpath)),
SendPropInt(SENDINFO(m_iMusicType))
#else
RecvPropString(RECVINFO(scriptpath)),
RecvPropInt(RECVINFO(m_iMusicType))
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(fmod_dynamic_player, CFMODDynamicPlayer);

#ifdef GAME_DLL
BEGIN_DATADESC(CFMODDynamicPlayer)
DEFINE_KEYFIELD(scriptpath,FIELD_STRING,"ScriptPath"),
DEFINE_INPUTFUNC(FIELD_VOID,"PrintToConsole",PrintToConsole),
DEFINE_INPUTFUNC(FIELD_VOID,"ParseScript",ParseScript),
END_DATADESC()
#endif



void CFMODDynamicPlayer::Spawn(void)
{
	m_iMusicType = MUSICTYPE_MEDIUM;
#ifdef GAME_DLL
	Warning("SERVERSIDE SCRIPT PATH = %s\n", scriptpath.Get());
#endif
}
void CFMODDynamicPlayer::InputTransitionToLightMusic(inputdata_t& inputdata)
{
	m_iMusicType = MUSICTYPE_TRANSITION_LIGHT;
}
void CFMODDynamicPlayer::InputTransitionToMediumMusic(inputdata_t& inputdata)
{
	m_iMusicType = MUSICTYPE_TRANSITION_MEDIUM;
}
void CFMODDynamicPlayer::InputTransitionToHeavyMusic(inputdata_t& inputdata)
{
	m_iMusicType = MUSICTYPE_TRANSITION_HEAVY;
}
void CFMODDynamicPlayer::InputImmediateHeavyMusic(inputdata_t &inputdata)
{
	m_iMusicType = MUSICTYPE_HEAVY;
}


void CFMODDynamicPlayer::PickRandomMusicStem()
{
#ifdef CLIENT_DLL
	/*switch (m_iMusicType)
	{
	case MUSICTYPE_LIGHT:
		PickRandomLightMusic();
		break;
	case MUSICTYPE_MEDIUM:
		PickRandomMediumMusic();
		break;
	case MUSICTYPE_HEAVY:
		PickRandomHeavyMusic();
		break;
	default:
		break;
	}*/
#endif
}

#ifdef CLIENT_DLL
void CFMODDynamicPlayer::PostDataUpdate(DataUpdateType_t updateType)
{
	BaseClass::PostDataUpdate(updateType);
	 
	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
		ParseMusicScript("MediumMusic");
		Warning("CLIENTSIDE SCRIPTPATH = %s\n", scriptpath.Get());
	}
}
void CFMODDynamicPlayer::ParseMusicScript(const char* pKeyName)
{
	Warning("Parsing Music Script\n");
	KeyValues* pKV = new KeyValues("MusicScript");

	if (!pKV->LoadFromFile(filesystem, scriptpath, "MOD"))
	{
		Warning("FMOD FAILED TO LOAD SCRIPT FILE. ABORTING.\n");
		pKV->deleteThis();
	}
	Warning("Script parsed successfully i guess?\n");
	KeyValues* pKVSub;

	if (pKeyName)
	{
		pKVSub = pKV->FindKey(pKeyName);
		ReadMusicDirectory(pKVSub);
	}
	pKV->deleteThis();
}

void CFMODDynamicPlayer::ReadMusicDirectory(KeyValues* KeyValue)
{
	if (!KeyValue)
	{
		Warning("FMOD COULDN'T READ SCRIPT\n");
		return;
	}

	KeyValues* pKVScript = KeyValue->GetFirstSubKey();

	while (pKVScript)
	{
		musicscript_t musicscript;
		m_musiclist.AddToTail(musicscript);
		Q_strcpy(musicscript.szMusicType, pKVScript->GetName());
		Q_strcpy(musicscript.szMusicPath, pKVScript->GetString());
		pKVScript = pKVScript->GetNextKey();
		
		Msg("%s\n", musicscript.szMusicPath);
		Msg("Fixed path %s\n", GetFixedPathToMusic(musicscript.szMusicPath));
	}

}
const char* CFMODDynamicPlayer::GetFixedPathToMusic(const char* path)
{

	char* fixedpath = new char[512];
	Q_snprintf(fixedpath, 512, "%s/sound%s/", engine->GetGameDirectory(), path);
	for (int i = 0; i < 512; i++)
	{
		if (fixedpath[i] == '\\')
			fixedpath[i] = '/';
	}

	return fixedpath;

}

const char* CFMODDynamicPlayer::GetPathToMusic()
{
	int rndm = RandomInt(0, m_musiclist.Count());
	const char* musicpath = m_musiclist[rndm].szMusicPath;
	return GetFixedPathToMusic(musicpath);
}
#endif
void CFMODDynamicPlayer::PrintToConsole(inputdata_t& inputdata)
{
	PrintScriptToConsole();
}
void CFMODDynamicPlayer::PrintScriptToConsole()
{
#ifdef CLIENT_DLL
	for (int i = 0; i < m_musiclist.Count(), i++;)
	{
		Msg("%s\n", m_musiclist[i].szMusicPath);
	}
#endif
}
void CFMODDynamicPlayer::ParseScript(inputdata_t &inputdata)
{
#ifdef CLIENT_DLL
	ParseMusicScript("MediumMusic");
#endif
}
#ifdef GAME_DLL
int CFMODDynamicPlayer::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}
#endif

