// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ($PIXELFOGTYPE == 0) && ($WRITEWATERFOGTODESTALPHA != 0)
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTDEPTHFILTERMODE != 0 )
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef HLR_PLAYER_PS30_H
#define HLR_PLAYER_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class hlr_player_ps30_Static_Index
{
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nFLASHLIGHTDEPTHFILTERMODE : 2;
	unsigned int m_nLERPENVMAP : 2;
#ifdef _DEBUG
	bool m_bFLASHLIGHT : 1;
	bool m_bFLASHLIGHTDEPTHFILTERMODE : 1;
	bool m_bLERPENVMAP : 1;
#endif	// _DEBUG
public:
	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHTDEPTHFILTERMODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nFLASHLIGHTDEPTHFILTERMODE = i;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = true;
#endif	// _DEBUG
	}

	void SetLERPENVMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLERPENVMAP = i;
#ifdef _DEBUG
		m_bLERPENVMAP = true;
#endif	// _DEBUG
	}

	hlr_player_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nFLASHLIGHT = 0;
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
		m_nLERPENVMAP = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
		m_bLERPENVMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHT && m_bFLASHLIGHTDEPTHFILTERMODE && m_bLERPENVMAP );
		return ( 160 * m_nFLASHLIGHT ) + ( 320 * m_nFLASHLIGHTDEPTHFILTERMODE ) + ( 960 * m_nLERPENVMAP ) + 0;
	}
};

#define shaderStaticTest_hlr_player_ps30 psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE + psh_forgot_to_set_static_LERPENVMAP


class hlr_player_ps30_Dynamic_Index
{
	unsigned int m_nUSECUSTOMCOLORS : 2;
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nWRITEWATERFOGTODESTALPHA : 2;
	unsigned int m_nNUM_LIGHTS : 3;
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
#ifdef _DEBUG
	bool m_bUSECUSTOMCOLORS : 1;
	bool m_bFLASHLIGHTSHADOWS : 1;
	bool m_bPIXELFOGTYPE : 1;
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
	bool m_bNUM_LIGHTS : 1;
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
#endif	// _DEBUG
public:
	void SetUSECUSTOMCOLORS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUSECUSTOMCOLORS = i;
#ifdef _DEBUG
		m_bUSECUSTOMCOLORS = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif	// _DEBUG
	}

	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif	// _DEBUG
	}

	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif	// _DEBUG
	}

	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif	// _DEBUG
	}

	hlr_player_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nUSECUSTOMCOLORS = 0;
		m_nFLASHLIGHTSHADOWS = 0;
		m_nPIXELFOGTYPE = 0;
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nNUM_LIGHTS = 0;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bUSECUSTOMCOLORS = false;
		m_bFLASHLIGHTSHADOWS = false;
		m_bPIXELFOGTYPE = false;
		m_bWRITEWATERFOGTODESTALPHA = false;
		m_bNUM_LIGHTS = false;
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bUSECUSTOMCOLORS && m_bFLASHLIGHTSHADOWS && m_bPIXELFOGTYPE && m_bWRITEWATERFOGTODESTALPHA && m_bNUM_LIGHTS && m_bWRITE_DEPTH_TO_DESTALPHA );
		return ( 1 * m_nUSECUSTOMCOLORS ) + ( 2 * m_nFLASHLIGHTSHADOWS ) + ( 4 * m_nPIXELFOGTYPE ) + ( 8 * m_nWRITEWATERFOGTODESTALPHA ) + ( 16 * m_nNUM_LIGHTS ) + ( 80 * m_nWRITE_DEPTH_TO_DESTALPHA ) + 0;
	}
};

#define shaderDynamicTest_hlr_player_ps30 psh_forgot_to_set_dynamic_USECUSTOMCOLORS + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA


#endif	// HLR_PLAYER_PS30_H