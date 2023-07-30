//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"
#include "commandbuilder.h"
#include "mathlib\noise.h"

#include "include/nightvision_enemyoverlay_vs30.inc"
#include "include/nightvision_enemyoverlay_ps30.inc"


const Sampler_t SAMPLER_NOISETEXTURE = SHADER_SAMPLER0;

struct ShieldVars_t
{
	ShieldVars_t()
	{
		memset(this, 0xFF, sizeof(*this));
	}
	int noisetexture;

};

BEGIN_VS_SHADER( Nightvision_EnemyOverlay, "Help for UnlitGeneric" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM(NOISETEXTURE,SHADER_PARAM_TYPE_TEXTURE, "","")

	END_SHADER_PARAMS

	void SetupVars( ShieldVars_t& info )
	{
	info.noisetexture = NOISETEXTURE;
	}

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		ShieldVars_t info;
		SetupVars(info);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	}

	SHADER_DRAW
	{
		ShieldVars_t info;
		SetupVars(info);
		bool bHasNoise = (info.noisetexture != -1) && params[info.noisetexture]->IsTexture();

		SHADOW_STATE
		{
			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
			// We need three texcoords, all in the default float2 size
			pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
			pShaderShadow->EnableTexture(SAMPLER_NOISETEXTURE, true);
			pShaderShadow->EnableSRGBRead(SAMPLER_NOISETEXTURE, false);

			DECLARE_STATIC_PIXEL_SHADER(nightvision_enemyoverlay_ps30);
			SET_STATIC_PIXEL_SHADER(nightvision_enemyoverlay_ps30);
			DECLARE_STATIC_VERTEX_SHADER(nightvision_enemyoverlay_vs30);
			SET_STATIC_VERTEX_SHADER(nightvision_enemyoverlay_vs30);
			//pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_EQUAL, params[info.shieldalpha]->GetFloatValue());
			pShaderShadow->EnableAlphaWrites(true);
			EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_SRC_ALPHA);
		}
		DYNAMIC_STATE
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(nightvision_enemyoverlay_ps30);
			SET_DYNAMIC_PIXEL_SHADER(nightvision_enemyoverlay_ps30);
			DECLARE_DYNAMIC_VERTEX_SHADER(nightvision_enemyoverlay_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
			SET_DYNAMIC_VERTEX_SHADER(nightvision_enemyoverlay_vs30);


			if (bHasNoise)
				BindTexture(SAMPLER_NOISETEXTURE,info.noisetexture);


			
			float randfloat[2] = { 0,0 };
			Vector randVec = RandomVector(0, 1);
			randfloat[0] = randVec[0];
			randfloat[1] = randVec[1];

			pShaderAPI->SetPixelShaderConstant(16, randfloat, 1);


		}
		Draw();
	}
	END_SHADER;
