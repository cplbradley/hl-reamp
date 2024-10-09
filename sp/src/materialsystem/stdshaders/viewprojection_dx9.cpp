#include "BaseVSShader.h"
#include "include/viewprojection_ps20b.inc"
#include "include/viewprojection_vs20.inc"


#include "cpp_shader_constant_register_map.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


const Sampler_t SAMPLER_BASETEXTURE = SHADER_SAMPLER0;


static ConVar building_cubemaps("building_cubemaps", "0");

struct viewproj_parms_t
{
	viewproj_parms_t()
	{
		memset(this, 0xFF, sizeof(*this));
	}

	int basetexture;
	int movementscale;
	int morphscale;
	int projectionscale;
	int rotationscale;
	int xmove;
	int ymove;
};


BEGIN_VS_SHADER(viewprojection, "")
	BEGIN_SHADER_PARAMS;
		SHADER_PARAM(MOVEMENTSCALE, SHADER_PARAM_TYPE_FLOAT, "0", "Overall scale of xy movement");
		SHADER_PARAM(MORPHSCALE, SHADER_PARAM_TYPE_FLOAT, "0", "Overall scale of morph");
		SHADER_PARAM(PROJECTIONSCALE, SHADER_PARAM_TYPE_FLOAT, "0", "Overall scale");
		SHADER_PARAM(ROTATIONSCALE, SHADER_PARAM_TYPE_FLOAT, "0", "Overall scale of rotation");
		SHADER_PARAM(XMOVEMENT, SHADER_PARAM_TYPE_FLOAT, "0", "X movement");
		SHADER_PARAM(YMOVEMENT, SHADER_PARAM_TYPE_FLOAT, "0", "Y movement");
	END_SHADER_PARAMS;

	void SetupVars(IMaterialVar** params, viewproj_parms_t& info)
	{
		info.basetexture = BASETEXTURE;
		info.movementscale = MOVEMENTSCALE;
		info.morphscale = MORPHSCALE;
		info.projectionscale = PROJECTIONSCALE;
		info.rotationscale = ROTATIONSCALE;
		info.xmove = XMOVEMENT;
		info.ymove = YMOVEMENT;
	};

	SHADER_INIT_PARAMS()
	{

	};

	SHADER_FALLBACK
	{
		return 0;
	};

	SHADER_INIT
	{
		viewproj_parms_t info;
		SetupVars(params, info);

		LoadTexture(info.basetexture, TEXTUREFLAGS_SRGB);
		InitFloatParam(info.movementscale, params, 0.f);
		InitFloatParam(info.morphscale, params, 0.f);
		InitFloatParam(info.projectionscale, params, 0.f);
		InitFloatParam(info.rotationscale, params, 0.f);
		InitFloatParam(info.xmove, params, 0.f);
		InitFloatParam(info.ymove, params, 0.f);
	};

	SHADER_DRAW
	{
		viewproj_parms_t info;
		SetupVars(params, info);

		bool bHasBaseTexture = (info.basetexture != -1) && params[info.basetexture]->IsTexture();
		bool bBuildingCubemaps = building_cubemaps.GetBool();


		if (IsSnapshotting())
		{
			pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE, true);
			pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);

			DECLARE_STATIC_VERTEX_SHADER(viewprojection_vs20);
			SET_STATIC_VERTEX_SHADER(viewprojection_vs20);
			DECLARE_STATIC_PIXEL_SHADER(viewprojection_ps20b);
			SET_STATIC_PIXEL_SHADER(viewprojection_ps20b);
		}
		else
		{
			if (bHasBaseTexture)
			{
				BindTexture(SAMPLER_BASETEXTURE, info.basetexture);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
			}


			DECLARE_DYNAMIC_VERTEX_SHADER(viewprojection_vs20);
			SET_DYNAMIC_VERTEX_SHADER(viewprojection_vs20);

			DECLARE_DYNAMIC_PIXEL_SHADER(viewprojection_ps20b);
			SET_DYNAMIC_PIXEL_SHADER(viewprojection_ps20b);


			float moveparms[4] = { 0.f,0.f,0.f,0.f };
			float cubemaps = bBuildingCubemaps ? 0 : 1;
			moveparms[0] = GetFloatParam(info.xmove, params);
			moveparms[1] = GetFloatParam(info.ymove, params);
			moveparms[2] = (float)pShaderAPI->CurrentTime();
			moveparms[3] = cubemaps;

			float scaleparms[4] = { 0.f,0.f,0.f,0.f };
			scaleparms[0] = GetFloatParam(info.projectionscale, params);
			scaleparms[1] = GetFloatParam(info.movementscale, params);
			scaleparms[2] = GetFloatParam(info.rotationscale, params);
			scaleparms[3] = GetFloatParam(info.morphscale, params);

			pShaderAPI->SetPixelShaderConstant(16, moveparms, 1);
			pShaderAPI->SetPixelShaderConstant(18, scaleparms, 1);
		}

		Draw();
	};

	END_SHADER;