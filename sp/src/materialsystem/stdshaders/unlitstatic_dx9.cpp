#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"
#include "commandbuilder.h"


#include "include/unlitstatic_vs20.inc"
#include "include/unlitstatic_ps20b.inc"

const Sampler_t SAMPLER_BASETEXTURE = SHADER_SAMPLER0;


struct UnlitStatic_Vars_t
{
	UnlitStatic_Vars_t()
	{
		memset(this, 0xFF, sizeof(*this));
	}

	int basetexture;
	int baseTextureFrame;
	int baseTextureTransform;
};

BEGIN_VS_SHADER(UnlitStatic,"")
END_SHADER_PARAMS;

void SetupVars(IMaterialVar** params, UnlitStatic_Vars_t& info)
{
	info.basetexture = BASETEXTURE;
}

SHADER_INIT_PARAMS()
{};
SHADER_FALLBACK
{
	return 0;
};
SHADER_INIT
{
	UnlitStatic_Vars_t info;
	SetupVars(params, info);

	if (params[info.basetexture]->IsDefined())
	{
		LoadTexture(info.basetexture, TEXTUREFLAGS_SRGB);
	}
};

SHADER_DRAW
{
	UnlitStatic_Vars_t info;
	SetupVars(params, info);

	bool bHasBaseTexture = (info.basetexture != -1) && params[info.basetexture]->IsTexture();
	bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;

	BlendType_t nBlendType = EvaluateBlendRequirements(info.basetexture, true);
	bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;


	if (IsSnapshotting())
	{
		pShaderShadow->EnableAlphaTest(bIsAlphaTested);
		SetDefaultBlendingShadowState(info.basetexture, true);
		pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE, true);
		pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);
		pShaderShadow->EnableSRGBWrite(true);

		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
		// We only need one texcoord, in the default float2 size
		pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);

		DECLARE_STATIC_VERTEX_SHADER(unlitstatic_vs20);
		SET_STATIC_VERTEX_SHADER(unlitstatic_vs20);
		DECLARE_STATIC_PIXEL_SHADER(unlitstatic_ps20b);
		SET_STATIC_PIXEL_SHADER(unlitstatic_ps20b);
		pShaderShadow->EnableAlphaWrites(bFullyOpaque);
	}
	else
	{
		if (bHasBaseTexture)
			BindTexture(SAMPLER_BASETEXTURE, info.basetexture, info.baseTextureFrame);
		else
			pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
		float vEyePos_SpecExponent[4];
		pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);

		DECLARE_DYNAMIC_VERTEX_SHADER(unlitstatic_vs20);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0);
		SET_DYNAMIC_VERTEX_SHADER(unlitstatic_vs20);
		DECLARE_DYNAMIC_PIXEL_SHADER(unlitstatic_ps20b);
		SET_DYNAMIC_PIXEL_SHADER(unlitstatic_ps20b);

		//SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.baseTextureTransform);
		//pShaderAPI->SetToneMappingScaleLinear(Vector(0));
	}
	Draw();
}

END_SHADER;