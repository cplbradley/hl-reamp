#include "cbase.h"
#include "c_buttons.h"

IMPLEMENT_CLIENTCLASS_DT(C_BaseButton,DT_BaseButton,CBaseButton)
RecvPropBool(RECVINFO(nbLocked)),
RecvPropBool(RECVINFO(nbActive)),
END_RECV_TABLE()



C_BaseButton::C_BaseButton()
{
	nbLocked = false;
	nbActive = false;
}

