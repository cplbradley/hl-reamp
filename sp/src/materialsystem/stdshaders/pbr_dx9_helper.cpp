//==================================================================================================
//
// Physically Based Rendering shader for brushes and models
//
//==================================================================================================

// Includes for all shaders
#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"
#include "commandbuilder.h"
// Includes for PS30
#include "view_shared.h"
#include "pbr_dx9_helper.h"

#include <functional>


#define PBR_Params()
	SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0", "") \ 
	SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Set the cubemap for this material.") \
	SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.") \
	SHADER_PARAM(EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emission texture") \
	SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture (deprecated, use $bumpmap)") \
	SHADER_PARAM(NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture") \
	SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Bumpmap") \
	SHADER_PARAM(PARALLAX, SHADER_PARAM_TYPE_BOOL, "0", "Use Parallax Occlusion Mapping.") \
	SHADER_PARAM(PARALLAXDEPTH, SHADER_PARAM_TYPE_FLOAT, "0.0030", "Depth of the Parallax Map") \
	SHADER_PARAM(PARALLAXCENTER, SHADER_PARAM_TYPE_FLOAT, "0.5", "Center depth of the Parallax Map") \
	SHADER_PARAM(LIGHTMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "lightmap texture--will be bound by the engine") \
	SHADER_PARAM(MULTIDETAIL, SHADER_PARAM_TYPE_BOOL, "0", "Use multiple details") \
	SHADER_PARAM(DETAILMASK, SHADER_PARAM_TYPE_TEXTURE, "", "detail mask with mask for detail1 in red, mask for detail2 in green, mask for detail3 in blue") \
	SHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture") \
	SHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail") \
	SHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture") \
	SHADER_PARAM(DETAILINMRAO, SHADER_PARAM_TYPE_BOOL, "0", "put the detail into the mrao") \
	SHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.") \
	SHADER_PARAM(FLATMRAO, SHADER_PARAM_TYPE_TEXTURE, "", "flat mrao for fullbright")

void InitPBR(CBaseVSShader* pShader, IMaterialVar** params, PBR_Vars_t& info)
{
	info.baseTexture = BASETEXTURE;
	info.baseColor = IS_FLAG_SET(MATERIAL_VAR_MODEL) ? COLOR2 : COLOR;
	info.normalTexture = NORMALTEXTURE;
	info.bumpMap = NORMALMAP;
	info.baseTextureFrame = FRAME;
	info.baseTextureTransform = BASETEXTURETRANSFORM;
	info.alphaTestReference = ALPHATESTREFERENCE;
	info.flashlightTexture = FLASHLIGHTTEXTURE;
	info.flashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
	info.envMap = ENVMAP;
	info.emissionTexture = EMISSIONTEXTURE;
	info.mraoTexture = MRAOTEXTURE;
	info.useParallax = PARALLAX;
	info.parallaxDepth = PARALLAXDEPTH;
	info.parallaxCenter = PARALLAXCENTER;
	info.lightmapTexture = LIGHTMAP;

	info.flatMRAO = FLATMRAO;
	info.m_nDetailMask = DETAILMASK;
	info.m_nDetail = DETAIL;
	info.m_nDetailFrame = DETAILFRAME;
	info.m_nDetailScale = DETAILSCALE;
	info.m_bMultiDetail = MULTIDETAIL;
	info.m_bDetailInMRAO = DETAILINMRAO;
	//info.m_nDetailTextureCombineMode = 0;
	info.m_nDetailTextureBlendFactor = DETAILBLENDFACTOR;
}



void StaticShaderPBRModelCombos(IShaderShadow* pShaderShadow, bool bHasEmissionTexture, bool bHasFlashlight, bool bHasDetailTexture, bool bUseParallax)
{
	DECLARE_STATIC_VERTEX_SHADER(pbr_model_vs30);
	SET_STATIC_VERTEX_SHADER(pbr_model_vs30);

	DECLARE_STATIC_PIXEL_SHADER(pbr_model_ps30);
	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, g_pHardwareConfig->GetShadowFilterMode());
	SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVE, bHasEmissionTexture);
	SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXOCCLUSION, bUseParallax);
	SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
	SET_STATIC_PIXEL_SHADER(pbr_model_ps30);
}

void DynamicShaderPBRModelCombos(IShaderDynamicAPI* pShaderAPI, bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, bool bFlashlightShadows, bool bLightMappedModel, int iNumLights)
{
	DECLARE_STATIC_VERTEX_SHADER(pbr_model_vs30);
	SET_STATIC_VERTEX_SHADER(pbr_model_vs30);
	DECLARE_DYNAMIC_PIXEL_SHADER(pbr_model_ps30);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, iNumLights);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
	SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bLightMappedModel);
	SET_DYNAMIC_PIXEL_SHADER(pbr_model_ps30)
}

