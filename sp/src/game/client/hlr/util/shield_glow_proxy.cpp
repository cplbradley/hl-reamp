#include "cbase.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterialvar.h"
#include "c_baseplayer.h"
#include "toolframework_client.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_ShieldGlowProxy : public IMaterialProxy
{
public:
	C_ShieldGlowProxy();
	virtual ~C_ShieldGlowProxy();

	virtual bool Init(IMaterial* pMaterial, KeyValues* pKeyValues);
	C_BaseEntity* BindArgToEntity(void* pArg);
	virtual void OnBind(void* pC_BaseEntity);
	virtual void Release() { delete this; }
	IMaterial* GetMaterial();

private:
	IMaterialVar* m_pBlendFactor;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_ShieldGlowProxy::C_ShieldGlowProxy()
{
	m_pBlendFactor = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_ShieldGlowProxy::~C_ShieldGlowProxy()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_ShieldGlowProxy::Init(IMaterial* pMaterial, KeyValues* pKeyValues)
{
	bool found;

	m_pBlendFactor = pMaterial->FindVar("$detailblendfactor", &found, false);
	if (!found)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_BaseEntity* C_ShieldGlowProxy::BindArgToEntity(void* pArg)
{
	IClientRenderable* pRend = (IClientRenderable*)pArg;
	return pRend ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ShieldGlowProxy::OnBind(void* pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity* pEntity = BindArgToEntity(pC_BaseEntity);
	C_AI_BaseNPC* pNPC = dynamic_cast<C_AI_BaseNPC*>(pEntity);

	if (pNPC)
	{
		m_pBlendFactor->SetFloatValue(pNPC->m_bShouldDrawShieldOverlay ? 1.0f : 0.0f);
	}

	if (ToolsEnabled())
		ToolFramework_RecordMaterialParams(GetMaterial());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IMaterial* C_ShieldGlowProxy::GetMaterial()
{
	return m_pBlendFactor->GetOwningMaterial();
}

EXPOSE_INTERFACE(C_ShieldGlowProxy, IMaterialProxy, "Shield	" IMATERIAL_PROXY_INTERFACE_VERSION);