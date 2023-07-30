#pragma once
#ifndef PBR_HELPER_H
#define PBR_HELPER_H

#include <string.h>
#include "BaseVSShader.h"

class CBaseVSShader;
class IMaterialVar;
class IShaderDynamicAPI;
class IShaderShadow;


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
    int m_iDetailBlendMode;

    int flatMRAO;
    //int m_nDetailTextureCombineMode;
    int m_nDetailTextureBlendFactor;

};

void StaticShaderPBRModelCombos(IShaderShadow* pShaderShadow, bool bHasEmissionTexture, bool bHasFlashlight, bool bHasDetailTexture, bool bUseParallax);
void DynamicShaderPBRModelCombos(IShaderDynamicAPI* pShaderAPI, bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, bool bFlashlightShadows, bool bLightMappedModel, int iNumLights);

#endif //PBR_HELPER_H