#include "BaseVSShader.h"
#include "include/smb_vs30.inc"
#include "include/smb_ps30.inc"
#include "VIEW_SHARED.H"

const Sampler_t SAMPLER_FBCURFRAME = SHADER_SAMPLER0;
const Sampler_t SAMPLER_FBPREVFRAME = SHADER_SAMPLER1;
const Sampler_t SAMPLER_DEPTH = SHADER_SAMPLER2;

static ConVar mat_simplemotionblur_scale("mat_simplemotionblur_scale", "1");
static ConVar mat_simplemotionblur("mat_simplemotionblur", "0");
static ConVar mat_smb_showdiff("mat_smb_showdiff", "0");





struct SMB_Params_t
{
	int curframe;
	int prevframe;
};


BEGIN_VS_SHADER(SMB, "SMB")
BEGIN_SHADER_PARAMS
SHADER_PARAM(CURFRAME, SHADER_PARAM_TYPE_TEXTURE, "", "");
SHADER_PARAM(PREVFRAME, SHADER_PARAM_TYPE_TEXTURE, "", "");
END_SHADER_PARAMS;

void SetupVars(IMaterialVar** params, SMB_Params_t& info)
{
	info.curframe = CURFRAME;
	info.prevframe = PREVFRAME;
}


SHADER_INIT_PARAMS() {};

SHADER_FALLBACK{ return 0; }

SHADER_INIT
{
	SMB_Params_t info;
	SetupVars(params, info);
	LoadTexture(info.curframe);
	LoadTexture(info.prevframe);

};


SHADER_DRAW
{
	SMB_Params_t info;
	SetupVars(params, info);

	if (IsSnapshotting())
	{
		pShaderShadow->EnableTexture(SAMPLER_FBCURFRAME, true);
		pShaderShadow->EnableSRGBRead(SAMPLER_FBCURFRAME, false);
		pShaderShadow->EnableTexture(SAMPLER_FBPREVFRAME, true);
		pShaderShadow->EnableSRGBRead(SAMPLER_FBPREVFRAME, false);
		

		int fmt = VERTEX_POSITION;
		pShaderShadow->VertexShaderVertexFormat(fmt, 1, 0, 0);

		DECLARE_STATIC_VERTEX_SHADER(smb_vs30);
		SET_STATIC_VERTEX_SHADER(smb_vs30);

		DECLARE_STATIC_PIXEL_SHADER(smb_ps30);
		SET_STATIC_PIXEL_SHADER(smb_ps30);
	}
	else
	{
		BindTexture(SAMPLER_FBCURFRAME, info.curframe);
		BindTexture(SAMPLER_FBPREVFRAME, info.prevframe);
		ClientRender_Globals_t* defGlobals = (ClientRender_Globals_t*)pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_GLOBALS_PTR);

		DECLARE_DYNAMIC_VERTEX_SHADER(smb_vs30);
		SET_DYNAMIC_VERTEX_SHADER(smb_vs30);
		DECLARE_DYNAMIC_PIXEL_SHADER(smb_ps30);
		SET_DYNAMIC_PIXEL_SHADER(smb_ps30);

		float blurscale = MIN(mat_simplemotionblur_scale.GetFloat(), 1.0f);
		float scale = blurscale * 0.5f;

		float scaleconst[2];
		scaleconst[0] = scale;
		scaleconst[1] = mat_smb_showdiff.GetBool() ? 1 : 0;

		
	
		pShaderAPI->SetPixelShaderConstant(1, scaleconst, 1);
		pShaderAPI->SetPixelShaderConstant(2, defGlobals->inverseViewProjection.Base(), 4);
		pShaderAPI->SetPixelShaderConstant(6, defGlobals->lastViewProjection.Base(), 4);
	}

	Draw();



};

END_SHADER;

