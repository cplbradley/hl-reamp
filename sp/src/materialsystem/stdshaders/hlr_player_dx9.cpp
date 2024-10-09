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
#include "include/hlr_player_vs30.inc"
#include "include/hlr_player_ps30.inc"
#include "view_shared.h"

// Defining samplers
const Sampler_t SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
const Sampler_t SAMPLER_NORMAL = SHADER_SAMPLER1;
const Sampler_t SAMPLER_ENVMAP = SHADER_SAMPLER2;
const Sampler_t SAMPLER_MRAO = SHADER_SAMPLER3;
const Sampler_t SAMPLER_SHADOWDEPTH = SHADER_SAMPLER4;
const Sampler_t SAMPLER_RANDOMROTATION = SHADER_SAMPLER5;
const Sampler_t SAMPLER_FLASHLIGHT = SHADER_SAMPLER6;
const Sampler_t SAMPLER_COLORMASK = SHADER_SAMPLER7;
const Sampler_t SAMPLER_ENVMAP2 = SHADER_SAMPLER8;
const Sampler_t SAMPLER_LIGHTMAP = SHADER_SAMPLER9;

// Convars
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar mat_specular("mat_specular", "1", FCVAR_CHEAT);
static ConVar mat_pbr_force_20b("mat_pbr_force_20b", "0", FCVAR_CHEAT);
static ConVar mat_pbr_parallaxmap("mat_pbr_parallaxmap", "1");
static ConVar mat_pbr_force_enabled("mat_pbr_force_enabled", "0");
static ConVar mat_player_primary_r("mat_player_primary_r", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_primary_g("mat_player_primary_g", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_primary_b("mat_player_primary_b", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_secondary_r("mat_player_secondary_r", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_secondary_g("mat_player_secondary_g", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_secondary_b("mat_player_secondary_b", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_tertiary_r("mat_player_tertiary_r", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_tertiary_g("mat_player_tertiary_g", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_tertiary_b("mat_player_tertiary_b", "100", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
static ConVar mat_player_use_custom_colors("mat_player_use_custom_colors", "0", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);


// Variables for this shader
struct PBR_Vars_t
{
    PBR_Vars_t()
    {
        memset(this, 0xFF, sizeof(*this));
    }

    int baseTexture;
    int normalTexture;
    int bumpMap;
    int envMap;
    int useParallax;
    int alphaTestReference;
    int flashlightTexture;
    int flashlightTextureFrame;
    int mraoTexture;
    int lerpEnvMaps;
    int color1;
    int color2;
    int color3;
    int colormask;
    int monochrome;

};

// Beginning the shader
BEGIN_VS_SHADER(hlr_player, "PBR shader")

// Setting up vmt parameters
BEGIN_SHADER_PARAMS;
SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0", "");
SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Set the cubemap for this material.");
SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.");
SHADER_PARAM(EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emission texture");
SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture (deprecated, use $bumpmap)");
SHADER_PARAM(NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture");
SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Bumpmap");
SHADER_PARAM(FLATMRAO, SHADER_PARAM_TYPE_TEXTURE, "", "flat mrao for fullbright")
SHADER_PARAM(LERPENVMAP, SHADER_PARAM_TYPE_BOOL, "0", "lerp between cubemaps")
SHADER_PARAM(MONOCHROME, SHADER_PARAM_TYPE_TEXTURE, "", "monochrome texture")
SHADER_PARAM(COLORMASK, SHADER_PARAM_TYPE_TEXTURE, "", "color mask for player coloration");

END_SHADER_PARAMS;

// Setting up variables for this shader
void SetupVars(IMaterialVar** params, PBR_Vars_t& info)
{
    info.baseTexture = BASETEXTURE;
    info.normalTexture = NORMALTEXTURE;
    info.bumpMap = NORMALMAP;
    info.alphaTestReference = ALPHATESTREFERENCE;
    info.flashlightTexture = FLASHLIGHTTEXTURE;
    info.flashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
    info.envMap = ENVMAP;
    info.mraoTexture = MRAOTEXTURE;
    info.lerpEnvMaps = LERPENVMAP;
    info.monochrome = MONOCHROME;
    info.colormask = COLORMASK;
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

    if (!params[FLATMRAO]->IsDefined())
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
};

// Define shader fallback
SHADER_FALLBACK
{
    if (g_pHardwareConfig->GetDXSupportLevel() < 95)
    {
        return "VertexLitGeneric";
    }
    if (!g_pHardwareConfig->SupportsShaderModel_3_0())
    {
        return "VertexLitGeneric";
    }
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

    Assert(info.mraoTexture >= 0);
    LoadTexture(info.mraoTexture, 0);

    LoadTexture(info.monochrome);
    LoadTexture(info.colormask);

    if (params[info.baseTexture]->IsDefined())
    {
        LoadTexture(info.baseTexture, TEXTUREFLAGS_SRGB);
    }
        SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
        SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
        SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
        SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
        SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
        SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
        SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
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
    bool bHasEnvTexture = (info.envMap != -1) && params[info.envMap]->IsTexture();
    bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
    bool bHasFlashlight = UsingFlashlight(params);
    bool bUseCustomColors = mat_player_use_custom_colors.GetInt() == 1;
    bool bHasColorMask = (info.colormask != -1) && params[info.colormask]->IsTexture();
    bool bLerpEnvMap = (info.lerpEnvMaps != -1) && params[info.lerpEnvMaps]->GetIntValue() == 1;

    // Determining whether we're dealing with a fully opaque material
    BlendType_t nBlendType = EvaluateBlendRequirements(info.baseTexture, true);
    bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;

    if (IsSnapshotting())
    {
        // If alphatest is on, enable it
        pShaderShadow->EnableAlphaTest(bIsAlphaTested);

        if (info.alphaTestReference != -1 && params[info.alphaTestReference]->GetFloatValue() > 0.0f)
        {
            pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.alphaTestReference]->GetFloatValue());
        }

        if (bHasFlashlight)
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
        pShaderShadow->EnableTexture(SAMPLER_MRAO, true);           // MRAO texture
        pShaderShadow->EnableSRGBRead(SAMPLER_MRAO, false);         // MRAO isn't sRGB
        pShaderShadow->EnableTexture(SAMPLER_NORMAL, true);         // Normal texture
        pShaderShadow->EnableSRGBRead(SAMPLER_NORMAL, false);  // Normals aren't sRGB

        if (bUseCustomColors)
        {
            pShaderShadow->EnableTexture(SAMPLER_COLORMASK, true);
            pShaderShadow->EnableSRGBRead(SAMPLER_COLORMASK, true);
        }

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

            if (bLerpEnvMap)
            {
                pShaderShadow->EnableTexture(SAMPLER_ENVMAP2, true);
                if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
                    pShaderShadow->EnableSRGBRead(SAMPLER_ENVMAP2, true);

            }
        }

        // Enabling sRGB writing
        // See common_ps_fxc.h line 349
        // PS2b shaders and up write sRGB
        pShaderShadow->EnableSRGBWrite(true);

        // We only need the position and surface normal
        unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
        // We need three texcoords, all in the default float2 size
        pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);


        // Setting up static vertex shader
    DECLARE_STATIC_VERTEX_SHADER(hlr_player_vs30);
    SET_STATIC_VERTEX_SHADER(hlr_player_vs30);

    // Setting up static pixel shader
DECLARE_STATIC_PIXEL_SHADER(hlr_player_ps30);
SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
SET_STATIC_PIXEL_SHADER_COMBO(LERPENVMAP, bLerpEnvMap);
SET_STATIC_PIXEL_SHADER(hlr_player_ps30);

// Setting up fog
DefaultFog(); // I think this is correct

// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
pShaderShadow->EnableAlphaWrites(bFullyOpaque);
}
else // Not snapshotting -- begin dynamic state
{
    ClientRender_Globals_t* defGlobals = (ClientRender_Globals_t*)pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_GLOBALS_PTR);

    bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
    bool bFullbright = mat_fullbright.GetInt() == 1;

    if (bHasBaseTexture)
    {
        if (!bUseCustomColors)
            BindTexture(SAMPLER_BASETEXTURE, info.baseTexture);
        else
            BindTexture(SAMPLER_BASETEXTURE, info.monochrome);
    }
    else
    {
        pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
    }

    pShaderAPI->BindStandardTexture(SAMPLER_LIGHTMAP, TEXTURE_BLACK);


    if (bHasColorMask)
        BindTexture(SAMPLER_COLORMASK, info.colormask);
    else
        pShaderAPI->BindStandardTexture(SAMPLER_COLORMASK, TEXTURE_WHITE);

    Vector color;
    color = Vector{ 1.f, 1.f, 1.f };
    pShaderAPI->SetPixelShaderConstant(PSREG_SELFILLUMTINT, color.Base());
    // Setting up basecolor tint

    // Setting up environment map
    if (bHasEnvTexture)
    {
        if (pShaderAPI->InEditorMode())
        {
            pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
            pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP2, TEXTURE_BLACK);
        }
        else
        {
            BindTexture(SAMPLER_ENVMAP, info.envMap, 0);

            if (bLerpEnvMap && defGlobals != nullptr && defGlobals->pPrevEnvMap != nullptr)
            {
                BindTexture(SAMPLER_ENVMAP2, defGlobals->pPrevEnvMap);
            }
            else
                BindTexture(SAMPLER_ENVMAP2, info.envMap, 0);
        }
    }
    else
    {
        pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
        pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP2, TEXTURE_BLACK);
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
        ITexture* pFlashlightDepthTexture;
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

    // Setting up dynamic vertex shader
    DECLARE_DYNAMIC_VERTEX_SHADER(hlr_player_vs30);
    SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
    SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
    SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
    SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
    SET_DYNAMIC_VERTEX_SHADER(hlr_player_vs30);

    // Setting up dynamic pixel shader
    DECLARE_DYNAMIC_PIXEL_SHADER(hlr_player_ps30);
    SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
    SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
    SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
    SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
    SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
    SET_DYNAMIC_PIXEL_SHADER_COMBO(USECUSTOMCOLORS, bUseCustomColors);
    SET_DYNAMIC_PIXEL_SHADER(hlr_player_ps30);

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
        pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP2, TEXTURE_BLACK);
        pShaderAPI->BindStandardTexture(SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT);
        pShaderAPI->BindStandardTexture(SAMPLER_MRAO, TEXTURE_WHITE);
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

        const FlashlightState_t& flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
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

    float flExtraVars[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float flColor1[3] = { 1.0f, 1.0f, 1.0f };
    float flColor2[3] = { 1.0f, 1.0f, 1.0f };
    float flColor3[3] = { 1.0f, 1.0f, 0.0f };

    flColor1[0] = mat_player_primary_r.GetFloat() / 255.0f;
    flColor1[1] = mat_player_primary_g.GetFloat() / 255.0f;
    flColor1[2] = mat_player_primary_b.GetFloat() / 255.0f;
    SetPixelShaderConstantGammaToLinear(32, flColor1);

    flColor2[0] = mat_player_secondary_r.GetFloat() / 255.0f;
    flColor2[1] = mat_player_secondary_g.GetFloat() / 255.0f;
    flColor2[2] = mat_player_secondary_b.GetFloat() / 255.0f;
    SetPixelShaderConstantGammaToLinear(33, flColor2);

    flColor3[0] = mat_player_tertiary_r.GetFloat() / 255.0f;
    flColor3[1] = mat_player_tertiary_g.GetFloat() / 255.0f;
    flColor3[2] = mat_player_tertiary_b.GetFloat() / 255.0f;

    SetPixelShaderConstantGammaToLinear(34, flColor3);

    if (defGlobals != nullptr)
        flExtraVars[0] = defGlobals->m_flCubemapLerp;

    pShaderAPI->SetPixelShaderConstant(35, flExtraVars);
}

    // Actually draw the shader
    Draw();
};

// Closing it off
END_SHADER;