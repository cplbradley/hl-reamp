// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef NIGHTVISION_POSTPROCESS_VS30_H
#define NIGHTVISION_POSTPROCESS_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class nightvision_postprocess_vs30_Static_Index
{
public:
	nightvision_postprocess_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_nightvision_postprocess_vs30 1


class nightvision_postprocess_vs30_Dynamic_Index
{
public:
	nightvision_postprocess_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_nightvision_postprocess_vs30 1


#endif	// NIGHTVISION_POSTPROCESS_VS30_H