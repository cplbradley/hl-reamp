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
#include "include/pbr_vs30.inc"
#include "include/pbr_ps30.inc"
#include "view_shared.h"



// Defining samplers
const Sampler_t SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
const Sampler_t SAMPLER_NORMAL = SHADER_SAMPLER1;
const Sampler_t SAMPLER_ENVMAP = SHADER_SAMPLER2;
const Sampler_t SAMPLER_SHADOWDEPTH = SHADER_SAMPLER4;
const Sampler_t SAMPLER_RANDOMROTATION = SHADER_SAMPLER5;
const Sampler_t SAMPLER_FLASHLIGHT = SHADER_SAMPLER6;
const Sampler_t SAMPLER_LIGHTMAP = SHADER_SAMPLER7;
const Sampler_t SAMPLER_MRAO = SHADER_SAMPLER10;
const Sampler_t SAMPLER_EMISSIVE = SHADER_SAMPLER11;
const Sampler_t SAMPLER_DETAIL = SHADER_SAMPLER12;
const Sampler_t SAMPLER_DETAILMASK = SHADER_SAMPLER13;

// Convars
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar mat_specular("mat_specular", "1", FCVAR_CHEAT);
static ConVar mat_pbr_force_20b("mat_pbr_force_20b", "0", FCVAR_CHEAT);
static ConVar mat_pbr_parallaxmap("mat_pbr_parallaxmap", "1");
static ConVar mat_pbr_force_enabled("mat_pbr_force_enabled", "0");

// Variables for this shader
struct PBR_Vars_t
{
    PBR_Vars_t()
    {
        memset(this, 0xFF, sizeof(*this));
    }

    int baseTexture;
    int baseColor;
    int normalTexture;
    int bumpMap;
    int envMap;
    int baseTextureFrame;
    int baseTextureTransform;
    int useParallax;
    int parallaxDepth;
    int parallaxCenter;
    int parallaxSteps;
    int parallaxQuality;
    int alphaTestReference;
    int flashlightTexture;
    int flashlightTextureFrame;
    int emissionTexture;
    int mraoTexture;
    int lightmapTexture;

    int m_nDetail;
    int m_nDetailMask;
    int m_nDetailFrame;
    int m_nDetailScale;
    int m_bMultiDetail;
    int m_bDetailInMRAO;

    int decalScale;

    int flatMRAO;
    int m_nDetailTextureBlendFactor;
};

// Beginning the shader
BEGIN_VS_SHADER(PBR, "PBR shader")

    // Setting up vmt parameters
    BEGIN_SHADER_PARAMS;
        SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0", "");
        SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Set the cubemap for this material.");
        SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.");
        SHADER_PARAM(EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emission texture");
        SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture (deprecated, use $bumpmap)");
        SHADER_PARAM(NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture");
        SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Bumpmap");
        SHADER_PARAM(PARALLAX, SHADER_PARAM_TYPE_BOOL, "0", "Use Parallax Occlusion Mapping.");
        SHADER_PARAM(PARALLAXDEPTH, SHADER_PARAM_TYPE_FLOAT, "0.0030", "Depth of the Parallax Map");
        SHADER_PARAM(PARALLAXCENTER, SHADER_PARAM_TYPE_FLOAT, "0.5", "Center depth of the Parallax Map");
        SHADER_PARAM(PARALLAXSTEPS,SHADER_PARAM_TYPE_INTEGER,"20","How many steps i should use when parallaxing")
        SHADER_PARAM(PARALLAXQUALITY, SHADER_PARAM_TYPE_FLOAT, "1", "Quality of submapping")
        SHADER_PARAM(LIGHTMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "lightmap texture--will be bound by the engine")
        SHADER_PARAM(MULTIDETAIL,SHADER_PARAM_TYPE_BOOL,"0","Use multiple details")
        SHADER_PARAM(DETAILMASK,SHADER_PARAM_TYPE_TEXTURE,"","detail mask with mask for detail1 in red, mask for detail2 in green, mask for detail3 in blue")
        SHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture")
        SHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail")
        SHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture")
        SHADER_PARAM(DETAILINMRAO,SHADER_PARAM_TYPE_BOOL,"0","put the detail into the mrao")
        SHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.")
        SHADER_PARAM(FLATMRAO,SHADER_PARAM_TYPE_TEXTURE,"","flat mrao for fullbright")
        
    END_SHADER_PARAMS;

    // Setting up variables for this shader
    void SetupVars(IMaterialVar **params, PBR_Vars_t &info)
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
        info.parallaxSteps = PARALLAXSTEPS;
        info.parallaxQuality = PARALLAXQUALITY;
        info.lightmapTexture = LIGHTMAP;
        info.flatMRAO = FLATMRAO;
        info.m_nDetailMask = DETAILMASK;
        info.m_nDetail = DETAIL;
        info.m_nDetailFrame = DETAILFRAME;
        info.m_nDetailScale = DETAILSCALE;
        info.m_bMultiDetail = MULTIDETAIL;
        info.m_bDetailInMRAO = DETAILINMRAO;

        info.m_nDetailTextureBlendFactor = DETAILBLENDFACTOR;
    };
    // Initializing parameters
    SHADER_INIT_PARAMS()
    {
        // Fallback for changed parameter
        if (params[NORMALTEXTURE]->IsDefined())
            params[NORMALMAP]->SetStringValue(params[NORMALTEXTURE]->GetStringValue());
        
        if (params[BUMPMAP]->IsDefined())
            params[NORMALMAP]->SetStringValue(params[BUMPMAP]->GetStringValue());

        // Dynamic lights need a bumpmap
        if (!params[NORMALMAP]->IsDefined())
            params[NORMALMAP]->SetStringValue("dev/flat_normal");

        if(!params[FLATMRAO]->IsDefined())
            params[FLATMRAO]->SetStringValue("tools/mrao_flat");

        // Set a good default mrao texture
        if (!params[MRAOTEXTURE]->IsDefined())
            params[MRAOTEXTURE]->SetStringValue("tools/mraoempty");

        // PBR relies heavily on envmaps
        if (!params[ENVMAP]->IsDefined())
            params[ENVMAP]->SetStringValue("env_cubemap");

        // Check if the hardware supports flashlight border color
        if (g_pHardwareConfig->SupportsBorderColor())
        {
            params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
        }
        else
        {
            params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
        }
		
        size_t iLength = Q_strlen(pMaterialName);
        if (iLength > 6)
        {
            const char* path = "models";
            if (memicmp(path, pMaterialName, 6) == 0)
            {
                if (!IS_FLAG_SET(MATERIAL_VAR_MODEL))
                    SET_FLAGS(MATERIAL_VAR_MODEL);
            }
        }
		
		if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
		{
			SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
		}

    };

    // Define shader fallback
    SHADER_FALLBACK
    {
        ConVarRef sm30error("g_hide_sm30error");
        ConVarRef dxerror("g_hide_dxerror");
        if (mat_pbr_force_enabled.GetBool() == true)
         return 0;
        
        if (g_pHardwareConfig->GetDXSupportLevel() < 95)
        {
            
            dxerror.SetValue(0);
            if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
                return "VertexLitGeneric";
            else
                return "LightmappedGeneric";
        }
        if (!g_pHardwareConfig->SupportsShaderModel_3_0() || mat_pbr_force_20b.GetBool() == true)
        {
            
            sm30error.SetValue(0);
            if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
                return "VertexLitGeneric";
            else
                return "LightmappedGeneric";
        }
        sm30error.SetValue(1);
        dxerror.SetValue(1);
        return 0;
    };

    SHADER_INIT
    {
        PBR_Vars_t info;
        SetupVars(params, info);

        Assert(info.flashlightTexture >= 0);
        LoadTexture(info.flashlightTexture, TEXTUREFLAGS_SRGB);

        Assert(info.bumpMap >= 0);
        LoadBumpMap(info.bumpMap);

        Assert(info.envMap >= 0);
        int envMapFlags = g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0;
        envMapFlags |= TEXTUREFLAGS_ALL_MIPS;
        LoadCubeMap(info.envMap, envMapFlags);

        if (info.emissionTexture >= 0 && params[EMISSIONTEXTURE]->IsDefined())
            LoadTexture(info.emissionTexture, TEXTUREFLAGS_SRGB);

        Assert(info.mraoTexture >= 0);
        LoadTexture(info.mraoTexture, 0);

        LoadTexture(info.flatMRAO);


        if (params[info.baseTexture]->IsDefined())
        {
            LoadTexture(info.baseTexture, TEXTUREFLAGS_SRGB);
        }
        if (info.lightmapTexture != -1 && params[info.lightmapTexture]->IsDefined())
        {
            LoadTexture(info.lightmapTexture);
        }

        InitIntParam(info.m_bMultiDetail, params, 0);
        InitIntParam(info.m_bDetailInMRAO, params, 0);
        InitFloatParam(info.m_nDetailTextureBlendFactor, params, 1.0);
        InitFloatParam(info.m_nDetailScale, params, 4.0f);
        InitFloatParam(info.m_nDetailFrame, params, 0);
        InitFloatParam(info.decalScale, params, 0.25f);
        InitIntParam(info.parallaxSteps, params, 20);
        InitIntParam(info.parallaxQuality, params, 1);

        if (info.m_nDetail != -1 && params[info.m_nDetail]->IsDefined())
        {
            LoadTexture(info.m_nDetail);
        }

        if (info.m_nDetailMask != -1 && params[info.m_nDetailMask]->IsDefined())
        {
            LoadTexture(info.m_nDetailMask);
            Msg("DetailMask is defined\n");
        }

        if (IS_FLAG_SET(MATERIAL_VAR_MODEL)) // Set material var2 flags specific to models
        {
            SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
            SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
            SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
            SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
            SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
            SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
            SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
        }
        else // Set material var2 flags specific to brushes
        {
            SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);                // Required for lightmaps
            SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);         // Required for lightmaps
            SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
            SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
        }
    };

    // Drawing the shader
    SHADER_DRAW
    {
        PBR_Vars_t info;
        SetupVars(params, info);

        // Setting up booleans
        bool bHasBaseTexture = (info.baseTexture != -1) && params[info.baseTexture]->IsTexture();
        bool bHasNormalTexture = (info.bumpMap != -1) && params[info.bumpMap]->IsTexture();
        bool bHasMraoTexture = (info.mraoTexture != -1) && params[info.mraoTexture]->IsTexture();
        bool bHasEmissionTexture = (info.emissionTexture != -1) && params[info.emissionTexture]->IsTexture();
        bool bHasEnvTexture = (info.envMap != -1) && params[info.envMap]->IsTexture();
        bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
        bool bHasFlashlight = UsingFlashlight(params);
        bool bHasColor = (info.baseColor != -1) && params[info.baseColor]->IsDefined();
        bool bLightMapped = !IS_FLAG_SET(MATERIAL_VAR_MODEL);
        bool bLightMappedModel = info.lightmapTexture != -1 && params[info.lightmapTexture]->IsDefined();

        // Determining whether we're dealing with a fully opaque material
        BlendType_t nBlendType = EvaluateBlendRequirements(info.baseTexture, true);
        bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;


        bool bDecal = IS_FLAG_SET(MATERIAL_VAR_DECAL);



        int iParallaxQuality;
        if (params[info.parallaxQuality]->GetIntValue() <= 0)
            iParallaxQuality = 1;
        else
          iParallaxQuality = MIN(params[info.parallaxQuality]->GetIntValue(), 5) * 32;
		


        //float fBlendFactor = GetFloatParam(info.m_nDetailTextureBlendFactor, params, 1.0);
        bool bHasDetailTexture = IsTextureSet(info.m_nDetail, params);
        bool bHasDetailMask = IsTextureSet(info.m_nDetailMask, params);
        bool bMultiDetail = IsBoolSet(info.m_bMultiDetail, params);
        bool bDetailInMrao = IsBoolSet(info.m_bDetailInMRAO, params);

        //int nDetailBlendMode = bHasDetailTexture ? GetIntParam(info.m_nDetailTextureCombineMode, params) : 0;
        //int nDetailTranslucencyTexture = -1;

        /*if (bHasDetailTexture)
        {
            if ((nDetailBlendMode == 3) || (nDetailBlendMode == 8) || (nDetailBlendMode == 9))
                nDetailTranslucencyTexture = info.m_nDetail;
        }*/


        if (IsSnapshotting())
        {
            // If alphatest is on, enable it
            pShaderShadow->EnableAlphaTest(bIsAlphaTested);

            if (info.alphaTestReference != -1 && params[info.alphaTestReference]->GetFloatValue() > 0.0f)
            {
                pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.alphaTestReference]->GetFloatValue());
            }

            if (bHasFlashlight )
            {
                pShaderShadow->EnableBlending(true);
                pShaderShadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE); // Additive blending
            }
            else
            {
                SetDefaultBlendingShadowState(info.baseTexture, true);
            }

            int nShadowFilterMode = bHasFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;

            // Setting up samplers
            pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE, true);    // Basecolor texture
            pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);   // Basecolor is sRGB
            pShaderShadow->EnableTexture(SAMPLER_EMISSIVE, true);       // Emission texture
            pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE, true);      // Emission is sRGB
            pShaderShadow->EnableTexture(SAMPLER_LIGHTMAP, true);       // Lightmap texture
            pShaderShadow->EnableSRGBRead(SAMPLER_LIGHTMAP, false);     // Lightmaps aren't sRGB
            pShaderShadow->EnableTexture(SAMPLER_MRAO, true);           // MRAO texture
            pShaderShadow->EnableSRGBRead(SAMPLER_MRAO, false);         // MRAO isn't sRGB
            pShaderShadow->EnableTexture(SAMPLER_NORMAL, true);         // Normal texture
            pShaderShadow->EnableSRGBRead(SAMPLER_NORMAL, false);       // Normals aren't sRGB


            if (bHasDetailTexture)
            {
                pShaderShadow->EnableTexture(SAMPLER_DETAIL, true);
            }
            if (bHasDetailMask)
                pShaderShadow->EnableTexture(SAMPLER_DETAILMASK, true);


            // If the flashlight is on, set up its textures
            if (bHasFlashlight)
            {
                pShaderShadow->EnableTexture(SAMPLER_SHADOWDEPTH, true);        // Shadow depth map
                pShaderShadow->SetShadowDepthFiltering(SAMPLER_SHADOWDEPTH);
                pShaderShadow->EnableSRGBRead(SAMPLER_SHADOWDEPTH, false);
                pShaderShadow->EnableTexture(SAMPLER_RANDOMROTATION, true);     // Noise map
                pShaderShadow->EnableTexture(SAMPLER_FLASHLIGHT, true);         // Flashlight cookie
                pShaderShadow->EnableSRGBRead(SAMPLER_FLASHLIGHT, true);
            }

            // Setting up envmap
            if (bHasEnvTexture)
            {
                pShaderShadow->EnableTexture(SAMPLER_ENVMAP, true); // Envmap
                if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
                {
                    pShaderShadow->EnableSRGBRead(SAMPLER_ENVMAP, true); // Envmap is only sRGB with HDR disabled?
                }
            }

            // Enabling sRGB writing
            // See common_ps_fxc.h line 349
            // PS2b shaders and up write sRGB
            pShaderShadow->EnableSRGBWrite(true);

            if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
            {
                // We only need the position and surface normal
                unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
                // We need three texcoords, all in the default float2 size
                pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
            }
            else
            {
                // We need the position, surface normal, and vertex compression format
                unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
                // We only need one texcoord, in the default float2 size
                pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);
            }

            

            int useParallax = params[info.useParallax]->GetIntValue();
            if (!mat_pbr_parallaxmap.GetBool())
            {
                useParallax = 0;
            }
                // Setting up static vertex shader
                DECLARE_STATIC_VERTEX_SHADER(pbr_vs30);
                SET_STATIC_VERTEX_SHADER_COMBO(ISDECAL, bDecal);
                SET_STATIC_VERTEX_SHADER(pbr_vs30);

                // Setting up static pixel shader
                DECLARE_STATIC_PIXEL_SHADER(pbr_ps30);
                SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
                SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
                SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPPED, bLightMapped);
                SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVE, bHasEmissionTexture);
                SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXOCCLUSION, useParallax);
                SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
                SET_STATIC_PIXEL_SHADER(pbr_ps30);

            // Setting up fog
            DefaultFog(); // I think this is correct

            // HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
            pShaderShadow->EnableAlphaWrites(bFullyOpaque);
        }
        else // Not snapshotting -- begin dynamic state
        {
            bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
            bool bFullbright = mat_fullbright.GetInt() == 1;
#if 0
            int iMraoDebug = mat_pbr_show_mrao.GetInt();
            if (iMraoDebug > 4)
                iMraoDebug = 4;
#endif			
			if (bHasBaseTexture)
            {
                BindTexture(SAMPLER_BASETEXTURE, info.baseTexture, info.baseTextureFrame);
            }
            else
            {
                pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
            }
            if (bHasDetailTexture)
                BindTexture(SAMPLER_DETAIL, info.m_nDetail, info.m_nDetailFrame);

            if (bHasDetailMask)
                BindTexture(SAMPLER_DETAILMASK, info.m_nDetailMask, info.m_nDetailFrame);

            Vector color;
            if (bHasColor)
            {
                params[info.baseColor]->GetVecValue(color.Base(), 3);
            }
            else
            {
                color = Vector{ 1.f, 1.f, 1.f };
            }
            pShaderAPI->SetPixelShaderConstant(PSREG_SELFILLUMTINT, color.Base());
            // Setting up basecolor tint

            // Setting up environment map
            if (bHasEnvTexture)
            {
                BindTexture(SAMPLER_ENVMAP, info.envMap, 0);
            }
            else
            {
                pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
            }
            // Setting up emissive texture
            if (bHasEmissionTexture)
            {
                BindTexture(SAMPLER_EMISSIVE, info.emissionTexture, 0);
            }
            else
            {
                pShaderAPI->BindStandardTexture(SAMPLER_EMISSIVE, TEXTURE_BLACK);
            }

            // Setting up normal map
            if (bHasNormalTexture)
            {
                BindTexture(SAMPLER_NORMAL, info.bumpMap, 0);
            }
            else
            {
                pShaderAPI->BindStandardTexture(SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT);
            }

            // Setting up mrao map
            if (bHasMraoTexture)
            {
                BindTexture(SAMPLER_MRAO, info.mraoTexture, 0);
            }
            else
            {
                pShaderAPI->BindStandardTexture(SAMPLER_MRAO, TEXTURE_WHITE);
            }

            // Getting the light state
            LightState_t lightState;
            pShaderAPI->GetDX9LightState(&lightState);

            // Brushes don't need ambient cubes or dynamic lights
            if (!IS_FLAG_SET(MATERIAL_VAR_MODEL))
            {
                lightState.m_bAmbientLight = false;
                lightState.m_nNumLights = 0;
            }

            // Setting up the flashlight related textures and variables
            bool bFlashlightShadows = false;
            if (bHasFlashlight)
            {
                Assert(info.flashlightTexture >= 0 && info.flashlightTextureFrame >= 0);
                Assert(params[info.flashlightTexture]->IsTexture());
                BindTexture(SAMPLER_FLASHLIGHT, info.flashlightTexture, info.flashlightTextureFrame);
                VMatrix worldToTexture;
                ITexture *pFlashlightDepthTexture;
                FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
                bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

                SetFlashLightColorFromState(state, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

                if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows)
                {
                    BindTexture(SAMPLER_SHADOWDEPTH, pFlashlightDepthTexture, 0);
                    pShaderAPI->BindStandardTexture(SAMPLER_RANDOMROTATION, TEXTURE_SHADOW_NOISE_2D);
                }
            }

            // Getting fog info
            MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
            int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;

            // Getting skinning info
            int numBones = pShaderAPI->GetCurrentNumBones();

            // Some debugging stuff
            bool bWriteDepthToAlpha = false;
            bool bWriteWaterFogToAlpha = false;
            if (bFullyOpaque)
            {
                bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
                bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
                AssertMsg(!(bWriteDepthToAlpha && bWriteWaterFogToAlpha),
                        "Can't write two values to alpha at the same time.");
            }

            float vEyePos_SpecExponent[4];
            pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);

            // Determining the max level of detail for the envmap
            int iEnvMapLOD = 6;
            auto envTexture = params[info.envMap]->GetTextureValue();
            if (envTexture)
            {
                // Get power of 2 of texture width
                int width = envTexture->GetMappingWidth();
                int mips = 0;
                while (width >>= 1)
                    ++mips;

                // Cubemap has 4 sides so 2 mips less
                iEnvMapLOD = mips;
            }

            // Dealing with very high and low resolution cubemaps
            if (iEnvMapLOD > 12)
                iEnvMapLOD = 12;
            if (iEnvMapLOD < 2)
                iEnvMapLOD = 2;
			
            // This has some spare space
            vEyePos_SpecExponent[3] = iEnvMapLOD;
            pShaderAPI->SetPixelShaderConstant(PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1);

            if (bLightMapped)
                s_pShaderAPI->BindStandardTexture(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP_BUMPED);
            else if (bLightMappedModel)
                BindTexture(SAMPLER_LIGHTMAP, info.lightmapTexture);

                // Setting up dynamic vertex shader
                DECLARE_DYNAMIC_VERTEX_SHADER(pbr_vs30);
                SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
                SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
                SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
                SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
                SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
                SET_DYNAMIC_VERTEX_SHADER(pbr_vs30);

                // Setting up dynamic pixel shader
                DECLARE_DYNAMIC_PIXEL_SHADER(pbr_ps30);
                SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
                SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
                SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
                SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
                SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
                SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bLightMappedModel);
               // SET_DYNAMIC_PIXEL_SHADER_COMBO(MRAO_DEBUG, iMraoDebug);
                SET_DYNAMIC_PIXEL_SHADER(pbr_ps30);

            // Setting up base texture transform
            SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.baseTextureTransform);
            SetVertexShaderTextureScaledTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, info.baseTextureTransform, info.m_nDetailScale);


           

            // This is probably important
            SetModulationPixelShaderDynamicState_LinearColorSpace(1);

            // Send ambient cube to the pixel shader, force to black if not available
            pShaderAPI->SetPixelShaderStateAmbientLightCube(PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight);

            // Send lighting array to the pixel shader
            pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);

            // Handle mat_fullbright 2 (diffuse lighting only)
            if (bLightingOnly)
            {
                pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY); // Basecolor
            }

            if (bFullbright)
            {
                pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
                pShaderAPI->BindStandardTexture(SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT);
                BindTexture(SAMPLER_MRAO, info.flatMRAO);
            }

            // Handle mat_specular 0 (no envmap reflections)
            if (!mat_specular.GetBool())
            {
                pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK); // Envmap
            }


            // Sending fog info to the pixel shader
            pShaderAPI->SetPixelShaderFogParams(PSREG_FOG_PARAMS);

            float modulationColor[4] = { 1.0, 1.0, 1.0, 1.0 };
            ComputeModulationColor(modulationColor);
            float flLScale = pShaderAPI->GetLightMapScaleFactor();
            modulationColor[0] *= flLScale;
            modulationColor[1] *= flLScale;
            modulationColor[2] *= flLScale;
            pShaderAPI->SetPixelShaderConstant(PSREG_DIFFUSE_MODULATION, modulationColor);

            // More flashlight related stuff
            if (bHasFlashlight)
            {
                VMatrix worldToTexture;
                float atten[4], pos[4], tweaks[4];

                const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
                SetFlashLightColorFromState(flashlightState, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

                BindTexture(SAMPLER_FLASHLIGHT, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame);

                // Set the flashlight attenuation factors
                atten[0] = flashlightState.m_fConstantAtten;
                atten[1] = flashlightState.m_fLinearAtten;
                atten[2] = flashlightState.m_fQuadraticAtten;
                atten[3] = flashlightState.m_FarZ;
                pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_ATTENUATION, atten, 1);

                // Set the flashlight origin
                pos[0] = flashlightState.m_vecLightOrigin[0];
                pos[1] = flashlightState.m_vecLightOrigin[1];
                pos[2] = flashlightState.m_vecLightOrigin[2];
                pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1);

                pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4);

                // Tweaks associated with a given flashlight
                tweaks[0] = ShadowFilterFromState(flashlightState);
                tweaks[1] = ShadowAttenFromState(flashlightState);
                HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
                pShaderAPI->SetPixelShaderConstant(PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1);
            }

            float flParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            // Parallax Depth (the strength of the effect)
            flParams[0] = GetFloatParam(info.parallaxDepth, params, 3.0f);
            // Parallax Center (the height at which it's not moved)
            flParams[1] = GetFloatParam(info.parallaxCenter, params, 3.0f);
            pShaderAPI->SetPixelShaderConstant(27, flParams, 1);


            float flDetailParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            //blend factor
            flDetailParams[0] = GetFloatParam(info.m_nDetailTextureBlendFactor, params, 0.5f);
            //detail mask
            flDetailParams[1] = bHasDetailMask ? 1 : 0;
            //multidetail
            flDetailParams[2] = bMultiDetail ? 1 : 0;
            //detail in mrao *DEPRECATED*
            flDetailParams[3] = bDetailInMrao ? 1 : 0;
            pShaderAPI->SetPixelShaderConstant(26, flDetailParams, 1);

        }

        // Actually draw the shader
        Draw();
    };

// Closing it off
END_SHADER;
