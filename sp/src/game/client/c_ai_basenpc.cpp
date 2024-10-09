//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ai_basenpc.h"
#include "engine/ivdebugoverlay.h"
#include "fx_quad.h"
#include "view.h"
#include "iviewrender.h"
#include "ivieweffects.h"
#include "view_scene.h"
#include "fx.h"
#include "model_types.h"
#include "bone_setup.h"
#include "viewrender.h"

#if defined( HL2_DLL ) || defined( HL2_EPISODIC )
#include "c_basehlplayer.h"
#endif

#include "death_pose.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PING_MAX_TIME	2.0

IMPLEMENT_CLIENTCLASS_DT( C_AI_BaseNPC, DT_AI_BaseNPC, CAI_BaseNPC )
	RecvPropInt( RECVINFO( m_lifeState ) ),
	RecvPropBool( RECVINFO( m_bPerformAvoidance ) ),
	RecvPropBool( RECVINFO( m_bIsMoving ) ),
	RecvPropBool( RECVINFO( m_bFadeCorpse ) ),
	RecvPropInt( RECVINFO ( m_iDeathPose) ),
	RecvPropInt( RECVINFO( m_iDeathFrame) ),
	RecvPropInt( RECVINFO( m_iSpeedModRadius ) ),
	RecvPropInt( RECVINFO( m_iSpeedModSpeed ) ),
	RecvPropInt( RECVINFO( m_bSpeedModActive ) ),
	RecvPropBool( RECVINFO( m_bImportanRagdoll ) ),
	RecvPropFloat( RECVINFO( m_flTimePingEffect ) ),
	RecvPropBool(RECVINFO(m_bIsGibbed)),
	RecvPropBool(RECVINFO(m_bShouldDrawShieldOverlay)),
	RecvPropInt(RECVINFO(m_iVortEffectType)),
END_RECV_TABLE()

extern ConVar cl_npc_speedmod_intime;

bool NPC_IsImportantNPC( C_BaseAnimating *pAnimating )
{
	C_AI_BaseNPC *pBaseNPC = dynamic_cast < C_AI_BaseNPC* > ( pAnimating );

	if ( pBaseNPC == NULL )
		return false;

	return pBaseNPC->ImportantRagdoll();
}

C_AI_BaseNPC::C_AI_BaseNPC()
{
}


void C_AI_BaseNPC::InterpScale()
{

	flscale += 0.05;
	flscale = MIN(flscale, 1.25);
}

ConVar testnvoverlay("testnvoverlay", "0");

int C_AI_BaseNPC::InternalDrawModel(int flags)
{
		
	if (m_bIsGibbed)
		return 0;

		C_BasePlayer* player = CBasePlayer::GetLocalPlayer();
		int ret = BaseClass::InternalDrawModel(flags);

		ConVarRef depthfarz("r_depthbuffer_farz");
		float fDist = (GetAbsOrigin() - player->GetAbsOrigin()).Length();
		bool bNightVision = ((player && player->IsEffectActive(EF_DIMLIGHT) && fDist < depthfarz.GetFloat()) || testnvoverlay.GetBool());

		if (ret != 0)
		{
			if (CurrentViewID() != VIEW_DEPTHBUFFER)
			{
				if (m_bShouldDrawShieldOverlay || bNightVision)
				{
					// Cyanide; So we basically need to redraw the model but scaled up slightly
					UpdateBoneAttachments();

					ClientModelRenderInfo_t info;
					ClientModelRenderInfo_t* pInfo;

					pInfo = &info;

					pInfo->flags = flags;
					pInfo->pRenderable = this;
					pInfo->instance = GetModelInstance();
					pInfo->entity_index = index;
					pInfo->pModel = GetModel();
					pInfo->origin = GetRenderOrigin();
					pInfo->angles = GetRenderAngles();
					pInfo->skin = GetSkin();
					pInfo->body = GetBody();
					pInfo->hitboxset = m_nHitboxSet;

					Assert(!pInfo->pModelToWorld);
					if (!pInfo->pModelToWorld)
					{
						pInfo->pModelToWorld = &pInfo->modelToWorld;

						// Turns the origin + angles into a matrix
						AngleMatrix(pInfo->angles, pInfo->origin, pInfo->modelToWorld);
					}
					;
					// Set override material for glow color
					IMaterial* pMatGlowColor = NULL;

					if (bNightVision && !pMatGlowColor)
						pMatGlowColor = materials->FindMaterial("engine/nightvision_enemyoverlay", TEXTURE_GROUP_OTHER);
					else
						pMatGlowColor = GetShieldType(m_iVortEffectType);

					if (pMatGlowColor)
					{
						pMatGlowColor->AddRef();
						modelrender->ForcedMaterialOverride(pMatGlowColor);
					}

					// Scale the base transform if we don't have a bone hierarchy
					CStudioHdr* pHdr = GetModelPtr();
					// Yes we do need to save how these were before
					matrix3x4_t pBones[MAXSTUDIOBONES]; // maybe have this be global?

					
					if (pHdr)
					{
						for (int i = 0; i < pHdr->numbones(); i++)
						{
							matrix3x4_t& transform = GetBoneForWrite(i);
							// Apply client-side effects to the transformation matrix
							InterpScale();
							float scale = bNightVision ? 1.1f : flscale;
							// Yes, we do need to copy this over to our array
							MatrixCopy(transform, pBones[i]);
							VectorScale(transform[0], scale, transform[0]);
							VectorScale(transform[1], scale, transform[1]);
							VectorScale(transform[2], scale, transform[2]);
						}
					}

					// Now draw the model
					DrawModelState_t state;
					matrix3x4_t* pBoneToWorld = NULL;
					modelrender->DrawModelSetup(*pInfo, &state, NULL, &pBoneToWorld);
					DoInternalDrawModel(pInfo, ((pInfo->flags & STUDIO_RENDER)) ? &state : NULL, pBoneToWorld);
					OnPostInternalDrawModel(pInfo);


					if (pMatGlowColor)
					{
						modelrender->ForcedMaterialOverride(0);
					}

					if (pHdr)
					{
						// Yes we need to apply them back after we've drawn the outline
						for (int i = 0; i < pHdr->numbones(); i++)
						{
							matrix3x4_t& transform = GetBoneForWrite(i);
							MatrixCopy(pBones[i], transform);
						}
					}

					return true;
				}
			}
		}

		return ret;
}

IMaterial* C_AI_BaseNPC::GetShieldType(int vorttype)
{
	switch (vorttype)
	{
	case 0:
		return materials->FindMaterial("engine/necro", TEXTURE_GROUP_OTHER, true);
		break;
	case 1:
		return materials->FindMaterial("engine/shield", TEXTURE_GROUP_OTHER, true);
		break;
	case 2:
		return materials->FindMaterial("engine/buff", TEXTURE_GROUP_OTHER, true);
		break;
	case 3:
		return materials->FindMaterial("engine/buff", TEXTURE_GROUP_OTHER, true);
		break;
	default:
		return materials->FindMaterial("engine/shield", TEXTURE_GROUP_OTHER, true);
		break;
	}
}
//-----------------------------------------------------------------------------
// Makes ragdolls ignore npcclip brushes
//-----------------------------------------------------------------------------
unsigned int C_AI_BaseNPC::PhysicsSolidMaskForEntity( void ) const 
{
	// This allows ragdolls to move through npcclip brushes
	if ( !IsRagdoll() )
	{
		return MASK_NPCSOLID; 
	}
	return MASK_SOLID;
}

const char* C_AI_BaseNPC::GetStartParticle()
{
	switch (m_iVortEffectType)
	{
	case 0:
		return "necro_form";
		break;
	case 1:
		return "shield_form";
		break;
	case 2:
		return "buff_form";
		break;
	case 3:
		return "buff_form";
		break;
	default:
		return "shield_form";
		break;
	}
}
const char* C_AI_BaseNPC::GetBurstParticle()
{
	switch (m_iVortEffectType)
	{
	case 0:
		return "necro_burst";
		break;
	case 1:
		return "shield_burst";
		break;
	case 2:
		return "buff_burst";
		break;
	case 3:
		return "buff_burst";
		break;
	default:
		return "shield_burst";
		break;
	}
}

void C_AI_BaseNPC::ClientThink( void )
{
	BaseClass::ClientThink();



	if (m_bShouldDrawShieldOverlay)
	{
		CParticleProperty* pProp = ParticleProp();

		if (!m_pShieldFX)
		{
			m_pShieldFX = pProp->Create(GetStartParticle(), PATTACH_ROOTBONE_FOLLOW);
		}
	}
	else
	{
		flscale = 1.0f;
		if (m_pShieldFX)
		{
			m_pShieldFX = NULL;
			CParticleProperty* pProp = ParticleProp();
			shieldtimer = 0;
			if (!m_pShieldBurstFX)
			{
				m_pShieldBurstFX = pProp->Create(GetBurstParticle(), PATTACH_ROOTBONE_FOLLOW);
			}
		}
	}
	
	shieldtimer++;
	if (shieldtimer >= 100)
		shieldtimer = 100;
	if (m_pShieldBurstFX && shieldtimer == 100)
	{
		m_pShieldBurstFX = NULL;
	}
#ifdef HL2_DLL
	C_BaseHLPlayer *pPlayer = dynamic_cast<C_BaseHLPlayer*>( C_BasePlayer::GetLocalPlayer() );

	if ( ShouldModifyPlayerSpeed() == true )
	{
		if ( pPlayer )
		{
			float flDist = (GetAbsOrigin() - pPlayer->GetAbsOrigin()).LengthSqr();

			if ( flDist <= GetSpeedModifyRadius() )
			{
				if ( pPlayer->m_hClosestNPC )
				{
					if ( pPlayer->m_hClosestNPC != this )
					{
						float flDistOther = (pPlayer->m_hClosestNPC->GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length();

						//If I'm closer than the other NPC then replace it with myself.
						if ( flDist < flDistOther )
						{
							pPlayer->m_hClosestNPC = this;
							pPlayer->m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_intime.GetFloat();
						}
					}
				}
				else
				{
					pPlayer->m_hClosestNPC = this;
					pPlayer->m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_intime.GetFloat();
				}
			}
		}
	}
#endif // HL2_DLL

#ifdef HL2_EPISODIC
	C_BaseHLPlayer *pPlayer = dynamic_cast<C_BaseHLPlayer*>( C_BasePlayer::GetLocalPlayer() );

	if ( pPlayer && m_flTimePingEffect > gpGlobals->curtime )
	{
		float fPingEffectTime = m_flTimePingEffect - gpGlobals->curtime;
		
		if ( fPingEffectTime > 0.0f )
		{
			Vector vRight, vUp;
			Vector vMins, vMaxs;

			float fFade;

			if( fPingEffectTime <= 1.0f )
			{
				fFade = 1.0f - (1.0f - fPingEffectTime);
			}
			else
			{
				fFade = 1.0f;
			}

			GetRenderBounds( vMins, vMaxs );
			AngleVectors (pPlayer->GetAbsAngles(), NULL, &vRight, &vUp );
			Vector p1 = GetAbsOrigin() + vRight * vMins.x + vUp * vMins.z;
			Vector p2 = GetAbsOrigin() + vRight * vMaxs.x + vUp * vMins.z;
			Vector p3 = GetAbsOrigin() + vUp * vMaxs.z;

			int r = 0 * fFade;
			int g = 255 * fFade;
			int b = 0 * fFade;

			debugoverlay->AddLineOverlay( p1, p2, r, g, b, true, 0.05f );
			debugoverlay->AddLineOverlay( p2, p3, r, g, b, true, 0.05f );
			debugoverlay->AddLineOverlay( p3, p1, r, g, b, true, 0.05f );
		}
	}
#endif
}

void C_AI_BaseNPC::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );
	SetNextClientThink(CLIENT_THINK_ALWAYS);
	if ( ( ShouldModifyPlayerSpeed() == true ) || ( m_flTimePingEffect > gpGlobals->curtime ) )
	{
		
	}
}


int C_AI_BaseNPC::DrawModel(int flags)
{
	if (m_bIsGibbed)
	{
		DevMsg("hiding ragdoll\n");
		return 0;
		
	}
	return BaseClass::DrawModel(flags);
}
void C_AI_BaseNPC::GetRagdollInitBoneArrays( matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt )
{
	ForceSetupBonesAtTime( pDeltaBones0, gpGlobals->curtime - boneDt );
	GetRagdollCurSequenceWithDeathPose( this, pDeltaBones1, gpGlobals->curtime, m_iDeathPose, m_iDeathFrame );
	float ragdollCreateTime = PhysGetSyncCreateTime();
	if ( ragdollCreateTime != gpGlobals->curtime )
	{
		// The next simulation frame begins before the end of this frame
		// so initialize the ragdoll at that time so that it will reach the current
		// position at curtime.  Otherwise the ragdoll will simulate forward from curtime
		// and pop into the future a bit at this point of transition
		ForceSetupBonesAtTime( pCurrentBones, ragdollCreateTime );
	}
	else
	{
		SetupBones( pCurrentBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime );
	}
}
ConVar ai_shadow_type("ai_shadow_type", "0");

ShadowType_t C_AI_BaseNPC::ShadowCastType()
{
	switch (ai_shadow_type.GetInt())
	{
		case 0:
		default:
			return SHADOWS_SIMPLE;
			break;
		case 1:
			return SHADOWS_RENDER_TO_TEXTURE;
			break;
		case 2:
			return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
			break;
		case 3:
			return SHADOWS_RENDER_TO_DEPTH_TEXTURE;
			break;
	}
}