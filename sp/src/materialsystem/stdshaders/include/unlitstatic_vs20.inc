// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef UNLITSTATIC_VS20_H
#define UNLITSTATIC_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class unlitstatic_vs20_Static_Index
{
public:
	unlitstatic_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_unlitstatic_vs20 1


class unlitstatic_vs20_Dynamic_Index
{
	unsigned int m_nSKINNING : 2;
#ifdef _DEBUG
	bool m_bSKINNING : 1;
#endif	// _DEBUG
public:
	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif	// _DEBUG
	}

	unlitstatic_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nSKINNING = 0;
#ifdef _DEBUG
		m_bSKINNING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bSKINNING );
		return ( 1 * m_nSKINNING ) + 0;
	}
};

#define shaderDynamicTest_unlitstatic_vs20 vsh_forgot_to_set_dynamic_SKINNING


#endif	// UNLITSTATIC_VS20_H