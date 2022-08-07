#include "cbase.h"
#include "fmod/fmod_system.h"
#include "fmod/fmod_dynamic_player.h"


/*using namespace FMOD;

System			*pSystem;
Sound			*pSound;
SoundGroup		*pSoundGroup;
Channel			*pChannel;
ChannelGroup	*pChannelGroup;
FMOD_RESULT	result;

CFMODSystem gFMODsys;
CFMODSystem* FMODSystem()
{
	return &gFMODsys;
}


CFMODSystem::CFMODSystem()
{
	unsigned int *audioPosition = 0;
	unsigned int *syncPoint = 0;
	pChannel->getPosition(audioPosition, FMOD_TIMEUNIT_MS);
	FMOD_SYNCPOINT *sync;
	pSound->getSyncPoint(1, &sync);
	pSound->getSyncPointInfo(sync, 0, 0, syncPoint, FMOD_TIMEUNIT_MS);

	if (audioPosition == syncPoint)
	{
		result = pSystem->createStream(FMODPlayer()->GetPathToMusic(), FMOD_DEFAULT, 0, &pSound);
		if (result != FMOD_OK)
			return;

		pSystem->playSound(FMOD_CHANNEL_FREE, pSound, false, &pChannel);
	}
}
void CFMODSystem::InitFMOD()
{
	result = System_Create(&pSystem);
	if (result != FMOD_OK)
		Warning("FMOD FAILED TO CREATE SYSTEM\n");
	result = pSystem->init(100, FMOD_INIT_NORMAL, 0);
	if (result != FMOD_OK)
		Warning("FMOD FAILED TO INITIALIZE\n");
}
void CFMODSystem::ExitFMOD()
{
	pSystem->release();
}*/