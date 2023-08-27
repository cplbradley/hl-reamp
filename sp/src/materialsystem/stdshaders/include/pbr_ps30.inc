// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ($PIXELFOGTYPE == 0) && ($WRITEWATERFOGTODESTALPHA != 0)
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTDEPTHFILTERMODE != 0 )
// ( $LIGHTMAPPED_MODEL == 1 ) && ( $LIGHTMAPPED == 1 )
// ( $LIGHTMAPPED == 1 ) && ( $NUM_LIGHTS > 0 )
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

#ifndef PBR_PS30_H
#define PBR_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class pbr_ps30_Static_Index
{
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nFLASHLIGHTDEPTHFILTERMODE : 2;
	unsigned int m_nLIGHTMAPPED : 2;
	unsigned int m_nEMISSIVE : 2;
	unsigned int m_nPARALLAXOCCLUSION : 2;
	unsigned int m_nDETAILTEXTURE : 2;
	unsigned int m_nLERPENVMAP : 2;
#ifdef _DEBUG
	bool m_bFLASHLIGHT : 1;
	bool m_bFLASHLIGHTDEPTHFILTERMODE : 1;
	bool m_bLIGHTMAPPED : 1;
	bool m_bEMISSIVE : 1;
	bool m_bPARALLAXOCCLUSION : 1;
	bool m_bDETAILTEXTURE : 1;
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

	void SetLIGHTMAPPED( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTMAPPED = i;
#ifdef _DEBUG
		m_bLIGHTMAPPED = true;
#endif	// _DEBUG
	}

	void SetEMISSIVE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEMISSIVE = i;
#ifdef _DEBUG
		m_bEMISSIVE = true;
#endif	// _DEBUG
	}

	void SetPARALLAXOCCLUSION( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPARALLAXOCCLUSION = i;
#ifdef _DEBUG
		m_bPARALLAXOCCLUSION = true;
#endif	// _DEBUG
	}

	void SetDETAILTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAILTEXTURE = i;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
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

	pbr_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nFLASHLIGHT = 0;
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
		m_nLIGHTMAPPED = 0;
		m_nEMISSIVE = 0;
		m_nPARALLAXOCCLUSION = 0;
		m_nDETAILTEXTURE = 0;
		m_nLERPENVMAP = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
		m_bLIGHTMAPPED = false;
		m_bEMISSIVE = false;
		m_bPARALLAXOCCLUSION = false;
		m_bDETAILTEXTURE = false;
		m_bLERPENVMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHT && m_bFLASHLIGHTDEPTHFILTERMODE && m_bLIGHTMAPPED && m_bEMISSIVE && m_bPARALLAXOCCLUSION && m_bDETAILTEXTURE && m_bLERPENVMAP );
		return ( 160 * m_nFLASHLIGHT ) + ( 320 * m_nFLASHLIGHTDEPTHFILTERMODE ) + ( 960 * m_nLIGHTMAPPED ) + ( 1920 * m_nEMISSIVE ) + ( 3840 * m_nPARALLAXOCCLUSION ) + ( 7680 * m_nDETAILTEXTURE ) + ( 15360 * m_nLERPENVMAP ) + 0;
	}
};

#define shaderStaticTest_pbr_ps30 psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE + psh_forgot_to_set_static_LIGHTMAPPED + psh_forgot_to_set_static_EMISSIVE + psh_forgot_to_set_static_PARALLAXOCCLUSION + psh_forgot_to_set_static_DETAILTEXTURE + psh_forgot_to_set_static_LERPENVMAP


class pbr_ps30_Dynamic_Index
{
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
	unsigned int m_nLIGHTMAPPED_MODEL : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nWRITEWATERFOGTODESTALPHA : 2;
	unsigned int m_nNUM_LIGHTS : 3;
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
#ifdef _DEBUG
	bool m_bFLASHLIGHTSHADOWS : 1;
	bool m_bLIGHTMAPPED_MODEL : 1;
	bool m_bPIXELFOGTYPE : 1;
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
	bool m_bNUM_LIGHTS : 1;
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
#endif	// _DEBUG
public:
	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif	// _DEBUG
	}

	void SetLIGHTMAPPED_MODEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTMAPPED_MODEL = i;
#ifdef _DEBUG
		m_bLIGHTMAPPED_MODEL = true;
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

	pbr_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nFLASHLIGHTSHADOWS = 0;
		m_nLIGHTMAPPED_MODEL = 0;
		m_nPIXELFOGTYPE = 0;
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nNUM_LIGHTS = 0;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = false;
		m_bLIGHTMAPPED_MODEL = false;
		m_bPIXELFOGTYPE = false;
		m_bWRITEWATERFOGTODESTALPHA = false;
		m_bNUM_LIGHTS = false;
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHTSHADOWS && m_bLIGHTMAPPED_MODEL && m_bPIXELFOGTYPE && m_bWRITEWATERFOGTODESTALPHA && m_bNUM_LIGHTS && m_bWRITE_DEPTH_TO_DESTALPHA );
		return ( 1 * m_nFLASHLIGHTSHADOWS ) + ( 2 * m_nLIGHTMAPPED_MODEL ) + ( 4 * m_nPIXELFOGTYPE ) + ( 8 * m_nWRITEWATERFOGTODESTALPHA ) + ( 16 * m_nNUM_LIGHTS ) + ( 80 * m_nWRITE_DEPTH_TO_DESTALPHA ) + 0;
	}
};

#define shaderDynamicTest_pbr_ps30 psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_LIGHTMAPPED_MODEL + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA


#endif	// PBR_PS30_H