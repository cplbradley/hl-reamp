#include "cbase.h"
#include "hl2_player.h"
#include "player.h"
#include "triggers.h"

#include "tier0/memdbgon.h"



class CScreenFlipper : public CLogicalEntity
{
	
	DECLARE_CLASS(CScreenFlipper, CLogicalEntity);
public:
	void Spawn();
	void InputFlipScreen(inputdata_t& inputdata);
	void FlipScreen();
	bool IsScreenFlipped() { return m_bFlipped; }
private:
	bool m_bFlipped;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(hlr_screen_flipper, CScreenFlipper);

BEGIN_DATADESC(CScreenFlipper)
DEFINE_FIELD(m_bFlipped,FIELD_BOOLEAN),
DEFINE_INPUTFUNC(FIELD_VOID,"FlipScreen",InputFlipScreen),
END_DATADESC()


void CScreenFlipper::Spawn()
{
	BaseClass::Spawn();
}

void CScreenFlipper::InputFlipScreen(inputdata_t& inputdata)
{
	FlipScreen();
}
void CScreenFlipper::FlipScreen()
{
	CHL2_Player* pPlayer = dynamic_cast<CHL2_Player*>(UTIL_GetLocalPlayer());

	if (m_bFlipped)
	{
		pPlayer->m_HL2Local.m_bInvertedScreen = false;
	}
	else
	{
		pPlayer->m_HL2Local.m_bInvertedScreen = true;
	}
}