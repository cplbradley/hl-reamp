#include "cbase.h"
#include "soundent.h"
#include "npcevent.h"
#include "globalstate.h"
#include "ai_squad.h"
#include "ai_tacticalservices.h"
#include "npc_manhack.h"
#include "npc_metropolice.h"
#include "weapon_stunstick.h"
#include "basegrenade_shared.h"
#include "ai_route.h"
#include "hl2_player.h"
#include "iservervehicle.h"
#include "items.h"
#include "hl2_gamerules.h"


#include "tier0/memdbgon.h"


class CNPC_EliteCP : public CNPC_MetroPolice
{
	DECLARE_CLASS(CNPC_EliteCP, CNPC_MetroPolice)
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(npc_elitecp, CNPC_EliteCP);
BEGIN_DATADESC(CNPC_EliteCP)
END_DATADESC()
