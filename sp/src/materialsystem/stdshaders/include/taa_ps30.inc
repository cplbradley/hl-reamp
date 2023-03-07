// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef TAA_PS30_H
#define TAA_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class taa_ps30_Static_Index
{
public:
	taa_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_taa_ps30 1


class taa_ps30_Dynamic_Index
{
	unsigned int m_nDRAWPREVFRAME : 2;
	unsigned int m_nDRAWDEPTHBUFFER : 2;
	unsigned int m_nDRAWVELOCITYBUFFER : 2;
	unsigned int m_nDRAWFRAMEBUFFER : 2;
#ifdef _DEBUG
	bool m_bDRAWPREVFRAME : 1;
	bool m_bDRAWDEPTHBUFFER : 1;
	bool m_bDRAWVELOCITYBUFFER : 1;
	bool m_bDRAWFRAMEBUFFER : 1;
#endif	// _DEBUG
public:
	void SetDRAWPREVFRAME( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDRAWPREVFRAME = i;
#ifdef _DEBUG
		m_bDRAWPREVFRAME = true;
#endif	// _DEBUG
	}

	void SetDRAWDEPTHBUFFER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDRAWDEPTHBUFFER = i;
#ifdef _DEBUG
		m_bDRAWDEPTHBUFFER = true;
#endif	// _DEBUG
	}

	void SetDRAWVELOCITYBUFFER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDRAWVELOCITYBUFFER = i;
#ifdef _DEBUG
		m_bDRAWVELOCITYBUFFER = true;
#endif	// _DEBUG
	}

	void SetDRAWFRAMEBUFFER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDRAWFRAMEBUFFER = i;
#ifdef _DEBUG
		m_bDRAWFRAMEBUFFER = true;
#endif	// _DEBUG
	}

	taa_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDRAWPREVFRAME = 0;
		m_nDRAWDEPTHBUFFER = 0;
		m_nDRAWVELOCITYBUFFER = 0;
		m_nDRAWFRAMEBUFFER = 0;
#ifdef _DEBUG
		m_bDRAWPREVFRAME = false;
		m_bDRAWDEPTHBUFFER = false;
		m_bDRAWVELOCITYBUFFER = false;
		m_bDRAWFRAMEBUFFER = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDRAWPREVFRAME && m_bDRAWDEPTHBUFFER && m_bDRAWVELOCITYBUFFER && m_bDRAWFRAMEBUFFER );
		return ( 1 * m_nDRAWPREVFRAME ) + ( 2 * m_nDRAWDEPTHBUFFER ) + ( 4 * m_nDRAWVELOCITYBUFFER ) + ( 8 * m_nDRAWFRAMEBUFFER ) + 0;
	}
};

#define shaderDynamicTest_taa_ps30 psh_forgot_to_set_dynamic_DRAWPREVFRAME + psh_forgot_to_set_dynamic_DRAWDEPTHBUFFER + psh_forgot_to_set_dynamic_DRAWVELOCITYBUFFER + psh_forgot_to_set_dynamic_DRAWFRAMEBUFFER


#endif	// TAA_PS30_H