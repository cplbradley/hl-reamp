// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SIMPLEMOTIONBLUR_VS30_H
#define SIMPLEMOTIONBLUR_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class simplemotionblur_vs30_Static_Index
{
public:
	simplemotionblur_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_simplemotionblur_vs30 1


class simplemotionblur_vs30_Dynamic_Index
{
public:
	simplemotionblur_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_simplemotionblur_vs30 1


#endif	// SIMPLEMOTIONBLUR_VS30_H