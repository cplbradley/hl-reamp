#include "BaseVSShader.h"
#include "include/taa_vs30.inc"
#include "include/taa_ps30.inc"
#include "view_shared.h"

const Sampler_t SAMPLER_FBCURFRAME = SHADER_SAMPLER0;
const Sampler_t SAMPLER_FBPREVFRAME = SHADER_SAMPLER1;
const Sampler_t SAMPLER_DEPTHBUFFER = SHADER_SAMPLER2;

static ConVar mat_drawprevframe("mat_drawprevframe", "0");
static ConVar mat_drawvelocitybuffer("mat_drawvelocitybuffer", "0");
static ConVar mat_drawframebuffer("mat_drawframebuffer", "0");
static ConVar mat_drawdepthbuffer("mat_drawdepthbuffer", "0");

static ConVar mat_numsamples("mat_numsamples", "2");





struct TAA_Params_t
{
	int curframe;
	int prevframe;
	int depthbuffer;
};


BEGIN_VS_SHADER(TAA, "TAA")
BEGIN_SHADER_PARAMS
SHADER_PARAM(CURFRAME, SHADER_PARAM_TYPE_TEXTURE, "", "");
SHADER_PARAM(PREVFRAME, SHADER_PARAM_TYPE_TEXTURE, "", "");
SHADER_PARAM(DEPTHBUFFERFRAME, SHADER_PARAM_TYPE_TEXTURE, "", "");
END_SHADER_PARAMS;

void SetupVars(IMaterialVar** params, TAA_Params_t& info)
{
	info.curframe = CURFRAME;
	info.prevframe = PREVFRAME;
	info.depthbuffer = DEPTHBUFFERFRAME;
}


SHADER_INIT_PARAMS() {};

SHADER_FALLBACK{ return 0; }

SHADER_INIT
{
	TAA_Params_t info;
	SetupVars(params, info);
	LoadTexture(info.curframe);
	LoadTexture(info.prevframe);
	LoadTexture(info.depthbuffer);

};


SHADER_DRAW
{
	TAA_Params_t info;
	SetupVars(params, info);

	if (IsSnapshotting())
	{
		pShaderShadow->EnableTexture(SAMPLER_FBCURFRAME, true);
		pShaderShadow->EnableSRGBRead(SAMPLER_FBCURFRAME, false);
		pShaderShadow->EnableTexture(SAMPLER_FBPREVFRAME, true);
		pShaderShadow->EnableSRGBRead(SAMPLER_FBPREVFRAME, false);
		pShaderShadow->EnableTexture(SAMPLER_DEPTHBUFFER, true);
		pShaderShadow->EnableSRGBRead(SAMPLER_DEPTHBUFFER, false);
		pShaderShadow->EnableAlphaWrites(true);

		int fmt = VERTEX_POSITION;
		pShaderShadow->VertexShaderVertexFormat(fmt, 1, 0, 0);

		DECLARE_STATIC_VERTEX_SHADER(taa_vs30);
		SET_STATIC_VERTEX_SHADER(taa_vs30);

		DECLARE_STATIC_PIXEL_SHADER(taa_ps30);
		SET_STATIC_PIXEL_SHADER(taa_ps30);
	}
	else
	{
		
		ClientRender_Globals_t* defGlobals = (ClientRender_Globals_t*)pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_GLOBALS_PTR);

		BindTexture(SAMPLER_FBCURFRAME, info.curframe);
		BindTexture(SAMPLER_FBPREVFRAME, info.prevframe);
		BindTexture(SAMPLER_DEPTHBUFFER, info.depthbuffer);

		bool bDrawPrevFrame = mat_drawprevframe.GetBool();
		bool bDrawVelocityBuffer = mat_drawvelocitybuffer.GetBool();
		bool bDrawFrameBuffer = mat_drawframebuffer.GetBool();
		bool bDrawDepthBuffer = mat_drawdepthbuffer.GetBool();

		DECLARE_DYNAMIC_VERTEX_SHADER(taa_vs30);
		SET_DYNAMIC_VERTEX_SHADER(taa_vs30);
		DECLARE_DYNAMIC_PIXEL_SHADER(taa_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(DRAWPREVFRAME, bDrawPrevFrame);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(DRAWVELOCITYBUFFER, bDrawVelocityBuffer);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(DRAWDEPTHBUFFER, bDrawDepthBuffer);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(DRAWFRAMEBUFFER, bDrawFrameBuffer);
		SET_DYNAMIC_PIXEL_SHADER(taa_ps30);


		
		pShaderAPI->SetPixelShaderConstant(1, defGlobals->curViewProjection.Base() , 4);
		pShaderAPI->SetPixelShaderConstant(10, defGlobals->inverseViewProjection.Base(), 4);
		pShaderAPI->SetPixelShaderConstant(20, defGlobals->lastViewProjection.Base(), 4);
		float numsamples[4];
		numsamples[0] = mat_numsamples.GetFloat();
		ShaderViewport_t vp;
		pShaderAPI->GetViewports(&vp, 1);
		int vpWide = vp.m_nWidth;
		int vpTall = vp.m_nHeight;
		numsamples[1] = vpWide;
		numsamples[2] = vpTall;
		numsamples[3] = pShaderAPI->CurrentTime();
		pShaderAPI->SetPixelShaderConstant(27, numsamples, 1);
	}
	
	Draw();



};

END_SHADER;

