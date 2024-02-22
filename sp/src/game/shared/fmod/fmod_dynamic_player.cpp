#include "cbase.h"
#include "fmod/fmod_dynamic_player.h"
#include "filesystem.h"
#include "KeyValues.h"
//#include "string_t.h"
#ifdef CLIENT_DLL
#include "c_baseentity.h"
#else
#include "baseentity.h"
#endif


#include "memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(FMODDynamicPlayer, DT_FMODDynamicPlayer)
BEGIN_NETWORK_TABLE_NOBASE(CFMODDynamicPlayer, DT_FMODDynamicPlayer)

#ifndef CLIENT_DLL
SendPropStringT(SENDINFO(scriptpath)),
SendPropInt(SENDINFO(m_iMusicType))
#else
RecvPropString(RECVINFO(scriptpath)),
RecvPropInt(RECVINFO(m_iMusicType))
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(fmod_dynamic_player, CFMODDynamicPlayer);

#ifndef CLIENT_DLL
BEGIN_DATADESC(CFMODDynamicPlayer)
DEFINE_KEYFIELD(scriptpath,FIELD_STRING,"ScriptPath"),
DEFINE_INPUTFUNC(FIELD_VOID,"PrintToConsole",PrintToConsole),
DEFINE_INPUTFUNC(FIELD_VOID,"ParseScript",ParseScript),
DEFINE_INPUTFUNC(FIELD_VOID,"TransitionToHeavyMusic", InputTransitionToHeavyMusic),
DEFINE_INPUTFUNC(FIELD_VOID,"ImmedateHeavyMusic", InputImmediateHeavyMusic),
DEFINE_INPUTFUNC(FIELD_VOID,"ImmediateMediumMusic", InputImmediateMediumMusic),
DEFINE_INPUTFUNC(FIELD_VOID,"RandomMusicPath",RandomMusicPath),
END_DATADESC()

#endif

void CFMODDynamicPlayer::Spawn(void)
{
#ifdef GAME_DLL
	UpdateTransmitState();
	Warning("SERVERSIDE SCRIPT PATH = %s\n", scriptpath);
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
#ifdef GAME_DLL
	TransmitMessage(SIGNAL_TRANSITION);
#endif
	m_iMusicType = MUSICTYPE_HEAVY;
	Msg("music type %i\n", m_iMusicType);
}
void CFMODDynamicPlayer::InputImmediateMediumMusic(inputdata_t& inputdata)
{
#ifdef GAME_DLL
	TransmitMessage(SIGNAL_TRANSITION);
#endif
	m_iMusicType = MUSICTYPE_MEDIUM;
	Msg("music type %i\n", m_iMusicType);
}

void CFMODDynamicPlayer::RandomMusicPath(inputdata_t& inputdata)
{
#ifdef GAME_DLL
	TransmitMessage(SIGNAL_RANDOMPATH);
#endif
}
#ifdef CLIENT_DLL
void CFMODDynamicPlayer::ClientThink()
{
	BaseClass::ClientThink();
}
#endif
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

const char* CFMODDynamicPlayer::GetMusicTypeForScriptParse()
{
	switch (m_iMusicType)
	{
	case MUSICTYPE_LIGHT:
		return "LightMusic";
		break;
	case MUSICTYPE_MEDIUM:
		return "MediumMusic";
		break;
	case MUSICTYPE_HEAVY:
		return "HeavyMusic";
		break;

	default:
		return "0";
		break;
	}
}

void CFMODDynamicPlayer::ReceiveMessage(int classID, bf_read& msg)
{
	switch (msg.ReadByte())
	{
	case SIGNAL_PARSE:
		ParseMusicScript("MediumMusic");
		break;
	case SIGNAL_PRINT:
		PrintScriptToConsole();
		break;
	case SIGNAL_TRANSITION:
		ParseMusicScript("HeavyMusic");
		break;
	case SIGNAL_RANDOMPATH:
		PrintRandomPath();
		break;
	default:
		break;
	}
}
void CFMODDynamicPlayer::PostDataUpdate(DataUpdateType_t updateType)
{
	BaseClass::PostDataUpdate(updateType);
	 
	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS); 
		//ParseMusicScript("HeavyMusic");
		Warning("CLIENTSIDE SCRIPTPATH = %s\n", scriptpath.Get());
	}
}
void CFMODDynamicPlayer::ParseMusicScript(const char* pKeyName)
{
	if (!m_musiclist.IsEmpty())
		m_musiclist.RemoveMultipleFromTail(m_musiclist.Count());
	Warning("Parsing Music Script\n");
	KeyValues* pKV = new KeyValues("MusicScript");

	if (!pKV->LoadFromFile(filesystem, scriptpath, "MOD"))
	{
		Warning("FMOD FAILED TO LOAD SCRIPT FILE. ABORTING.\n");
		pKV->deleteThis();
		return;
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

void CFMODDynamicPlayer::PrintRandomPath()
{
	int rnd = RandomInt(0, m_musiclist.Count());
	if (rnd == lastrand)
	{
		rnd = RandomInt(0, m_musiclist.Count());
	}
	lastrand = rnd;
	if (m_musiclist.Element(lastrand).szMusicPath == NULL)
		Warning("NULL PATH\n");
	Msg("%s\n", m_musiclist.Element(rnd).szMusicPath);
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
		Msg("Music Type: %s\n", musicscript.szMusicType);
		Msg("Music Relative Path: %s\n", musicscript.szMusicPath);
		Msg("Music Absolute Path: %s\n", GetFixedPathToMusic(musicscript.szMusicPath));
	}

}
const char* CFMODDynamicPlayer::GetFixedPathToMusic(const char* path)
{

	char* fixedpath = new char[512];
	Q_snprintf(fixedpath, 512, "%s/sound/%s", engine->GetGameDirectory(), path);
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
#else
void CFMODDynamicPlayer::TransmitMessage(int message)
{
	EntityMessageBegin(this);
	WRITE_BYTE(message);
	MessageEnd();
}
#endif
void CFMODDynamicPlayer::PrintToConsole(inputdata_t& inputdata)
{
#ifdef GAME_DLL
	TransmitMessage(SIGNAL_PRINT);
#endif
}
void CFMODDynamicPlayer::PrintScriptToConsole()
{
#ifdef CLIENT_DLL
	for (int i = 0; i < m_musiclist.Count(), i++;)
	{
		
		Msg("path: %s\n", STRING(m_musiclist[i].szMusicPath));
	}
#endif
}
void CFMODDynamicPlayer::ParseScript(inputdata_t &inputdata)
{
#ifdef GAME_DLL
	TransmitMessage(SIGNAL_PARSE);
#endif
}
#ifdef GAME_DLL
int CFMODDynamicPlayer::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}
#endif

void CFMODDynamicPlayer::fuckit()
{
#ifdef CLIENT_DLL
	ParseMusicScript("HeavyMusic");
#endif
}

void CFMODDynamicPlayer::CheckMusicFile(void)
{
#ifdef CLIENT_DLL
	if (m_iMusicType != m_iPrevMusicType)
	{
		m_musiclist.RemoveAll();
		ParseMusicScript(GetMusicTypeForScriptParse());
	}
	Msg("%s\n", GetPathToMusic());
#endif
}