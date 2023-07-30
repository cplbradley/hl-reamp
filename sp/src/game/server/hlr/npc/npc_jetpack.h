#include "cbase.h"

class CNPCJetpack : public CBaseCombatCharacter
{
	DECLARE_CLASS(CNPCJetpack,CBaseCombatCharacter)
public:
	void Spawn(void);
	void Precache(void);
	void Launch(void);
	void Explode(void);
	
private:
	CAI_BaseNPC *pNPCParent;

};
