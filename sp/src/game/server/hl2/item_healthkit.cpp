//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements health kits and wall mounted health chargers.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "items.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "Sprite.h"
#include "SpriteTrail.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_healthkit( "sk_healthkit","0" );		
ConVar	sk_healthvial( "sk_healthvial","0" );		
ConVar	sk_healthcharger( "sk_healthcharger","0" );		
ConVar	sk_healthkit_small("sk_healthkit_small", "0");

//-----------------------------------------------------------------------------
// Small health kit. Heals the player when picked up.
//-----------------------------------------------------------------------------
class CHealthKit : public CItem
{
public:
	DECLARE_CLASS( CHealthKit, CItem );
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );
	void CreateEffects(void);
	bool MyTouch( CBasePlayer *pPlayer );
protected:
	CHandle<CSprite>	m_pSprite;
};

LINK_ENTITY_TO_CLASS( item_healthkit, CHealthKit );
PRECACHE_REGISTER(item_healthkit);

BEGIN_DATADESC(CHealthKit)
DEFINE_FIELD(m_pSprite,FIELD_EHANDLE),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKit::Spawn( void )
{
	Precache();
	SetModel( "models/items/healthkit.mdl" );
	CreateEffects();
	BaseClass::Spawn();
}

void CHealthKit::CreateEffects(void)
{
	m_pSprite = CSprite::SpriteCreate("sprites/health.vmt", GetAbsOrigin(), false);

	if (m_pSprite != NULL)
	{
		m_pSprite->FollowEntity(this);
		m_pSprite->SetLocalOrigin(vec3_origin + Vector(0, 0, 16));
		m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxHologram);
		m_pSprite->SetScale(0.4f);
		m_pSprite->SetGlowProxySize(16.0f);
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKit::Precache( void )
{
	PrecacheModel("models/items/healthkit.mdl");
	PrecacheMaterial("sprites/health.vmt");
	PrecacheScriptSound( "HealthKit.Touch" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : 
//-----------------------------------------------------------------------------
bool CHealthKit::MyTouch( CBasePlayer *pPlayer )
{
	if ( pPlayer->TakeHealth( sk_healthkit.GetFloat(), DMG_GENERIC ) )
	{
		CPASAttenuationFilter filter( pPlayer, "HealthKit.Touch" );
		EmitSound( filter, pPlayer->entindex(), "HealthKit.Touch" );

		if ( g_pGameRules->ItemShouldRespawn( this ) )
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);	
		}

		return true;
	}

	return false;
}

class CHealthKitSmall : public CItem
{
public:
	DECLARE_CLASS(CHealthKitSmall, CItem);
	DECLARE_DATADESC();

	void Spawn(void);
	void Precache(void);
	void CreateEffects(void);
	bool MyTouch(CBasePlayer* pPlayer);
protected:
	CHandle<CSprite>	m_pSprite;
};

LINK_ENTITY_TO_CLASS(item_healthkit_small, CHealthKitSmall);
PRECACHE_REGISTER(item_healthkit_small);

BEGIN_DATADESC(CHealthKitSmall)
DEFINE_FIELD(m_pSprite, FIELD_EHANDLE),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKitSmall::Spawn(void)
{
	Precache();
	SetModel("models/items/healthkit_small.mdl");
	CreateEffects();
	BaseClass::Spawn();
}

void CHealthKitSmall::CreateEffects(void)
{
	m_pSprite = CSprite::SpriteCreate("sprites/health.vmt", GetAbsOrigin(), false);

	if (m_pSprite != NULL)
	{
		m_pSprite->FollowEntity(this);
		m_pSprite->SetLocalOrigin(vec3_origin + Vector(0, 0, 16));
		m_pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxHologram);
		m_pSprite->SetScale(0.4f);
		m_pSprite->SetGlowProxySize(16.0f);
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHealthKitSmall::Precache(void)
{
	PrecacheModel("models/items/healthkit_small.mdl");
	PrecacheMaterial("sprites/health.vmt");
	PrecacheScriptSound("HealthKit.Touch");
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : 
//-----------------------------------------------------------------------------
bool CHealthKitSmall::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer->TakeHealth(sk_healthkit_small.GetFloat(), DMG_GENERIC))
	{
		CPASAttenuationFilter filter(pPlayer, "HealthKit.Touch");
		EmitSound(filter, pPlayer->entindex(), "HealthKit.Touch");

		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}

		return true;
	}

	return false;
}
//-----------------------------------------------------------------------------
// Small dynamically dropped health kit
//-----------------------------------------------------------------------------

class CHealthVial : public CItem
{
public:
	DECLARE_CLASS( CHealthVial, CItem );
	DECLARE_DATADESC();
protected:
	CHandle<CSpriteTrail>	m_pGlowTrail;
	CHandle<CSprite>		m_pMainGlow;
	void Spawn( void )
	{
		Precache();
		SetModel( "models/items/resdrop.mdl" );
		SetRenderColor(0, 255, 50);
		CreateEffects();
		SetThink(&CHealthVial::DelayedKill);
		SetNextThink(gpGlobals->curtime + 15.0f);
		BaseClass::Spawn();
	}

	void Precache( void )
	{
		PrecacheModel("models/items/resdrop.mdl");
		PrecacheModel("sprites/laser.vmt");
		PrecacheModel("sprites/glow04.vmt");
		PrecacheScriptSound( "HealthVial.Touch" );
	}
	void CreateEffects( void )
	{
		m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/laser.vmt", GetLocalOrigin(), false);

		if (m_pGlowTrail != NULL)
		{
			m_pGlowTrail->FollowEntity(this);
			m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 255, 0, 255, kRenderFxNone);
			m_pGlowTrail->SetStartWidth(14.0f);
			m_pGlowTrail->SetEndWidth(0.0f);
			m_pGlowTrail->SetLifeTime(1.0f);
		}
		m_pMainGlow = CSprite::SpriteCreate("sprites/animglow01.vmt", GetLocalOrigin(), false);


		if (m_pMainGlow != NULL)
		{
			m_pMainGlow->FollowEntity(this);
			m_pMainGlow->SetTransparency(kRenderGlow, 0, 255, 0, 200, kRenderFxNoDissipation);
			m_pMainGlow->SetScale(0.45f);
			m_pMainGlow->SetGlowProxySize(4.0f);
		}

	}
	void CheckQuantity(void)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		int hp = pPlayer->GetHealth();
		if (hp < pPlayer->GetMaxHealth())
		{
			m_bShouldSeek = true;
		}
		else
		{
			m_bShouldSeek = false;
		}

	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->TakeHealth( sk_healthvial.GetFloat(), DMG_GENERIC ) )
		{
			CPASAttenuationFilter filter( pPlayer, "HealthVial.Touch" );
			EmitSound( filter, pPlayer->entindex(), "HealthVial.Touch" );

			if ( g_pGameRules->ItemShouldRespawn( this ) )
			{
				Respawn();
			}
			else
			{
				UTIL_Remove(this);	
			}

			return true;
		}

		return false;
	}
	void DelayedKill(void)
	{
		UTIL_Remove(this);
	}
};

LINK_ENTITY_TO_CLASS( item_healthvial, CHealthVial );
PRECACHE_REGISTER( item_healthvial );
BEGIN_DATADESC(CHealthVial)
DEFINE_THINKFUNC(DelayedKill),
END_DATADESC()

//-----------------------------------------------------------------------------
// Wall mounted health kit. Heals the player when used.
//-----------------------------------------------------------------------------
class CWallHealth : public CBaseToggle
{
public:
	DECLARE_CLASS( CWallHealth, CBaseToggle );

	void Spawn( );
	void Precache( void );
	int  DrawDebugTextOverlays(void);
	bool CreateVPhysics(void);
	void Off(void);
	void Recharge(void);
	bool KeyValue(  const char *szKeyName, const char *szValue );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() | m_iCaps; }

	float m_flNextCharge; 
	int		m_iReactivate ; // DeathMatch Delay until reactvated
	int		m_iJuice;
	int		m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;

	int		m_nState;
	int		m_iCaps;

	COutputFloat m_OutRemainingHealth;
	COutputEvent m_OnPlayerUse;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);


BEGIN_DATADESC( CWallHealth )

	DEFINE_FIELD( m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD( m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD( m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD( m_iOn, FIELD_INTEGER),
	DEFINE_FIELD( m_flSoundTime, FIELD_TIME),
	DEFINE_FIELD( m_nState, FIELD_INTEGER ),
	DEFINE_FIELD( m_iCaps, FIELD_INTEGER ),

	// Function Pointers
	DEFINE_FUNCTION( Off ),
	DEFINE_FUNCTION( Recharge ),

	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),
	DEFINE_OUTPUT( m_OutRemainingHealth, "OutRemainingHealth"),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pkvd - 
//-----------------------------------------------------------------------------
bool CWallHealth::KeyValue(  const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "style") ||
		FStrEq(szKeyName, "height") ||
		FStrEq(szKeyName, "value1") ||
		FStrEq(szKeyName, "value2") ||
		FStrEq(szKeyName, "value3"))
	{
		return(true);
	}
	else if (FStrEq(szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(szValue);
		return(true);
	}

	return(BaseClass::KeyValue( szKeyName, szValue ));
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWallHealth::Spawn(void)
{
	Precache( );

	SetSolid( SOLID_BSP );
	SetMoveType( MOVETYPE_PUSH );

	SetModel( STRING( GetModelName() ) );

	m_iJuice = sk_healthcharger.GetFloat();

	m_nState = 0;	
	
	m_iCaps	= FCAP_CONTINUOUS_USE;

	CreateVPhysics();
}

int CWallHealth::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];
		Q_snprintf(tempstr,sizeof(tempstr),"Charge left: %i", m_iJuice );
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}

//-----------------------------------------------------------------------------

bool CWallHealth::CreateVPhysics(void)
{
	VPhysicsInitStatic();
	return true;

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWallHealth::Precache(void)
{
	PrecacheScriptSound( "WallHealth.Deny" );
	PrecacheScriptSound( "WallHealth.Start" );
	PrecacheScriptSound( "WallHealth.LoopingContinueCharge" );
	PrecacheScriptSound( "WallHealth.Recharge" );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CWallHealth::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	// Make sure that we have a caller
	if (!pActivator)
		return;

	// if it's not a player, ignore
	if ( !pActivator->IsPlayer() )
		return;

	CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(pActivator);

	// Reset to a state of continuous use.
	m_iCaps = FCAP_CONTINUOUS_USE;

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		m_nState = 1;			
		Off();
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise.
	// disabled HEV suit dependency for now.
	//if ((m_iJuice <= 0) || (!(pActivator->m_bWearingSuit)))
	if (m_iJuice <= 0)
	{
		if (m_flSoundTime <= gpGlobals->curtime)
		{
			m_flSoundTime = gpGlobals->curtime + 0.62;
			EmitSound( "WallHealth.Deny" );
		}
		return;
	}

	if( pActivator->GetHealth() >= pActivator->GetMaxHealth() )
	{
		if( pPlayer )
		{
			pPlayer->m_afButtonPressed &= ~IN_USE;
		}

		// Make the user re-use me to get started drawing health.
		m_iCaps = FCAP_IMPULSE_USE;

		return;
	}

	SetNextThink( gpGlobals->curtime + 0.25f );
	SetThink(&CWallHealth::Off);

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->curtime)
		return;

	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		EmitSound( "WallHealth.Start" );
		m_flSoundTime = 0.56 + gpGlobals->curtime;

		m_OnPlayerUse.FireOutput( pActivator, this );
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->curtime))
	{
		m_iOn++;
		CPASAttenuationFilter filter( this, "WallHealth.LoopingContinueCharge" );
		filter.MakeReliable();
		EmitSound( filter, entindex(), "WallHealth.LoopingContinueCharge" );
	}

	// charge the player
	if ( pActivator->TakeHealth( 1, DMG_GENERIC ) )
	{
		m_iJuice--;
	}

	// Send the output.
	float flRemaining = m_iJuice / sk_healthcharger.GetFloat();
	m_OutRemainingHealth.Set(flRemaining, pActivator, this);

	// govern the rate of charge
	m_flNextCharge = gpGlobals->curtime + 0.1;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWallHealth::Recharge(void)
{
	EmitSound( "WallHealth.Recharge" );
	m_iJuice = sk_healthcharger.GetFloat();
	m_nState = 0;			
	SetThink( NULL );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWallHealth::Off(void)
{
	// Stop looping sound.
	if (m_iOn > 1)
		StopSound( "WallHealth.LoopingContinueCharge" );

	m_iOn = 0;

	if ((!m_iJuice) &&  ( ( m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime() ) > 0) )
	{
		SetNextThink( gpGlobals->curtime + m_iReactivate );
		SetThink(&CWallHealth::Recharge);
	}
	else
		SetThink( NULL );
}

//-----------------------------------------------------------------------------
// Wall mounted health kit. Heals the player when used.
//-----------------------------------------------------------------------------
class CNewWallHealth : public CBaseAnimating
{
public:
	DECLARE_CLASS( CNewWallHealth, CBaseAnimating );

	void Spawn( );
	void Precache( void );
	int  DrawDebugTextOverlays(void);
	bool CreateVPhysics(void);
	void Off(void);
	void Recharge(void);
	bool KeyValue(  const char *szKeyName, const char *szValue );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() | m_iCaps; }

	float m_flNextCharge; 
	int		m_iReactivate ; // DeathMatch Delay until reactvated
	int		m_iJuice;
	int		m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;

	int		m_nState;
	int		m_iCaps;

	COutputFloat m_OutRemainingHealth;
	COutputEvent m_OnPlayerUse;

	void StudioFrameAdvance ( void );

	float m_flJuice;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( item_healthcharger, CNewWallHealth);


BEGIN_DATADESC( CNewWallHealth )

	DEFINE_FIELD( m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD( m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD( m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD( m_iOn, FIELD_INTEGER),
	DEFINE_FIELD( m_flSoundTime, FIELD_TIME),
	DEFINE_FIELD( m_nState, FIELD_INTEGER ),
	DEFINE_FIELD( m_iCaps, FIELD_INTEGER ),
	DEFINE_FIELD( m_flJuice, FIELD_FLOAT ),

	// Function Pointers
	DEFINE_FUNCTION( Off ),
	DEFINE_FUNCTION( Recharge ),

	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),
	DEFINE_OUTPUT( m_OutRemainingHealth, "OutRemainingHealth"),

END_DATADESC()

#define HEALTH_CHARGER_MODEL_NAME "models/props_combine/health_charger001.mdl"
#define CHARGE_RATE 0.25f
#define CHARGES_PER_SECOND 1.0f / CHARGE_RATE
#define CALLS_PER_SECOND 7.0f * CHARGES_PER_SECOND


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pkvd - 
//-----------------------------------------------------------------------------
bool CNewWallHealth::KeyValue(  const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "style") ||
		FStrEq(szKeyName, "height") ||
		FStrEq(szKeyName, "value1") ||
		FStrEq(szKeyName, "value2") ||
		FStrEq(szKeyName, "value3"))
	{
		return(true);
	}
	else if (FStrEq(szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(szValue);
		return(true);
	}

	return(BaseClass::KeyValue( szKeyName, szValue ));
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNewWallHealth::Spawn(void)
{
	Precache( );

	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	SetModel( HEALTH_CHARGER_MODEL_NAME );
	AddEffects( EF_NOSHADOW );

	ResetSequence( LookupSequence( "idle" ) );

	m_iJuice = sk_healthcharger.GetFloat();

	m_nState = 0;	
	
	m_iReactivate = 0;
	m_iCaps	= FCAP_CONTINUOUS_USE;

	CreateVPhysics();

	m_flJuice = m_iJuice;
	SetCycle( 1.0f - ( m_flJuice /  sk_healthcharger.GetFloat() ) );
}

int CNewWallHealth::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];
		Q_snprintf(tempstr,sizeof(tempstr),"Charge left: %i", m_iJuice );
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}

//-----------------------------------------------------------------------------

bool CNewWallHealth::CreateVPhysics(void)
{
	VPhysicsInitStatic();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNewWallHealth::Precache(void)
{
	PrecacheModel( HEALTH_CHARGER_MODEL_NAME );

	PrecacheScriptSound( "WallHealth.Deny" );
	PrecacheScriptSound( "WallHealth.Start" );
	PrecacheScriptSound( "WallHealth.LoopingContinueCharge" );
	PrecacheScriptSound( "WallHealth.Recharge" );
}

void CNewWallHealth::StudioFrameAdvance( void )
{
	m_flPlaybackRate = 0;

	float flMaxJuice = sk_healthcharger.GetFloat();
	
	SetCycle( 1.0f - (float)( m_flJuice / flMaxJuice ) );
//	Msg( "Cycle: %f - Juice: %d - m_flJuice :%f - Interval: %f\n", (float)GetCycle(), (int)m_iJuice, (float)m_flJuice, GetAnimTimeInterval() );

	if ( !m_flPrevAnimTime )
	{
		m_flPrevAnimTime = gpGlobals->curtime;
	}

	// Latch prev
	m_flPrevAnimTime = m_flAnimTime;
	// Set current
	m_flAnimTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CNewWallHealth::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	// Make sure that we have a caller
	if (!pActivator)
		return;

	// if it's not a player, ignore
	if ( !pActivator->IsPlayer() )
		return;
	CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(pActivator);

	// Reset to a state of continuous use.
	m_iCaps = FCAP_CONTINUOUS_USE;

	if ( m_iOn )
	{
		float flCharges = CHARGES_PER_SECOND;
		float flCalls = CALLS_PER_SECOND;

		m_flJuice -= flCharges / flCalls;
		StudioFrameAdvance();
	}

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		ResetSequence( LookupSequence( "emptyclick" ) );
		m_nState = 1;			
		Off();
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise.
	// disabled HEV suit dependency for now.
	//if ((m_iJuice <= 0) || (!(pActivator->m_bWearingSuit)))
	if (m_iJuice <= 0)
	{
		if (m_flSoundTime <= gpGlobals->curtime)
		{
			m_flSoundTime = gpGlobals->curtime + 0.62;
			EmitSound( "WallHealth.Deny" );
		}
		return;
	}

	if( pActivator->GetHealth() >= pActivator->GetMaxHealth() )
	{
		if( pPlayer )
		{
			pPlayer->m_afButtonPressed &= ~IN_USE;
		}

		// Make the user re-use me to get started drawing health.
		m_iCaps = FCAP_IMPULSE_USE;
		
		EmitSound( "WallHealth.Deny" );
		return;
	}

	SetNextThink( gpGlobals->curtime + CHARGE_RATE );
	SetThink( &CNewWallHealth::Off );

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->curtime)
		return;

	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		EmitSound( "WallHealth.Start" );
		m_flSoundTime = 0.56 + gpGlobals->curtime;

		m_OnPlayerUse.FireOutput( pActivator, this );
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->curtime))
	{
		m_iOn++;
		CPASAttenuationFilter filter( this, "WallHealth.LoopingContinueCharge" );
		filter.MakeReliable();
		EmitSound( filter, entindex(), "WallHealth.LoopingContinueCharge" );
	}

	// charge the player
	if ( pActivator->TakeHealth( 1, DMG_GENERIC ) )
	{
		m_iJuice--;
	}

	// Send the output.
	float flRemaining = m_iJuice / sk_healthcharger.GetFloat();
	m_OutRemainingHealth.Set(flRemaining, pActivator, this);

	// govern the rate of charge
	m_flNextCharge = gpGlobals->curtime + 0.1;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNewWallHealth::Recharge(void)
{
	EmitSound( "WallHealth.Recharge" );
	m_flJuice = m_iJuice = sk_healthcharger.GetFloat();
	m_nState = 0;

	ResetSequence( LookupSequence( "idle" ) );
	StudioFrameAdvance();

	m_iReactivate = 0;

	SetThink( NULL );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNewWallHealth::Off(void)
{
	// Stop looping sound.
	if (m_iOn > 1)
		StopSound( "WallHealth.LoopingContinueCharge" );

	if ( m_nState == 1 )
	{
		SetCycle( 1.0f );
	}

	m_iOn = 0;
	m_flJuice = m_iJuice;

	if ( m_iReactivate == 0 )
	{
		if ((!m_iJuice) && g_pGameRules->FlHealthChargerRechargeTime() > 0 )
		{
			m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime();
			SetNextThink( gpGlobals->curtime + m_iReactivate );
			SetThink(&CNewWallHealth::Recharge);
		}
		else
			SetThink( NULL );
	}
}

