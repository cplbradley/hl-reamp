//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A wet version of base * lightmap
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"

#include "include/alphacable_vs20.inc"
#include "include/alphacable_ps20b.inc"
#include "cpp_shader_constant_register_map.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);

DEFINE_FALLBACK_SHADER( AlphaCable, AlphaCable_DX9 )

BEGIN_VS_SHADER(AlphaCable_DX9,
			  "Help for Cable shader" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "cable/cablenormalmap", "bumpmap texture" )
		SHADER_PARAM( MINLIGHT, SHADER_PARAM_TYPE_FLOAT, "0.1", "Minimum amount of light (0-1 value)" )
		SHADER_PARAM( MAXLIGHT, SHADER_PARAM_TYPE_FLOAT, "0.3", "Maximum amount of light" )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if ( !(g_pHardwareConfig->SupportsPixelShaders_2_0() && g_pHardwareConfig->SupportsVertexShaders_2_0()) ||
				(g_pHardwareConfig->GetDXSupportLevel() < 90) )
		{
			return "Cable_DX8";
		}
		return 0;
	}

	SHADER_INIT
	{
		LoadBumpMap( BUMPMAP );
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
	}

	SHADER_DRAW
	{
		BlendType_t nBlendType = EvaluateBlendRequirements( BASETEXTURE, true );
		bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
		bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested; //dest alpha is free for special use

		SHADOW_STATE
		{
			// Enable blending?
			if ( IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ) )
			{
				pShaderShadow->EnableDepthWrites( false );
				pShaderShadow->EnableBlending( true );
				pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			}
			pShaderShadow->EnableAlphaTest(true);

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			if ( g_pHardwareConfig->GetDXSupportLevel() >= 90)
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, true );
			}
			
			int tCoordDimensions[] = {2,2};
			pShaderShadow->VertexShaderVertexFormat( 
				VERTEX_POSITION | VERTEX_COLOR | VERTEX_TANGENT_S | VERTEX_TANGENT_T, 
				2, tCoordDimensions, 0 );

			DECLARE_STATIC_VERTEX_SHADER( alphacable_vs20 );
			SET_STATIC_VERTEX_SHADER( alphacable_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( alphacable_ps20b );
			SET_STATIC_PIXEL_SHADER( alphacable_ps20b );

			// we are writing linear values from this shader.
			// This is kinda wrong.  We are writing linear or gamma depending on "IsHDREnabled" below.
			// The COLOR really decides if we are gamma or linear.  
			pShaderShadow->EnableSRGBWrite( true );

			FogToFogColor();

			pShaderShadow->EnableAlphaWrites( false );
		}
		DYNAMIC_STATE
		{
			bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET( MATERIAL_VAR_NO_DEBUG_OVERRIDE );

			BindTexture( SHADER_SAMPLER0, BUMPMAP );
			if ( bLightingOnly )
			{
				pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_GREY );

			}
			else
			{
				BindTexture( SHADER_SAMPLER1, BASETEXTURE );			
			}

			pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );		

			float vEyePos_SpecExponent[4];
			pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_SpecExponent );
			vEyePos_SpecExponent[3] = 0.0f;
			pShaderAPI->SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1 );

			DECLARE_DYNAMIC_VERTEX_SHADER( alphacable_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			SET_DYNAMIC_VERTEX_SHADER( alphacable_vs20 );

			DECLARE_DYNAMIC_PIXEL_SHADER( alphacable_ps20b );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bFullyOpaque && pShaderAPI->ShouldWriteDepthToDestAlpha() );
			SET_DYNAMIC_PIXEL_SHADER( alphacable_ps20b );
		}
		Draw();
	}
END_SHADER
