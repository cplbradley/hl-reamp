// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $MULTITEXTURE && $BASETEXTURE
// $FLOWMAP && $MULTITEXTURE
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef FANCYWATER_VS20_H
#define FANCYWATER_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class fancywater_vs20_Static_Index
{
	unsigned int m_nBASETEXTURE : 2;
	unsigned int m_nMULTITEXTURE : 2;
	unsigned int m_nFLOWMAP : 2;
#ifdef _DEBUG
	bool m_bBASETEXTURE : 1;
	bool m_bMULTITEXTURE : 1;
	bool m_bFLOWMAP : 1;
#endif	// _DEBUG
public:
	void SetBASETEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURE = i;
#ifdef _DEBUG
		m_bBASETEXTURE = true;
#endif	// _DEBUG
	}

	void SetMULTITEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMULTITEXTURE = i;
#ifdef _DEBUG
		m_bMULTITEXTURE = true;
#endif	// _DEBUG
	}

	void SetFLOWMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLOWMAP = i;
#ifdef _DEBUG
		m_bFLOWMAP = true;
#endif	// _DEBUG
	}

	fancywater_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBASETEXTURE = 0;
		m_nMULTITEXTURE = 0;
		m_nFLOWMAP = 0;
#ifdef _DEBUG
		m_bBASETEXTURE = false;
		m_bMULTITEXTURE = false;
		m_bFLOWMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBASETEXTURE && m_bMULTITEXTURE && m_bFLOWMAP );
		return ( 1 * m_nBASETEXTURE ) + ( 2 * m_nMULTITEXTURE ) + ( 4 * m_nFLOWMAP ) + 0;
	}
};

#define shaderStaticTest_fancywater_vs20 vsh_forgot_to_set_static_BASETEXTURE + vsh_forgot_to_set_static_MULTITEXTURE + vsh_forgot_to_set_static_FLOWMAP


class fancywater_vs20_Dynamic_Index
{
public:
	fancywater_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_fancywater_vs20 1


#endif	// FANCYWATER_VS20_H