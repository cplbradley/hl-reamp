#include "cbase.h"
#include "baseanimating.h"

#include "tier0/memdbgon.h"




class CPropArmorController : public CPointEntity
{
	DECLARE_CLASS(CPropArmorController, CPointEntity);
	DECLARE_DATADESC();
public:
	void Spawn();
	void SetArmorGroups();
	void GetArmorProp();
	void InputSetHelmetGroup(inputdata_t& inputdata);
private:
	const char* szArmorPropName;
	CBaseEntity* ArmorProp;
	bool m_bHelmetOnly;
};

LINK_ENTITY_TO_CLASS(prop_armor_controller, CPropArmorController);

BEGIN_DATADESC(CPropArmorController)
DEFINE_KEYFIELD(szArmorPropName,FIELD_STRING,"ArmorPropName"),
DEFINE_KEYFIELD(m_bHelmetOnly,FIELD_BOOLEAN,"HelmetOnly"),
DEFINE_INPUTFUNC(FIELD_VOID,"SetHelmetGroup",InputSetHelmetGroup),
END_DATADESC()

void UpdateArmor(const CCommand& args)
{

	CBaseEntity* pEnt = gEntList.FindEntityByClassname(NULL, "prop_armor_controller");

	while(pEnt)
	{
		CPropArmorController* controller = dynamic_cast<CPropArmorController*>(pEnt);
		controller->SetArmorGroups();
		pEnt = gEntList.FindEntityByClassname(pEnt, "prop_armor_controller");
	}
}
ConCommand updatearmor("updatearmor", UpdateArmor);

void CPropArmorController::Spawn()
{
	BaseClass::Spawn();
	GetArmorProp();
}

void CPropArmorController::GetArmorProp()
{
	ArmorProp = gEntList.FindEntityByName(NULL, szArmorPropName);
	if (ArmorProp)
		SetArmorGroups();
}
void CPropArmorController::SetArmorGroups()
{
	if (ArmorProp && !m_bHelmetOnly)
	{
		ConVarRef helmet("cl_armor_helmet");
		ConVarRef chest("cl_armor_chest");
		ConVarRef codpiece("cl_armor_codpiece");
		ConVarRef upperarm("cl_armor_upperarm");
		ConVarRef forearm("cl_armor_forearm");
		ConVarRef thighs("cl_armor_thighs");
		ConVarRef boots("cl_armor_boots");

		CBaseAnimating* pArmor = dynamic_cast<CBaseAnimating*>(ArmorProp);

		int headgroup = pArmor->FindBodygroupByName("helmet");
		int chestgroup = pArmor->FindBodygroupByName("chest");
		int codpiecegroup = pArmor->FindBodygroupByName("codpiece");
		int bootsgroup = pArmor->FindBodygroupByName("boots");
		int thighsgroup = pArmor->FindBodygroupByName("thighs");
		int forearmgroup = pArmor->FindBodygroupByName("forearms");
		int upperarmgroup = pArmor->FindBodygroupByName("upperarms");

		pArmor->SetBodygroup(headgroup, helmet.GetInt());
		pArmor->SetBodygroup(chestgroup, chest.GetInt());
		pArmor->SetBodygroup(codpiecegroup, codpiece.GetInt());
		pArmor->SetBodygroup(bootsgroup, boots.GetInt());
		pArmor->SetBodygroup(thighsgroup, thighs.GetInt());
		pArmor->SetBodygroup(forearmgroup, forearm.GetInt());
		pArmor->SetBodygroup(upperarmgroup, upperarm.GetInt());
	}
}

void CPropArmorController::InputSetHelmetGroup(inputdata_t& inputdata)
{
	if (ArmorProp)
	{
		ConVarRef helmet("cl_armor_helmet");

		CBaseAnimating* pArmor = dynamic_cast<CBaseAnimating*>(ArmorProp);
		int headgroup = pArmor->FindBodygroupByName("helmet");

		pArmor->SetBodygroup(headgroup, helmet.GetInt());
	}
}