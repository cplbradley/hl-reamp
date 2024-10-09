#include "cbase.h"
#include "usermessages.h"

#include "tier0/memdbgon.h"


class CEnvSpecialMessage : public CLogicalEntity
{
	DECLARE_CLASS(CEnvSpecialMessage, CLogicalEntity);
	DECLARE_DATADESC();

public:
	void SendData();

	void InputDisplayMessage(inputdata_t& data);

private:
	string_t szString;
	float m_flVertPos;
};

LINK_ENTITY_TO_CLASS(env_special_message, CEnvSpecialMessage);

BEGIN_DATADESC(CEnvSpecialMessage)
DEFINE_KEYFIELD(szString,FIELD_STRING,"Message"),
DEFINE_KEYFIELD(m_flVertPos,FIELD_FLOAT,"VertPos"),
DEFINE_INPUTFUNC(FIELD_VOID,"DisplayMessage",InputDisplayMessage),
END_DATADESC()

void CEnvSpecialMessage::InputDisplayMessage(inputdata_t& data)
{
	SendData();
}

void CEnvSpecialMessage::SendData()
{
	CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
	user.MakeReliable();
	UserMessageBegin(user, "SpecialMessage");
	WRITE_STRING(STRING(szString));
	WRITE_FLOAT(m_flVertPos);
	MessageEnd();
}