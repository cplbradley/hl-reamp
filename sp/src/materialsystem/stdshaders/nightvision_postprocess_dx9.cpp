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

#include "include/nightvision_postprocess_vs30.inc"
#include "include/nightvision_postprocess_ps30.inc"


static ConVar hud_color_r("hud_color_r", "0");
static ConVar hud_color_g("hud_color_r", "0");
static ConVar hud_color_b("hud_color_r", "0");
static ConVar g_custom_nightvision("g_custom_nightvision", "0");

const Sampler_t SAMPLER_FRAMEBUFFER = SHADER_SAMPLER0;
const Sampler_t SAMPLER_NOISETEXTURE = SHADER_SAMPLER1;
const Sampler_t SAMPLER_DEPTHBUFFER = SHADER_SAMPLER2;
const Sampler_t SAMPLER_DETAIL = SHADER_SAMPLER3;

struct NVVars_t
{
	NVVars_t()
	{
		memset(this, 0xFF, sizeof(*this));
	}
	int noisetexture;
	int framebuffer;
	int depthbuffer;
	int detailtexture;
	int edgestrength;

};

BEGIN_VS_SHADER( nightvision_postprocess, "" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM(FRAMEBUFFER,SHADER_PARAM_TYPE_TEXTURE,"","")
		SHADER_PARAM(NOISETEXTURE,SHADER_PARAM_TYPE_TEXTURE, "","")
		SHADER_PARAM(DEPTHBUFFER,SHADER_PARAM_TYPE_TEXTURE,"","")
		SHADER_PARAM(EDGESTRENGTH,SHADER_PARAM_TYPE_FLOAT,"3","")
		SHADER_PARAM(DETAILTEXTURE,SHADER_PARAM_TYPE_TEXTURE,"","")
	END_SHADER_PARAMS

	void SetupVars(NVVars_t& info )
	{
	info.noisetexture = NOISETEXTURE;
	info.edgestrength = EDGESTRENGTH;
	info.framebuffer = FRAMEBUFFER;
	info.detailtexture = DETAILTEXTURE;
	info.depthbuffer = DEPTHBUFFER;
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
		NVVars_t info;
		SetupVars(info);
		LoadTexture(info.framebuffer);
		LoadTexture(info.depthbuffer);
		LoadTexture(info.noisetexture);
		LoadTexture(info.detailtexture);
	}

	SHADER_DRAW
	{
		NVVars_t info;
		SetupVars(info);

		if(IsSnapshotting())
		{
			unsigned int flags = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
			pShaderShadow->EnableTexture(SAMPLER_FRAMEBUFFER, true);
			pShaderShadow->EnableSRGBRead(SAMPLER_FRAMEBUFFER, false);
			pShaderShadow->EnableTexture(SAMPLER_NOISETEXTURE, true);
			pShaderShadow->EnableSRGBRead(SAMPLER_NOISETEXTURE, false);
			pShaderShadow->EnableTexture(SAMPLER_DEPTHBUFFER, true);
			pShaderShadow->EnableSRGBRead(SAMPLER_DEPTHBUFFER, false);
			pShaderShadow->EnableTexture(SAMPLER_DETAIL, true);
			pShaderShadow->EnableSRGBRead(SAMPLER_DETAIL, false);

			DECLARE_STATIC_PIXEL_SHADER(nightvision_postprocess_ps30);
			SET_STATIC_PIXEL_SHADER(nightvision_postprocess_ps30);
			DECLARE_STATIC_VERTEX_SHADER(nightvision_postprocess_vs30);
			SET_STATIC_VERTEX_SHADER(nightvision_postprocess_vs30);
		}
		else
		{
			DECLARE_DYNAMIC_PIXEL_SHADER(nightvision_postprocess_ps30);
			SET_DYNAMIC_PIXEL_SHADER(nightvision_postprocess_ps30);
			DECLARE_DYNAMIC_VERTEX_SHADER(nightvision_postprocess_vs30);
			SET_DYNAMIC_VERTEX_SHADER(nightvision_postprocess_vs30);


				BindTexture(SAMPLER_NOISETEXTURE,info.noisetexture);
				BindTexture(SAMPLER_FRAMEBUFFER, info.framebuffer);
				BindTexture(SAMPLER_DEPTHBUFFER, info.depthbuffer);
				BindTexture(SAMPLER_DETAIL, info.detailtexture);


			
			float randnoise[3] = { 0.0f, 0.0f, 0.0f };
			Vector vecRand = RandomVector(0.0f, 10.0f);
			randnoise[0] = vecRand[0];
			randnoise[1] = vecRand[1];
			randnoise[2] = pShaderAPI->CurrentTime();

			pShaderAPI->SetPixelShaderConstant(0, randnoise);

			float hudcolor[3] = { 0.f, 0.f, 0.f };
			hudcolor[0] = hud_color_r.GetFloat();
			hudcolor[1] = hud_color_g.GetFloat();
			hudcolor[2] = hud_color_b.GetFloat();

			if (!g_custom_nightvision.GetBool())
			{
				hudcolor[0] = 0;
				hudcolor[1] = 255;
				hudcolor[2] = 0;
			}
			pShaderAPI->SetPixelShaderConstant(1, hudcolor);

			ShaderViewport_t vp;
			pShaderAPI->GetViewports(&vp,1);
			float texel[3] = { 0.f, 0.f, 0.f };
			texel[0] = 1.0f / vp.m_nHeight;
			texel[1] = 1.0f / vp.m_nWidth;
			texel[2] = params[info.edgestrength]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(2, texel);
		}
		Draw();
	}
	END_SHADER;
