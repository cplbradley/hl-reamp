#include "cbase.h"
#include "player.h"
#include "mathlib/mathlib.h"
#include "ai_speech.h"
#include "stringregistry.h"
#include "gamerules.h"
#include "game.h"
#include <ctype.h>
#include "entitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "ndebugoverlay.h"
#include "soundscape.h"
#include "igamesystem.h"
#include "KeyValues.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

class CAmbientMusic : public CPointEntity
{
public:
	DECLARE_CLASS(CAmbientMusic, CPointEntity);

	void Spawn(void);
	void Precache(void);
	void UpdateStatus()

	
};