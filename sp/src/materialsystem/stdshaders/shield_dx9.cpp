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

#include "include/shield_vs30.inc"
#include "include/shield_ps30.inc"

extern ConVar r_flashlight_version2;

struct ShieldVars_t
{
	ShieldVars_t()
	{
		memset(this, 0xFF, sizeof(*this));
	}


	int shieldcolor;
	int shieldalpha;
	int shieldwidth;

};

BEGIN_VS_SHADER( Shield, "Help for UnlitGeneric" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM(SHIELDCOLOR,SHADER_PARAM_TYPE_COLOR, "[0 0 0]","")
		SHADER_PARAM(SHIELDALPHA, SHADER_PARAM_TYPE_FLOAT, "0.5","")
		SHADER_PARAM(SHIELDWIDTH,SHADER_PARAM_TYPE_FLOAT,"3","")

	END_SHADER_PARAMS

	void SetupVars( ShieldVars_t& info )
	{
	info.shieldcolor = SHIELDCOLOR;
	info.shieldalpha = SHIELDALPHA;
	info.shieldwidth = SHIELDWIDTH;
	}

	SHADER_INIT_PARAMS()
	{
		if (!params[SHIELDCOLOR]->IsDefined())
			params[SHIELDCOLOR]->SetVecValue(0.5, 0.5, 0.5);

		if (!params[SHIELDALPHA]->IsDefined())
			params[SHIELDALPHA]->SetFloatValue(0.5);
		if (!params[SHIELDWIDTH]->IsDefined())
			params[SHIELDWIDTH]->SetFloatValue(3.0);
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		ShieldVars_t info;
		SetupVars(info);
		//SET_FLAGS(MATERIAL_VAR_TRANSLUCENT);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	}

	SHADER_DRAW
	{
		ShieldVars_t info;
		SetupVars(info);
	

		SHADOW_STATE
		{
			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
			// We need three texcoords, all in the default float2 size
			pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
			DECLARE_STATIC_PIXEL_SHADER(shield_ps30);
			SET_STATIC_PIXEL_SHADER(shield_ps30);
			DECLARE_STATIC_VERTEX_SHADER(shield_vs30);
			SET_STATIC_VERTEX_SHADER(shield_vs30);
			//pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_EQUAL, params[info.shieldalpha]->GetFloatValue());
			pShaderShadow->EnableAlphaWrites(true);
			EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_SRC_ALPHA);
		}
		DYNAMIC_STATE
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(shield_ps30);
			SET_DYNAMIC_PIXEL_SHADER(shield_ps30);
			DECLARE_DYNAMIC_VERTEX_SHADER(shield_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
			SET_DYNAMIC_VERTEX_SHADER(shield_vs30);
			
			Vector color;
			float alpha[1] = { 0.0f };
			alpha[0] = GetFloatParam(info.shieldalpha, params, 0.5f);
			params[info.shieldcolor]->GetVecValue(color.Base(), 3);
			pShaderAPI->SetPixelShaderConstant(27, color.Base());
			pShaderAPI->SetPixelShaderConstant(26, alpha);
			float width[1] = { 0.0f };
			width[0] = GetFloatParam(info.shieldwidth, params, 3.0f);
			pShaderAPI->SetVertexShaderConstant(28, width);

		}
		Draw();
	}
	END_SHADER;
