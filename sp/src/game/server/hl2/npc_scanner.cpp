//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "soundenvelope.h"
#include "ai_hint.h"
#include "ai_moveprobe.h"
#include "ai_squad.h"
#include "beam_shared.h"
#include "globalstate.h"
#include "soundent.h"
#include "npc_citizen17.h"
#include "hlr/hlr_projectile.h"
#include "gib.h"
#include "spotlightend.h"
#include "IEffects.h"
#include "items.h"
#include "ai_route.h"
#include "weapon_rpg.h"
#include "player_pickup.h"
#include "weapon_physcannon.h"
#include "hl2_player.h"
#include "npc_scanner.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Singleton interfaces
//-----------------------------------------------------------------------------
extern IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;

//-----------------------------------------------------------------------------
// Parameters for how the scanner relates to citizens.
//-----------------------------------------------------------------------------
#define SCANNER_CIT_INSPECT_DELAY		10		// Check for citizens this often
#define	SCANNER_CIT_INSPECT_GROUND_DIST	500		// How far to look for citizens to inspect
#define	SCANNER_CIT_INSPECT_FLY_DIST	1500	// How far to look for citizens to inspect

#define SCANNER_CIT_INSPECT_LENGTH		5		// How long does the inspection last
#define SCANNER_HINT_INSPECT_LENGTH		5		// How long does the inspection last
#define SCANNER_SOUND_INSPECT_LENGTH	5		// How long does the inspection last

#define SCANNER_HINT_INSPECT_DELAY		15		// Check for hint nodes this often
	
#define	SPOTLIGHT_WIDTH					32

#define SCANNER_SPOTLIGHT_NEAR_DIST		64
#define SCANNER_SPOTLIGHT_FAR_DIST		256
#define SCANNER_SPOTLIGHT_FLY_HEIGHT	72
#define SCANNER_NOSPOTLIGHT_FLY_HEIGHT	72

#define SCANNER_FLASH_MIN_DIST			900		// How far does flash effect enemy
#define SCANNER_FLASH_MAX_DIST			1200	// How far does flash effect enemy

#define	SCANNER_FLASH_MAX_VALUE			240		// How bright is maximum flash

#define SCANNER_PHOTO_NEAR_DIST			256
#define SCANNER_PHOTO_FAR_DIST			768

#define	SCANNER_FOLLOW_DIST				128

#define	SCANNER_NUM_GIBS				6		// Number of gibs in gib file

#define	MIN_STALKER_FIRE_RANGE		64
#define	MAX_STALKER_FIRE_RANGE		3600 // 3600 feet.
#define	STALKER_LASER_ATTACHMENT	1
#define	STALKER_TRIGGER_DIST		200	// Enemy dist. that wakes up the stalker
#define	STALKER_SENTENCE_VOLUME		(float)0.35
#define STALKER_LASER_DURATION		99999
#define STALKER_LASER_RECHARGE		1
#define STALKER_PLAYER_AGGRESSION	1


// Strider Scout Scanners
#define SCANNER_SCOUT_MAX_SPEED			150

ConVar	sk_scanner_health( "sk_scanner_health","0");
ConVar	sk_scanner_projectile_speed("sk_scanner_projectile_speed", "600");
ConVar	g_debug_cscanner( "g_debug_cscanner", "0" );

//-----------------------------------------------------------------------------
// Private activities.
//-----------------------------------------------------------------------------
static int ACT_SCANNER_SMALL_FLINCH_ALERT = 0;
static int ACT_SCANNER_SMALL_FLINCH_COMBAT = 0;
static int ACT_SCANNER_INSPECT = 0;
static int ACT_SCANNER_WALK_ALERT = 0;
static int ACT_SCANNER_WALK_COMBAT = 0;
static int ACT_SCANNER_FLARE = 0;
static int ACT_SCANNER_RETRACT = 0;
static int ACT_SCANNER_FLARE_PRONGS = 0;
static int ACT_SCANNER_RETRACT_PRONGS = 0;
static int ACT_SCANNER_FLARE_START = 0;

//-----------------------------------------------------------------------------
// Interactions
//-----------------------------------------------------------------------------
int	g_interactionScannerInspect				= 0;
int	g_interactionScannerInspectBegin		= 0;
int g_interactionScannerInspectHandsUp		= 0;
int g_interactionScannerInspectShowArmband	= 0;//<<TEMP>>still to be completed
int	g_interactionScannerInspectDone			= 0;
int g_interactionScannerSupportEntity		= 0;
int g_interactionScannerSupportPosition		= 0;



enum StalkerBeamPower_e
{
	STALKER_BEAM_LOW,
	STALKER_BEAM_MED,
	STALKER_BEAM_HIGH,
};
//-----------------------------------------------------------------------------
// Animation events
//------------------------------------------------------------------------
int AE_SCANNER_CLOSED;

//-----------------------------------------------------------------------------
// Attachment points
//-----------------------------------------------------------------------------
#define SCANNER_ATTACHMENT_LIGHT	"light"
#define SCANNER_ATTACHMENT_FLASH	1
#define SCANNER_ATTACHMENT_LPRONG	2
#define SCANNER_ATTACHMENT_RPRONG	3

//-----------------------------------------------------------------------------
// Other defines.
//-----------------------------------------------------------------------------
#define SCANNER_MAX_BEAMS		4

BEGIN_DATADESC( CNPC_CScanner )

	DEFINE_SOUNDPATCH( m_pEngineSound ),

	DEFINE_EMBEDDED( m_KilledInfo ),
	DEFINE_FIELD( m_flGoalOverrideDistance,	FIELD_FLOAT ),
	DEFINE_FIELD( m_bPhotoTaken,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vInspectPos,			FIELD_VECTOR ),
	DEFINE_FIELD( m_fInspectEndTime,		FIELD_TIME ),
	DEFINE_FIELD( m_fCheckCitizenTime,		FIELD_TIME ),
	DEFINE_FIELD( m_fCheckHintTime,			FIELD_TIME ),
	DEFINE_KEYFIELD( m_bShouldInspect,		FIELD_BOOLEAN,	"ShouldInspect" ),
	DEFINE_KEYFIELD( m_bOnlyInspectPlayers, FIELD_BOOLEAN,  "OnlyInspectPlayers" ),
	DEFINE_KEYFIELD( m_bNeverInspectPlayers,FIELD_BOOLEAN,  "NeverInspectPlayers" ),
	DEFINE_FIELD( m_fNextPhotographTime,	FIELD_TIME ),
//	DEFINE_FIELD( m_pEyeFlash,				FIELD_CLASSPTR ),
	DEFINE_FIELD( m_vSpotlightTargetPos,	FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vSpotlightCurrentPos,	FIELD_POSITION_VECTOR ),
// don't save (recreated after restore/transition)
//	DEFINE_FIELD( m_hSpotlight,				FIELD_EHANDLE ),
//	DEFINE_FIELD( m_hSpotlightTarget,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_vSpotlightDir,			FIELD_VECTOR ),
	DEFINE_FIELD( m_vSpotlightAngVelocity,	FIELD_VECTOR ),
	DEFINE_FIELD( m_flSpotlightCurLength,	FIELD_FLOAT ),
	DEFINE_FIELD( m_fNextSpotlightTime,		FIELD_TIME ),
	DEFINE_FIELD( m_nHaloSprite,			FIELD_INTEGER ),
	DEFINE_FIELD( m_fNextFlySoundTime,		FIELD_TIME ),
	DEFINE_FIELD( m_nFlyMode,				FIELD_INTEGER ),
	DEFINE_FIELD( m_nPoseTail,				FIELD_INTEGER ),
	DEFINE_FIELD( m_nPoseDynamo,			FIELD_INTEGER ),
	DEFINE_FIELD( m_nPoseFlare,				FIELD_INTEGER ),
	DEFINE_FIELD( m_nPoseFaceVert,			FIELD_INTEGER ),
	DEFINE_FIELD( m_nPoseFaceHoriz,			FIELD_INTEGER ),

	DEFINE_FIELD( m_bIsClawScanner,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIsOpen,				FIELD_BOOLEAN ),

	// DEFINE_FIELD( m_bHasSpoken,			FIELD_BOOLEAN ),

	DEFINE_FIELD( m_pSmokeTrail,			FIELD_CLASSPTR ),
	DEFINE_FIELD( m_flFlyNoiseBase,			FIELD_FLOAT ),
	DEFINE_FIELD( m_flEngineStallTime,		FIELD_TIME ),

	DEFINE_FIELD( m_vecDiveBombDirection,	FIELD_VECTOR ),
	DEFINE_FIELD( m_flDiveBombRollForce,	FIELD_FLOAT ),

	DEFINE_KEYFIELD( m_flSpotlightMaxLength,	FIELD_FLOAT,	"SpotlightLength"),
	DEFINE_KEYFIELD( m_flSpotlightGoalWidth,	FIELD_FLOAT,	"SpotlightWidth"),

	// Physics Influence
	DEFINE_FIELD( m_hPhysicsAttacker, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flLastPhysicsInfluenceTime, FIELD_TIME ),

	DEFINE_KEYFIELD( m_bNoLight, FIELD_BOOLEAN, "SpotlightDisabled" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "DisableSpotlight", InputDisableSpotlight ),
	DEFINE_INPUTFUNC( FIELD_STRING, "InspectTargetPhoto", InputInspectTargetPhoto ),
	DEFINE_INPUTFUNC( FIELD_STRING, "InspectTargetSpotlight", InputInspectTargetSpotlight ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "InputShouldInspect", InputShouldInspect ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetFollowTarget", InputSetFollowTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ClearFollowTarget", InputClearFollowTarget ),

	DEFINE_INPUTFUNC( FIELD_STRING, "DeployMine", InputDeployMine ),
	DEFINE_INPUTFUNC( FIELD_STRING, "EquipMine", InputEquipMine ),

	DEFINE_OUTPUT( m_OnPhotographPlayer, "OnPhotographPlayer" ),
	DEFINE_OUTPUT( m_OnPhotographNPC, "OnPhotographNPC" ),







	DEFINE_THINKFUNC(BeamThink),
	DEFINE_KEYFIELD(m_eBeamPower, FIELD_INTEGER, "BeamPower"),
	DEFINE_FIELD(m_vLaserDir, FIELD_VECTOR),
	DEFINE_FIELD(m_vLaserTargetPos, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(m_fBeamEndTime, FIELD_FLOAT),
	DEFINE_FIELD(m_fBeamRechargeTime, FIELD_FLOAT),
	DEFINE_FIELD(m_fNextDamageTime, FIELD_FLOAT),
	DEFINE_FIELD(m_bPlayingHitWall, FIELD_FLOAT),
	DEFINE_FIELD(m_bPlayingHitFlesh, FIELD_FLOAT),
	DEFINE_FIELD(m_pBeam, FIELD_CLASSPTR),
	DEFINE_FIELD(m_pLightGlow, FIELD_CLASSPTR),
	DEFINE_FIELD(m_flNextNPCThink, FIELD_FLOAT),
	DEFINE_FIELD(m_vLaserCurPos, FIELD_POSITION_VECTOR),

END_DATADESC()


LINK_ENTITY_TO_CLASS(npc_cscanner, CNPC_CScanner);

float g_StalkerBeamThinkTime = 0.0; //0.025;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_CScanner::CNPC_CScanner()
{
#ifdef _DEBUG
	m_vInspectPos.Init();
	m_vSpotlightTargetPos.Init();
	m_vSpotlightCurrentPos.Init();
	m_vSpotlightDir.Init();
	m_vSpotlightAngVelocity.Init();
#endif
	m_bShouldInspect = true;
	m_bOnlyInspectPlayers = false;
	m_bNeverInspectPlayers = false;

	char szMapName[256];
	Q_strncpy(szMapName, STRING(gpGlobals->mapname), sizeof(szMapName) );
	Q_strlower(szMapName);

	if( !Q_strnicmp( szMapName, "d3_c17", 6 ) )
	{
		// Streetwar scanners are claw scanners
		m_bIsClawScanner = true;
	}
	else
	{
		m_bIsClawScanner = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CScanner::Spawn(void)
{
	// Check for user error
	if (m_flSpotlightMaxLength <= 0)
	{
		DevMsg("CNPC_CScanner::Spawn: Invalid spotlight length <= 0, setting to 500\n");
		m_flSpotlightMaxLength = 500;
	}
	
	if (m_flSpotlightGoalWidth <= 0)
	{
		DevMsg("CNPC_CScanner::Spawn: Invalid spotlight width <= 0, setting to 100\n");
		m_flSpotlightGoalWidth = 100;
	}

	if (m_flSpotlightGoalWidth > MAX_BEAM_WIDTH )
	{
		DevMsg("CNPC_CScanner::Spawn: Invalid spotlight width %.1f (max %.1f).\n", m_flSpotlightGoalWidth, MAX_BEAM_WIDTH );
		m_flSpotlightGoalWidth = MAX_BEAM_WIDTH; 
	}

	Precache();

	if( m_bIsClawScanner )
	{
		SetModel( "models/shield_scanner.mdl");
	}
	else
	{
		SetModel( "models/combine_scanner.mdl");
	}

	m_iHealth				= sk_scanner_health.GetFloat();
	m_iMaxHealth = m_iHealth;

	// ------------------------------------
	//	Init all class vars 
	// ------------------------------------
	m_vInspectPos			= vec3_origin;
	m_fInspectEndTime		= 0;
	m_fCheckCitizenTime		= gpGlobals->curtime + SCANNER_CIT_INSPECT_DELAY;
	m_fCheckHintTime		= gpGlobals->curtime + SCANNER_HINT_INSPECT_DELAY;
	m_fNextPhotographTime	= 0;

	m_vSpotlightTargetPos	= vec3_origin;
	m_vSpotlightCurrentPos	= vec3_origin;

	m_hSpotlight			= NULL;
	m_hSpotlightTarget		= NULL;
	m_flFieldOfView = 0.1;
	AngleVectors( GetLocalAngles(), &m_vSpotlightDir );
	m_vSpotlightAngVelocity = vec3_origin;

	m_pEyeFlash				= 0;
	m_fNextSpotlightTime	= 0;
	m_nFlyMode				= SCANNER_FLY_PATROL;
	m_vCurrentBanking		= m_vSpotlightDir;
	m_flSpotlightCurLength	= m_flSpotlightMaxLength;

	m_nPoseTail = LookupPoseParameter( "tail_control" );
	m_nPoseDynamo = LookupPoseParameter( "dynamo_wheel" );
	m_nPoseFlare = LookupPoseParameter( "alert_control" );
	m_nPoseFaceVert = LookupPoseParameter( "flex_vert" );
	m_nPoseFaceHoriz = LookupPoseParameter( "flex_horz" );

	// --------------------------------------------
	m_flDistTooFar = MAX_STALKER_FIRE_RANGE;
	CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 );
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);

	m_bPhotoTaken = false;

	BaseClass::Spawn();

	// Watch for this error state
	if ( m_bOnlyInspectPlayers && m_bNeverInspectPlayers )
	{
		Assert( 0 );
		Warning( "ERROR: Scanner set to never and always inspect players!\n" );
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CNPC_CScanner::Activate()
{
	BaseClass::Activate();

	// Have to do this here because sprites do not go across level transitions
	m_pEyeFlash = CSprite::SpriteCreate( "sprites/blueflare1.vmt", GetLocalOrigin(), FALSE );
	m_pEyeFlash->SetTransparency( kRenderGlow, 255, 255, 255, 0, kRenderFxNoDissipation );
	m_pEyeFlash->SetAttachment( this, LookupAttachment( SCANNER_ATTACHMENT_LIGHT ) );
	m_pEyeFlash->SetBrightness( 0 );
	m_pEyeFlash->SetScale( 1.4 );
}

//------------------------------------------------------------------------------
// Purpose: Override to split in two when attacked
//------------------------------------------------------------------------------
int CNPC_CScanner::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// Turn off my spotlight when shot
	SpotlightDestroy();
	m_fNextSpotlightTime = gpGlobals->curtime + 2.0f;

	return (BaseClass::OnTakeDamage_Alive( info ));
}
	
//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::Gib( void )
{
	if ( IsMarkedForDeletion() )
		return;

	// Spawn all gibs
	if( m_bIsClawScanner )
	{
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib1.mdl");
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib2.mdl");
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib3.mdl");
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib4.mdl");
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib5.mdl");
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib6.mdl");
	}
	else
	{
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/scanner_gib01.mdl" );
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/scanner_gib02.mdl" );
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/scanner_gib04.mdl" );
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/scanner_gib05.mdl" );
	}

	// Add a random chance of spawning a battery...
	DeployMine();

	BaseClass::Gib();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::Event_Killed( const CTakeDamageInfo &info )
{
	// Copy off the takedamage info that killed me, since we're not going to call
	// up into the base class's Event_Killed() until we gib. (gibbing is ultimate death)
	m_KilledInfo = info;	

	DeployMine();

	ClearInspectTarget();

	// Interrupt whatever schedule I'm on
	SetCondition(COND_SCHEDULE_DONE);

	// Remove spotlight
	SpotlightDestroy();

	// Remove sprite
	UTIL_Remove(m_pEyeFlash);
	m_pEyeFlash = NULL;

	// If I have an enemy and I'm up high, do a dive bomb (unless dissolved)
	if ( !m_bIsClawScanner && GetEnemy() != NULL && (info.GetDamageType() & DMG_DISSOLVE) == false )
	{
		Vector vecDelta = GetLocalOrigin() - GetEnemy()->GetLocalOrigin();
		if ( ( vecDelta.z > 120 ) && ( vecDelta.Length() > 360 ) )
		{	
			// If I'm divebombing, don't take any more damage. It will make Event_Killed() be called again.
			// This is especially bad if someone machineguns the divebombing scanner. 
			AttackDivebomb();
			return;
		}
	}
	KillAttackBeam();

	Gib();
}


//-----------------------------------------------------------------------------
// Purpose: Tells use whether or not the NPC cares about a given type of hint node.
// Input  : sHint - 
// Output : TRUE if the NPC is interested in this hint type, FALSE if not.
//-----------------------------------------------------------------------------
bool CNPC_CScanner::FValidateHintType(CAI_Hint *pHint)
{
	return( pHint->HintType() == HINT_WORLD_WINDOW );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : Type - 
//-----------------------------------------------------------------------------
int CNPC_CScanner::TranslateSchedule( int scheduleType ) 
{
	switch ( scheduleType )
	{
		case SCHED_IDLE_STAND:
		{
			return SCHED_SCANNER_PATROL;
		}

		case SCHED_SCANNER_PATROL:
			return SCHED_CSCANNER_PATROL;
	}
	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : idealActivity - 
//			*pIdealWeaponActivity - 
// Output : int
//-----------------------------------------------------------------------------
Activity CNPC_CScanner::NPC_TranslateActivity( Activity eNewActivity )
{
	if( !m_bIsClawScanner )
	{
		return BaseClass::NPC_TranslateActivity( eNewActivity );
	}

	// The claw scanner came along a little late and doesn't have the activities
	// of the city scanner. So Just pick between these three
	if( eNewActivity == ACT_DISARM )
	{
		// Closing up.
		return eNewActivity;
	}

	if( m_bIsOpen )
	{
		return ACT_IDLE_ANGRY;
	}
	else
	{
		return ACT_IDLE;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_CScanner::HandleAnimEvent( animevent_t *pEvent )
{
	if( pEvent->event == AE_SCANNER_CLOSED )
	{
		m_bIsOpen = false;
		SetActivity( ACT_IDLE );
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
char *CNPC_CScanner::GetEngineSound( void )
{
	if( m_bIsClawScanner )
		return "NPC_SScanner.FlyLoop";

	return "NPC_CScanner.FlyLoop";
}

//-----------------------------------------------------------------------------
// Purpose: Plays the engine sound.
//-----------------------------------------------------------------------------
void CNPC_CScanner::NPCThink(void)
{
	if (!IsAlive())
	{
		SetActivity((Activity)ACT_SCANNER_RETRACT_PRONGS);
		StudioFrameAdvance( );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	else
	{
		BaseClass::NPCThink();
		SpotlightUpdate();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CScanner::Precache(void)
{
	// Model
	if( m_bIsClawScanner )
	{
		PrecacheModel("models/shield_scanner.mdl");

		PrecacheModel("models/gibs/Shield_Scanner_Gib1.mdl");
		PrecacheModel("models/gibs/Shield_Scanner_Gib2.mdl");
		PrecacheModel("models/gibs/Shield_Scanner_Gib3.mdl");
		PrecacheModel("models/gibs/Shield_Scanner_Gib4.mdl");
		PrecacheModel("models/gibs/Shield_Scanner_Gib5.mdl");
		PrecacheModel("models/gibs/Shield_Scanner_Gib6.mdl");

		PrecacheScriptSound( "NPC_SScanner.Shoot");
		PrecacheScriptSound( "NPC_SScanner.Alert" );
		PrecacheScriptSound( "NPC_SScanner.Die" );
		PrecacheScriptSound( "NPC_SScanner.Combat" );
		PrecacheScriptSound( "NPC_SScanner.Idle" );
		PrecacheScriptSound( "NPC_SScanner.Pain" );
		PrecacheScriptSound( "NPC_SScanner.TakePhoto" );
		PrecacheScriptSound( "NPC_SScanner.AttackFlash" );
		PrecacheScriptSound( "NPC_SScanner.DiveBombFlyby" );
		PrecacheScriptSound( "NPC_SScanner.DiveBomb" );
		PrecacheScriptSound( "NPC_SScanner.DeployMine" );

		PrecacheScriptSound( "NPC_SScanner.FlyLoop" );
		UTIL_PrecacheOther( "combine_mine" );
	}
	else
	{
		PrecacheModel("models/combine_scanner.mdl");

		PrecacheModel("models/gibs/scanner_gib01.mdl" );
		PrecacheModel("models/gibs/scanner_gib02.mdl" );	
		PrecacheModel("models/gibs/scanner_gib02.mdl" );
		PrecacheModel("models/gibs/scanner_gib04.mdl" );
		PrecacheModel("models/gibs/scanner_gib05.mdl" );

		PrecacheScriptSound( "NPC_CScanner.Shoot");
		PrecacheScriptSound( "NPC_CScanner.Alert" );
		PrecacheScriptSound( "NPC_CScanner.Die" );
		PrecacheScriptSound( "NPC_CScanner.Combat" );
		PrecacheScriptSound( "NPC_CScanner.Idle" );
		PrecacheScriptSound( "NPC_CScanner.Pain" );
		PrecacheScriptSound( "NPC_CScanner.TakePhoto" );
		PrecacheScriptSound( "NPC_CScanner.AttackFlash" );
		PrecacheScriptSound( "NPC_CScanner.DiveBombFlyby" );
		PrecacheScriptSound( "NPC_CScanner.DiveBomb" );
		PrecacheScriptSound( "NPC_CScanner.DeployMine" );

		PrecacheScriptSound( "NPC_CScanner.FlyLoop" );
	}

	// Sprites
	m_nHaloSprite = PrecacheModel("sprites/light_glow03.vmt");
	PrecacheModel( "sprites/glow_test02.vmt" );
	UTIL_PrecacheOther("item_ammo_smg1");
	UTIL_PrecacheOther("item_ammo_ar2_large");
	UTIL_PrecacheOther("item_rpg_round");
	UTIL_PrecacheOther("item_box_buckshot");

	PrecacheModel("sprites/laser.vmt");

	PrecacheModel("sprites/redglow1.vmt");
	PrecacheModel("sprites/orangeglow1.vmt");
	PrecacheModel("sprites/yellowglow1.vmt");

	PrecacheScriptSound("NPC_Stalker.BurnFlesh");
	PrecacheScriptSound("NPC_Stalker.BurnWall");
	PrecacheScriptSound("NPC_Stalker.Hit");

	

	BaseClass::Precache();
}

//------------------------------------------------------------------------------
// Purpose: Request help inspecting from other squad members
//------------------------------------------------------------------------------
void CNPC_CScanner::RequestInspectSupport(void)
{
	if (m_pSquad)
	{
		AISquadIter_t iter;
		for (CAI_BaseNPC *pSquadMember = m_pSquad->GetFirstMember( &iter ); pSquadMember; pSquadMember = m_pSquad->GetNextMember( &iter ) )
		{
			if (pSquadMember != this)
			{
				if (GetTarget())
				{
					pSquadMember->DispatchInteraction(g_interactionScannerSupportEntity,((void *)((CBaseEntity*)GetTarget())),this);
				}
				else
				{
					pSquadMember->DispatchInteraction(g_interactionScannerSupportPosition,((void *)m_vInspectPos.Base()),this);
				}
			}
		}
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CNPC_CScanner::IsValidInspectTarget(CBaseEntity *pEntity)
{
	// If a citizen, make sure he can be inspected again
	if (pEntity->Classify() == CLASS_CITIZEN_PASSIVE)
	{
		if (((CNPC_Citizen*)pEntity)->GetNextScannerInspectTime() > gpGlobals->curtime)
		{
			return false;
		}
	}

	// Make sure no other squad member has already chosen to 
	// inspect this entity
	if (m_pSquad)
	{
		AISquadIter_t iter;
		for (CAI_BaseNPC *pSquadMember = m_pSquad->GetFirstMember( &iter ); pSquadMember; pSquadMember = m_pSquad->GetNextMember( &iter ) )
		{
			if (pSquadMember->GetTarget() == pEntity)
			{
				return false;
			}
		}
	}

	// Do not inspect friendly targets
	if ( IRelationType( pEntity ) == D_LI )
		return false;

	return true;
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
CBaseEntity* CNPC_CScanner::BestInspectTarget(void)
{
	if ( !m_bShouldInspect )
		return NULL;

	CBaseEntity*	pBestEntity = NULL;
	float			fBestDist	= MAX_COORD_RANGE;
	float			fTestDist;

	CBaseEntity *pEntity = NULL;

	// If I have a spotlight, search from the spotlight position
	// otherwise search from my position
	Vector	vSearchOrigin;
	float	fSearchDist;
	if (m_hSpotlightTarget != NULL)
	{
		vSearchOrigin	= m_hSpotlightTarget->GetAbsOrigin();
		fSearchDist		= SCANNER_CIT_INSPECT_GROUND_DIST;
	}
	else
	{
		vSearchOrigin	= WorldSpaceCenter();
		fSearchDist		= SCANNER_CIT_INSPECT_FLY_DIST;
	}

	if ( m_bOnlyInspectPlayers )
	{
		CBasePlayer *pPlayer = AI_GetSinglePlayer();
		if ( !pPlayer )
			return NULL;

		if ( !pPlayer->IsAlive() || (pPlayer->GetFlags() & FL_NOTARGET) )
			return NULL;

		return WorldSpaceCenter().DistToSqr( pPlayer->EyePosition() ) <= (fSearchDist * fSearchDist) ? pPlayer : NULL;
	}

	CUtlVector<CBaseEntity *> candidates;
	float fSearchDistSq = fSearchDist * fSearchDist;
	int i;

	// Inspect players unless told otherwise
	if ( m_bNeverInspectPlayers == false )
	{
		// Players
		for ( i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

			if ( pPlayer )
			{
				if ( vSearchOrigin.DistToSqr(pPlayer->GetAbsOrigin()) < fSearchDistSq )
				{
					candidates.AddToTail( pPlayer );
				}
			}
		}
	}
	
	// NPCs
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	
	for ( i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		if ( ppAIs[i] != this && vSearchOrigin.DistToSqr(ppAIs[i]->GetAbsOrigin()) < fSearchDistSq )
		{
			candidates.AddToTail( ppAIs[i] );
		}
	}

	for ( i = 0; i < candidates.Count(); i++ )
	{
		pEntity = candidates[i];
		Assert( pEntity != this && (pEntity->MyNPCPointer() || pEntity->IsPlayer() ) );

		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
		if ( ( pNPC && pNPC->Classify() == CLASS_CITIZEN_PASSIVE ) || pEntity->IsPlayer() )
		{
			if ( pEntity->GetFlags() & FL_NOTARGET )
				continue;

			if ( pEntity->IsAlive() == false )
				continue;

			// Ensure it's within line of sight
			if ( !FVisible( pEntity ) )
				continue;

			fTestDist = ( GetAbsOrigin() - pEntity->EyePosition() ).Length();
			if ( fTestDist < fBestDist )
			{
				if ( IsValidInspectTarget( pEntity ) )
				{
					fBestDist	= fTestDist;
					pBestEntity	= pEntity; 
				}
			}
		}
	}
	return pBestEntity;
}


//------------------------------------------------------------------------------
// Purpose: Clears any previous inspect target and set inspect target to
//			 the given entity and set the durection of the inspection
//------------------------------------------------------------------------------
void CNPC_CScanner::SetInspectTargetToEnt(CBaseEntity *pEntity, float fInspectDuration)
{
	ClearInspectTarget();
	SetTarget(pEntity);
	
	m_fInspectEndTime = gpGlobals->curtime + fInspectDuration;
}


//------------------------------------------------------------------------------
// Purpose: Clears any previous inspect target and set inspect target to
//			 the given hint node and set the durection of the inspection
//------------------------------------------------------------------------------
void CNPC_CScanner::SetInspectTargetToHint(CAI_Hint *pHint, float fInspectDuration)
{
	ClearInspectTarget();

	float yaw = pHint->Yaw();
	// --------------------------------------------
	// Figure out the location that the hint hits
	// --------------------------------------------
	Vector vHintDir	= UTIL_YawToVector( yaw );

	Vector vHintOrigin;
	pHint->GetPosition( this, &vHintOrigin );

	Vector vHintEnd	= vHintOrigin + (vHintDir * 512);
	
	trace_t tr;
	AI_TraceLine ( vHintOrigin, vHintEnd, MASK_BLOCKLOS, this, COLLISION_GROUP_NONE, &tr);
	
	if ( g_debug_cscanner.GetBool() )
	{
		NDebugOverlay::Line( vHintOrigin, tr.endpos, 255, 0, 0, true, 4.0f );
		NDebugOverlay::Cross3D( tr.endpos, -Vector(8,8,8), Vector(8,8,8), 255, 0, 0, true, 4.0f );
	}

	if (tr.fraction == 1.0f )
	{
		DevMsg("ERROR: Scanner hint node not facing a surface!\n");
	}
	else
	{
		SetHintNode( pHint );
		m_vInspectPos = tr.endpos;
		pHint->Lock( this );

		m_fInspectEndTime = gpGlobals->curtime + fInspectDuration;
	}
}


//------------------------------------------------------------------------------
// Purpose: Clears any previous inspect target and set inspect target to
//			 the given position and set the durection of the inspection
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_CScanner::SetInspectTargetToPos(const Vector &vInspectPos, float fInspectDuration)
{
	ClearInspectTarget();
	m_vInspectPos		= vInspectPos;

	m_fInspectEndTime	= gpGlobals->curtime + fInspectDuration;
}


//------------------------------------------------------------------------------
// Purpose: Clears out any previous inspection targets
//------------------------------------------------------------------------------
void CNPC_CScanner::ClearInspectTarget(void)
{
	if ( GetIdealState() != NPC_STATE_SCRIPT )
	{
		SetTarget( NULL );
	}

	ClearHintNode( SCANNER_HINT_INSPECT_LENGTH );
	m_vInspectPos	= vec3_origin;
}


//------------------------------------------------------------------------------
// Purpose: Returns true if there is a position to be inspected.
//------------------------------------------------------------------------------
bool CNPC_CScanner::HaveInspectTarget( void )
{
	if ( GetTarget() != NULL )
		return true;

	if ( m_vInspectPos != vec3_origin )
		return true;

	return false;
}


//------------------------------------------------------------------------------
// Purpose: 
//------------------------------------------------------------------------------
Vector CNPC_CScanner::InspectTargetPosition(void)
{
	// If we have a target, return an adjust position
	if ( GetTarget() != NULL )
	{
		Vector	vEyePos = GetTarget()->EyePosition();

		// If in spotlight mode, aim for ground below target unless is client
		if ( m_nFlyMode == SCANNER_FLY_SPOT && !(GetTarget()->GetFlags() & FL_CLIENT) )
		{
			Vector vInspectPos;
			vInspectPos.x	= vEyePos.x;
			vInspectPos.y	= vEyePos.y;
			vInspectPos.z	= GetFloorZ( vEyePos );

			// Let's take three-quarters between eyes and ground
			vInspectPos.z	+= ( vEyePos.z - vInspectPos.z ) * 0.75f;

			return vInspectPos;
		}
		else
		{
			// Otherwise aim for eyes
			return vEyePos;
		}
	}
	else if ( m_vInspectPos != vec3_origin )
	{
		return m_vInspectPos;
	}
	else
	{
		DevMsg("InspectTargetPosition called with no target!\n");
		
		return m_vInspectPos;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputShouldInspect( inputdata_t &inputdata )
{
	m_bShouldInspect = ( inputdata.value.Int() != 0 );
	
	if ( !m_bShouldInspect )
	{
		if ( GetEnemy() == GetTarget() )
			SetEnemy(NULL);
		ClearInspectTarget();
		SetTarget(NULL);
		SpotlightDestroy();
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_CScanner::DeployMine()
{
	CBaseEntity *child;
	// iterate through all children
	for ( child = FirstMoveChild(); child != NULL; child = child->NextMovePeer() )
	{
		if( FClassnameIs( child, "combine_mine" ) )
		{
			child->SetParent( NULL );
			child->SetAbsVelocity( GetAbsVelocity() );
			child->SetOwnerEntity( this );

			ScannerEmitSound( "DeployMine" );

			IPhysicsObject *pPhysObj = child->VPhysicsGetObject();
			if( pPhysObj )
			{
				// Make sure the mine's awake
				pPhysObj->Wake();
			}

			if( m_bIsClawScanner )
			{
				// Fold up.
				SetActivity( ACT_DISARM );
			}

			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CNPC_CScanner::GetMaxSpeed()
{
	if( IsStriderScout() )
	{
		return SCANNER_SCOUT_MAX_SPEED;
	}

	return BaseClass::GetMaxSpeed();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputDeployMine(inputdata_t &inputdata)
{
	DeployMine();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputEquipMine(inputdata_t &inputdata)
{
	CBaseEntity *child;
	// iterate through all children
	for ( child = FirstMoveChild(); child != NULL; child = child->NextMovePeer() )
	{
		if( FClassnameIs( child, "combine_mine" ) )
		{
			// Already have a mine!
			return;
		}
	}

	CBaseEntity *pEnt;

	pEnt = CreateEntityByName( "combine_mine" );
	bool bPlacedMine = false;

	if( m_bIsClawScanner )
	{
		Vector	vecOrigin;
		QAngle	angles;
		int		attachment;

		attachment = LookupAttachment( "claw" );

		if( attachment > -1 )
		{
			GetAttachment( attachment, vecOrigin, angles );
			
			pEnt->SetAbsOrigin( vecOrigin );
			pEnt->SetAbsAngles( angles );
			pEnt->SetOwnerEntity( this );
			pEnt->SetParent( this, attachment );

			m_bIsOpen = true;
			SetActivity( ACT_IDLE_ANGRY );
			bPlacedMine = true;
		}
	}


	if( !bPlacedMine )
	{
		Vector vecMineLocation = GetAbsOrigin();
		vecMineLocation.z -= 32.0;

		pEnt->SetAbsOrigin( vecMineLocation );
		pEnt->SetAbsAngles( GetAbsAngles() );
		pEnt->SetOwnerEntity( this );
		pEnt->SetParent( this );
	}

	pEnt->Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: Tells the scanner to go photograph an entity.
// Input  : String name or classname of the entity to inspect.
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputInspectTargetPhoto(inputdata_t &inputdata)
{
	m_vLastPatrolDir = vec3_origin;
	m_bPhotoTaken = false;
	InspectTarget( inputdata, SCANNER_FLY_PHOTO );
}


//-----------------------------------------------------------------------------
// Purpose: Tells the scanner to go spotlight an entity.
// Input  : String name or classname of the entity to inspect.
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputInspectTargetSpotlight(inputdata_t &inputdata)
{
	InspectTarget( inputdata, SCANNER_FLY_SPOT );
}


//-----------------------------------------------------------------------------
// Purpose: Tells the scanner to go photo or spotlight an entity.
// Input  : String name or classname of the entity to inspect.
//-----------------------------------------------------------------------------
void CNPC_CScanner::InspectTarget( inputdata_t &inputdata, ScannerFlyMode_t eFlyMode )
{
	CBaseEntity *pEnt = gEntList.FindEntityGeneric( NULL, inputdata.value.String(), this, inputdata.pActivator );
	
	if ( pEnt != NULL )
	{
		// Set and begin to inspect our target
		SetInspectTargetToEnt( pEnt, SCANNER_CIT_INSPECT_LENGTH );
		
		m_nFlyMode = eFlyMode;
		SetCondition( COND_CSCANNER_HAVE_INSPECT_TARGET );
		
		// Stop us from any other navigation we were doing
		GetNavigator()->ClearGoal();
	}
	else
	{
		DevMsg( "InspectTarget: target %s not found!\n", inputdata.value.String() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_CScanner::MovingToInspectTarget( void )
{
	// If we're flying to a photograph target and the photo isn't yet taken, we're still moving to it
	if ( m_nFlyMode == SCANNER_FLY_PHOTO && m_bPhotoTaken == false )
		return true;

	// If we're still on a path, then we're still moving
	if ( HaveInspectTarget() && GetNavigator()->IsGoalActive() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CScanner::GatherConditions( void )
{
	BaseClass::GatherConditions();

	// Clear out our old conditions
	ClearCondition( COND_CSCANNER_INSPECT_DONE );
	ClearCondition( COND_CSCANNER_HAVE_INSPECT_TARGET );
	ClearCondition( COND_CSCANNER_SPOT_ON_TARGET );
	ClearCondition( COND_CSCANNER_CAN_PHOTOGRAPH );

	// We don't do any of these checks if we have an enemy
	if ( GetEnemy() )
		return;

	// --------------------------------------
	//  COND_CSCANNER_INSPECT_DONE
	//
	// If my inspection over 
	// ---------------------------------------------------------

	// Refresh our timing if we're still moving to our inspection target
	if ( MovingToInspectTarget() )
	{
		m_fInspectEndTime = gpGlobals->curtime + SCANNER_CIT_INSPECT_LENGTH;
	}

	// Update our follow times
	if ( HaveInspectTarget() && gpGlobals->curtime > m_fInspectEndTime && m_nFlyMode != SCANNER_FLY_FOLLOW )
	{
		SetCondition ( COND_CSCANNER_INSPECT_DONE );

		m_fCheckCitizenTime	= gpGlobals->curtime + SCANNER_CIT_INSPECT_DELAY;
		m_fCheckHintTime	= gpGlobals->curtime + SCANNER_HINT_INSPECT_DELAY;
		ClearInspectTarget();
	}

	// ----------------------------------------------------------
	//  If I heard a sound and I don't have an enemy, inspect it
	// ----------------------------------------------------------
	if ( ( HasCondition( COND_HEAR_COMBAT ) || HasCondition( COND_HEAR_DANGER ) ) && m_nFlyMode != SCANNER_FLY_FOLLOW )
	{
		CSound *pSound = GetBestSound();
		
		if ( pSound )
		{
			// Chase an owner if we can
			if ( pSound->m_hOwner != NULL )
			{
				// Don't inspect sounds of things we like
				if ( IRelationType( pSound->m_hOwner ) != D_LI )
				{
					// Only bother if we can see it
					if ( FVisible( pSound->m_hOwner ) )
					{
						SetInspectTargetToEnt( pSound->m_hOwner, SCANNER_SOUND_INSPECT_LENGTH );
					}
				}
			}
			else
			{
				// Otherwise chase the specific sound
				Vector vSoundPos = pSound->GetSoundOrigin();
				SetInspectTargetToPos( vSoundPos, SCANNER_SOUND_INSPECT_LENGTH );
			}

			m_nFlyMode = (random->RandomInt(0,2)==0) ? SCANNER_FLY_SPOT : SCANNER_FLY_PHOTO;
		}
	}

	// --------------------------------------
	//  COND_CSCANNER_HAVE_INSPECT_TARGET
	//
	// Look for a nearby citizen or player to hassle. 
	// ---------------------------------------------------------

	// Check for citizens to inspect
	if ( gpGlobals->curtime	> m_fCheckCitizenTime && HaveInspectTarget() == false )
	{
		CBaseEntity *pBestEntity = BestInspectTarget();
		
		if ( pBestEntity != NULL )
		{
			SetInspectTargetToEnt( pBestEntity, SCANNER_CIT_INSPECT_LENGTH );
			m_nFlyMode = (random->RandomInt(0,3)==0) ? SCANNER_FLY_SPOT : SCANNER_FLY_PHOTO;
			SetCondition ( COND_CSCANNER_HAVE_INSPECT_TARGET );
		}
	}

	// Check for hints to inspect
	if ( gpGlobals->curtime > m_fCheckHintTime && HaveInspectTarget() == false )
	{
		SetHintNode( CAI_HintManager::FindHint( this, HINT_WORLD_WINDOW, 0, SCANNER_CIT_INSPECT_FLY_DIST ) );

		if ( GetHintNode() )
		{
			m_fCheckHintTime = gpGlobals->curtime + SCANNER_HINT_INSPECT_DELAY;

			m_nFlyMode = (random->RandomInt(0,2)==0) ? SCANNER_FLY_SPOT : SCANNER_FLY_PHOTO;

			SetInspectTargetToHint( GetHintNode(), SCANNER_HINT_INSPECT_LENGTH );

			SetCondition ( COND_CSCANNER_HAVE_INSPECT_TARGET );
		}
	}

	// --------------------------------------
	//  COND_CSCANNER_SPOT_ON_TARGET
	//
	//  True when spotlight is on target ent
	// --------------------------------------

	if ( m_hSpotlightTarget != NULL	&& HaveInspectTarget() && m_hSpotlightTarget->GetSmoothedVelocity().Length() < 25 )
	{
		// If I have a target entity, check my spotlight against the
		// actual position of the entity
		if (GetTarget())
		{
			float fInspectDist = (m_vSpotlightTargetPos - m_vSpotlightCurrentPos).Length();
			if ( fInspectDist < 100 )
			{
				SetCondition( COND_CSCANNER_SPOT_ON_TARGET );
			}
		}
		// Otherwise just check by beam direction
		else
		{
			Vector vTargetDir = SpotlightTargetPos() - GetLocalOrigin();
			VectorNormalize(vTargetDir);
			float dotpr = DotProduct(vTargetDir, m_vSpotlightDir);
			if (dotpr > 0.95)
			{
				SetCondition( COND_CSCANNER_SPOT_ON_TARGET );
			}
		}
	}

	// --------------------------------------------
	//  COND_CSCANNER_CAN_PHOTOGRAPH
	//
	//  True when can photograph target ent
	// --------------------------------------------

	ClearCondition( COND_CSCANNER_CAN_PHOTOGRAPH );

	if ( m_nFlyMode == SCANNER_FLY_PHOTO )
	{
		// Make sure I have something to photograph and I'm ready to photograph and I'm not moving to fast
		if ( gpGlobals->curtime > m_fNextPhotographTime && HaveInspectTarget() && GetCurrentVelocity().LengthSqr() < (64*64) )
		{
			// Check that I'm in the right distance range
			float  fInspectDist = (InspectTargetPosition() - GetAbsOrigin()).Length2D();
			
			// See if we're within range
			if ( fInspectDist > SCANNER_PHOTO_NEAR_DIST && fInspectDist < SCANNER_PHOTO_FAR_DIST )
			{
				// Make sure we're looking at the target
				if ( UTIL_AngleDiff( GetAbsAngles().y, VecToYaw( InspectTargetPosition() - GetAbsOrigin() ) ) < 4.0f )
				{
					trace_t tr;
					AI_TraceLine ( GetAbsOrigin(), InspectTargetPosition(), MASK_BLOCKLOS, GetTarget(), COLLISION_GROUP_NONE, &tr);
					
					if ( tr.fraction == 1.0f )
					{
						SetCondition( COND_CSCANNER_CAN_PHOTOGRAPH );
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	// Go back to idling if we're done
	if ( GetIdealActivity() == ACT_SCANNER_FLARE_START )
	{
		if ( IsSequenceFinished() )
		{
			SetIdealActivity( (Activity) ACT_IDLE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Overridden because if the player is a criminal, we hate them.
// Input  : pTarget - Entity with which to determine relationship.
// Output : Returns relationship value.
//-----------------------------------------------------------------------------
Disposition_t CNPC_CScanner::IRelationType(CBaseEntity *pTarget)
{
	// If it's the player and they are a criminal, we hates them
	if ( pTarget && pTarget->Classify() == CLASS_PLAYER )
	{
		if ( GlobalEntity_GetState("gordon_precriminal") == GLOBAL_ON )
			return D_NU;
	}

	return BaseClass::IRelationType( pTarget );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		case TASK_CSCANNER_PHOTOGRAPH:
		{
			if ( IsWaitFinished() )
			{	
				// If light was on turn it off
				if ( m_pEyeFlash->GetBrightness() > 0 )
				{
					m_pEyeFlash->SetBrightness( 0 );

					// I'm done with this target
					if ( gpGlobals->curtime > m_fInspectEndTime )
					{
						ClearInspectTarget();
						TaskComplete();
					}
					// Otherwise take another picture
					else
					{
						SetWait( 5.0f, 10.0f );
					}
				}
				// If light was off, take another picture
				else
				{
					TakePhoto();
					SetWait( 0.1f );
				}
			}
			break;
		}
		case TASK_CSCANNER_ATTACK_PRE_FLASH:
		{
			AttackPreFlash();
			
			if ( IsWaitFinished() )
			{
				TaskComplete();
			}
			break;
		}
		case TASK_CSCANNER_ATTACK_FLASH:
		{
			if (IsWaitFinished())
			{
				AttackFlashBlind();
				TaskComplete();
			}
			break;
		}
		case TASK_RANGE_ATTACK1:
			UpdateAttackBeam();
			if (!TaskIsRunning() || HasCondition(COND_TASK_FAILED))
			{
				KillAttackBeam();
			}
			break;
		default:
		{
			BaseClass::RunTask(pTask);
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Gets the appropriate next schedule based on current condition
//			bits.
//-----------------------------------------------------------------------------
int CNPC_CScanner::SelectSchedule(void)
{
	// Turn our flash off in case we were interrupted while it was on.
	if ( m_pEyeFlash )
	{
		m_pEyeFlash->SetBrightness( 0 );
	}

	// ----------------------------------------------------
	//  If I'm dead, go into a dive bomb
	// ----------------------------------------------------
	if ( m_iHealth <= 0 )
	{
		m_flSpeed = SCANNER_MAX_DIVE_BOMB_SPEED;
		return SCHED_SCANNER_ATTACK_DIVEBOMB;
	}

	
	// -------------------------------
	// If I'm in a script sequence
	// -------------------------------
	if ( m_NPCState == NPC_STATE_SCRIPT )
		return(BaseClass::SelectSchedule());
	if (!HasCondition(COND_SEE_ENEMY))
	{
		return SCHED_SCANNER_PATROL;
	}
	// -------------------------------
	// Flinch
	// -------------------------------
	if ( HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE) )
	{
		if ( IsHeldByPhyscannon( ) ) 
 			return SCHED_SMALL_FLINCH;

		if ( m_NPCState == NPC_STATE_IDLE )
			return SCHED_SMALL_FLINCH;

		if ( m_NPCState == NPC_STATE_ALERT )
		{
			if ( m_iHealth < ( 3 * sk_scanner_health.GetFloat() / 4 ))
				return SCHED_TAKE_COVER_FROM_ORIGIN;

			if ( SelectWeightedSequence( ACT_SMALL_FLINCH ) != -1 )
				return SCHED_SMALL_FLINCH;
		}
		else
		{
			if ( random->RandomInt( 0, 10 ) < 4 )
				return SCHED_SMALL_FLINCH;
		}
	}

	// I'm being held by the physcannon... struggle!
	if ( IsHeldByPhyscannon( ) ) 
		return SCHED_SCANNER_HELD_BY_PHYSCANNON;

	// ----------------------------------------------------------
	//  If I have an enemy
	// ----------------------------------------------------------
	if ( GetEnemy() != NULL && GetEnemy()->IsAlive() && m_bShouldInspect )
	{
		// Always chase the enemy
		SetInspectTargetToEnt( GetEnemy(), 9999 );

		// Patrol if the enemy has vanished
		if ( HasCondition( COND_LOST_ENEMY ) )
			return SCHED_SCANNER_PATROL;
		
		// Chase via route if we're directly blocked
		if ( HasCondition( COND_SCANNER_FLY_BLOCKED ) )
			return SCHED_SCANNER_CHASE_ENEMY;
		
		// Attack if it's time
		if ( gpGlobals->curtime < m_flNextAttack )
			return SCHED_CSCANNER_SPOTLIGHT_HOVER;

		if (HasCondition(COND_CAN_RANGE_ATTACK1))
			return SCHED_RANGE_ATTACK1;
		// Melee attack if possible
		if (HasCondition(COND_CAN_MELEE_ATTACK1) && HasCondition(COND_ENEMY_FACING_ME))
		{ 
			if ( random->RandomInt(0,1) )
				return SCHED_CSCANNER_ATTACK_FLASH;

			// TODO: a schedule where he makes an alarm sound?
			return SCHED_SCANNER_CHASE_ENEMY;
		}

		// If I'm far from the enemy, stay up high and approach in spotlight mode
		float fAttack2DDist = ( GetEnemyLKP() - GetAbsOrigin() ).Length2D();

		if ( fAttack2DDist > SCANNER_ATTACK_FAR_DIST )
			return SCHED_CSCANNER_SPOTLIGHT_HOVER;

		// Otherwise fly in low for attack
		return SCHED_SCANNER_ATTACK_HOVER;
	}

	// ----------------------------------------------------------
	//  If I have something to inspect
	// ----------------------------------------------------------
	if ( HaveInspectTarget() )
	{
		// Pathfind to our goal
		if ( HasCondition( COND_SCANNER_FLY_BLOCKED ) )
			return SCHED_CSCANNER_MOVE_TO_INSPECT;

		// If I was chasing, pick with photographing or spotlighting 
		if ( m_nFlyMode == SCANNER_FLY_CHASE )
		{
			m_nFlyMode = (random->RandomInt(0,1)==0) ? SCANNER_FLY_SPOT : SCANNER_FLY_PHOTO;
		}

		// Handle spotlight
		if ( m_nFlyMode == SCANNER_FLY_SPOT )
		{
			if (HasCondition( COND_CSCANNER_SPOT_ON_TARGET ))
			{
				if (GetTarget())
				{
					RequestInspectSupport();

					CAI_BaseNPC *pNPC = GetTarget()->MyNPCPointer();
					// If I'm leading the inspection, so verbal inspection
					if (pNPC && pNPC->GetTarget() == this)
					{
						return SCHED_CSCANNER_SPOTLIGHT_INSPECT_CIT;
					}

					return SCHED_CSCANNER_SPOTLIGHT_HOVER;
				}

				return SCHED_CSCANNER_SPOTLIGHT_INSPECT_POS;
			}

			return SCHED_CSCANNER_SPOTLIGHT_HOVER;
		}
		
		// Handle photographing
		if ( m_nFlyMode == SCANNER_FLY_PHOTO )
		{
			if ( HasCondition( COND_CSCANNER_CAN_PHOTOGRAPH ))
				return SCHED_CSCANNER_PHOTOGRAPH;

			return SCHED_CSCANNER_PHOTOGRAPH_HOVER;
		}
		
		// Handle following after a target
		if ( m_nFlyMode == SCANNER_FLY_FOLLOW )
		{
			//TODO: Randomly make noise, photograph, etc
			return SCHED_SCANNER_FOLLOW_HOVER;
		}

		// Handle patrolling
		if ( ( m_nFlyMode == SCANNER_FLY_PATROL ) || ( m_nFlyMode == SCANNER_FLY_FAST ) )
			return SCHED_SCANNER_PATROL;
	}

	// Default to patrolling around
	return SCHED_SCANNER_PATROL;
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::SpotlightDestroy(void)
{
	if ( m_hSpotlight )
	{
		UTIL_Remove(m_hSpotlight);
		m_hSpotlight = NULL;
		
		UTIL_Remove(m_hSpotlightTarget);
		m_hSpotlightTarget = NULL;
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::SpotlightCreate(void)
{
	// Make sure we don't already have one
	if ( m_hSpotlight != NULL )
		return;

	// Can we create a spotlight yet?
	if ( gpGlobals->curtime < m_fNextSpotlightTime )
		return;

	// If I have an enemy, start spotlight on my enemy
	if (GetEnemy() != NULL)
	{
		Vector vEnemyPos	= GetEnemyLKP();
		Vector vTargetPos	= vEnemyPos;
		vTargetPos.z		= GetFloorZ(vEnemyPos);
		m_vSpotlightDir = vTargetPos - GetLocalOrigin();
		VectorNormalize(m_vSpotlightDir);
	}
	// If I have an target, start spotlight on my target
	else if (GetTarget() != NULL)
	{
		Vector vTargetPos	= GetTarget()->GetLocalOrigin();
		vTargetPos.z		= GetFloorZ(GetTarget()->GetLocalOrigin());
		m_vSpotlightDir = vTargetPos - GetLocalOrigin();
		VectorNormalize(m_vSpotlightDir);
	}
	// Other wise just start looking down
	else
	{
		m_vSpotlightDir	= Vector(0,0,-1); 
	}

	trace_t tr;
	AI_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + m_vSpotlightDir * 2024, MASK_OPAQUE, this, COLLISION_GROUP_NONE, &tr );

	m_hSpotlightTarget = (CSpotlightEnd*)CreateEntityByName( "spotlight_end" );
	m_hSpotlightTarget->Spawn();
	m_hSpotlightTarget->SetLocalOrigin( tr.endpos );
	m_hSpotlightTarget->SetOwnerEntity( this );
	// YWB:  Because the scanner only moves the target during think, make sure we interpolate over 0.1 sec instead of every tick!!!
	m_hSpotlightTarget->SetSimulatedEveryTick( false );

	// Using the same color as the beam...
	m_hSpotlightTarget->SetRenderColor( 255, 0, 0 );
	m_hSpotlightTarget->m_Radius = m_flSpotlightMaxLength;

	m_hSpotlight = CBeam::BeamCreate( "sprites/glow_test02.vmt", SPOTLIGHT_WIDTH );
	// Set the temporary spawnflag on the beam so it doesn't save (we'll recreate it on restore)
	m_hSpotlight->AddSpawnFlags( SF_BEAM_TEMPORARY );
	m_hSpotlight->SetColor( 255, 0, 0 ); 
	m_hSpotlight->SetHaloTexture( m_nHaloSprite );
	m_hSpotlight->SetHaloScale( 32 );
	m_hSpotlight->SetEndWidth( m_hSpotlight->GetWidth() );
	m_hSpotlight->SetBeamFlags( (FBEAM_SHADEOUT|FBEAM_NOTILE) );
	m_hSpotlight->SetBrightness( 32 );
	m_hSpotlight->SetNoise( 0 );
	m_hSpotlight->EntsInit( this, m_hSpotlightTarget );
	m_hSpotlight->SetHDRColorScale( 0.75f );	// Scale this back a bit on HDR maps
	// attach to light
	m_hSpotlight->SetStartAttachment( LookupAttachment( SCANNER_ATTACHMENT_LIGHT ) );

	m_vSpotlightAngVelocity = vec3_origin;
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
Vector CNPC_CScanner::SpotlightTargetPos(void)
{
	// ----------------------------------------------
	//  If I have an enemy 
	// ----------------------------------------------
	if (GetEnemy() != NULL)
	{
		// If I can see my enemy aim for him
		if (HasCondition(COND_SEE_ENEMY))
		{
			// If its client aim for his eyes
			if (GetEnemy()->GetFlags() & FL_CLIENT)
			{
				m_vSpotlightTargetPos = GetEnemy()->EyePosition();
			}
			// Otherwise same for his feet
			else
			{
				m_vSpotlightTargetPos	= GetEnemy()->GetLocalOrigin();
				m_vSpotlightTargetPos.z	= GetFloorZ(GetEnemy()->GetLocalOrigin());
			}
		}
		// Otherwise aim for last known position if I can see LKP
		else
		{
			Vector vLKP				= GetEnemyLKP();
			m_vSpotlightTargetPos.x	= vLKP.x;
			m_vSpotlightTargetPos.y	= vLKP.y;
			m_vSpotlightTargetPos.z	= GetFloorZ(vLKP);
		}
	}
	// ----------------------------------------------
	//  If I have an inspect target
	// ----------------------------------------------
	else if (HaveInspectTarget())
	{
		m_vSpotlightTargetPos = InspectTargetPosition();
	}
	else
	{
		// This creates a nice patrol spotlight sweep
		// in the direction that I'm travelling
		m_vSpotlightTargetPos	= GetCurrentVelocity();
		m_vSpotlightTargetPos.z = 0;
		VectorNormalize( m_vSpotlightTargetPos );
		m_vSpotlightTargetPos   *= 5;

		float noiseScale = 2.5;
		const Vector &noiseMod = GetNoiseMod();
		m_vSpotlightTargetPos.x += noiseScale*sin(noiseMod.x * gpGlobals->curtime + noiseMod.x);
		m_vSpotlightTargetPos.y += noiseScale*cos(noiseMod.y* gpGlobals->curtime + noiseMod.y);
		m_vSpotlightTargetPos.z -= fabs(noiseScale*cos(noiseMod.z* gpGlobals->curtime + noiseMod.z) );
		m_vSpotlightTargetPos   = GetLocalOrigin()+m_vSpotlightTargetPos * 2024;
	}

	return m_vSpotlightTargetPos;
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
Vector CNPC_CScanner::SpotlightCurrentPos(void)
{
	Vector vTargetDir		= SpotlightTargetPos() - GetLocalOrigin();
	VectorNormalize(vTargetDir);

	if (!m_hSpotlight)
	{
		DevMsg("Spotlight pos. called w/o spotlight!\n");
		return vec3_origin;
	}
	// -------------------------------------------------
	//  Beam has momentum relative to it's ground speed
	//  so sclae the turn rate based on its distance
	//  from the beam source
	// -------------------------------------------------
	float	fBeamDist		= (m_hSpotlightTarget->GetLocalOrigin() - GetLocalOrigin()).Length();

	float	fBeamTurnRate	= atan(50/fBeamDist);
	Vector  vNewAngVelocity = fBeamTurnRate * (vTargetDir - m_vSpotlightDir);

	float	myDecay	 = 0.4;
	m_vSpotlightAngVelocity = (myDecay * m_vSpotlightAngVelocity + (1-myDecay) * vNewAngVelocity);

	// ------------------------------
	//  Limit overall angular speed
	// -----------------------------
	if (m_vSpotlightAngVelocity.Length() > 1)
	{

		Vector velDir = m_vSpotlightAngVelocity;
		VectorNormalize(velDir);
		m_vSpotlightAngVelocity = velDir * 1;
	}

	// ------------------------------
	//  Calculate new beam direction
	// ------------------------------
	m_vSpotlightDir = m_vSpotlightDir + m_vSpotlightAngVelocity;
	m_vSpotlightDir = m_vSpotlightDir;
	VectorNormalize(m_vSpotlightDir);


	// ---------------------------------------------
	//	Get beam end point.  Only collide with
	//  solid objects, not npcs
	// ---------------------------------------------
	trace_t tr;
	Vector vTraceEnd = GetAbsOrigin() + (m_vSpotlightDir * 2 * m_flSpotlightMaxLength);
	AI_TraceLine ( GetAbsOrigin(), vTraceEnd, MASK_OPAQUE, this, COLLISION_GROUP_NONE, &tr);

	return (tr.endpos);
}


//------------------------------------------------------------------------------
// Purpose: Update the direction and position of my spotlight
//------------------------------------------------------------------------------
void CNPC_CScanner::SpotlightUpdate(void)
{
	//FIXME: JDW - E3 Hack
	if ( m_bNoLight )
	{
		if ( m_hSpotlight )
		{
			SpotlightDestroy();
		}

		return;
	}

	if ((m_nFlyMode != SCANNER_FLY_SPOT) &&
		(m_nFlyMode != SCANNER_FLY_PATROL) && 
		(m_nFlyMode != SCANNER_FLY_FAST))
	{
		if ( m_hSpotlight )
		{	
			SpotlightDestroy();
		}
		return;
	}
	
	// If I don't have a spotlight attempt to create one

	if ( m_hSpotlight == NULL )
	{
		SpotlightCreate();
		
		if ( m_hSpotlight== NULL )
			return;
	}

	// Calculate the new homing target position
	m_vSpotlightCurrentPos = SpotlightCurrentPos();

	// ------------------------------------------------------------------
	//  If I'm not facing the spotlight turn it off 
	// ------------------------------------------------------------------
	Vector vSpotDir = m_vSpotlightCurrentPos - GetAbsOrigin();
	VectorNormalize(vSpotDir);
	
	Vector	vForward;
	AngleVectors( GetAbsAngles(), &vForward );

	float dotpr = DotProduct( vForward, vSpotDir );
	
	if ( dotpr < 0.0 )
	{
		// Leave spotlight off for a while
		m_fNextSpotlightTime = gpGlobals->curtime + 3.0f;

		SpotlightDestroy();
		return;
	}

	// --------------------------------------------------------------
	//  Update spotlight target velocity
	// --------------------------------------------------------------
	Vector vTargetDir  = (m_vSpotlightCurrentPos - m_hSpotlightTarget->GetLocalOrigin());
	float  vTargetDist = vTargetDir.Length();

	Vector vecNewVelocity = vTargetDir;
	VectorNormalize(vecNewVelocity);
	vecNewVelocity *= (10 * vTargetDist);

	// If a large move is requested, just jump to final spot as we
	// probably hit a discontinuity
	if (vecNewVelocity.Length() > 200)
	{
		VectorNormalize(vecNewVelocity);
		vecNewVelocity *= 200;
		m_hSpotlightTarget->SetLocalOrigin( m_vSpotlightCurrentPos );
	}
	m_hSpotlightTarget->SetAbsVelocity( vecNewVelocity );

	m_hSpotlightTarget->m_vSpotlightOrg = GetAbsOrigin();

	// Avoid sudden change in where beam fades out when cross disconinuities
	m_hSpotlightTarget->m_vSpotlightDir = m_hSpotlightTarget->GetLocalOrigin() - m_hSpotlightTarget->m_vSpotlightOrg;
	float flBeamLength	= VectorNormalize( m_hSpotlightTarget->m_vSpotlightDir );
	m_flSpotlightCurLength = (0.80*m_flSpotlightCurLength) + (0.2*flBeamLength);

	// Fade out spotlight end if past max length.  
	if (m_flSpotlightCurLength > 2*m_flSpotlightMaxLength)
	{
		m_hSpotlightTarget->SetRenderColorA( 0 );
		m_hSpotlight->SetFadeLength(m_flSpotlightMaxLength);
	}
	else if (m_flSpotlightCurLength > m_flSpotlightMaxLength)		
	{
		m_hSpotlightTarget->SetRenderColorA( (1-((m_flSpotlightCurLength-m_flSpotlightMaxLength)/m_flSpotlightMaxLength)) );
		m_hSpotlight->SetFadeLength(m_flSpotlightMaxLength);
	}
	else
	{
		m_hSpotlightTarget->SetRenderColorA( 1.0 );
		m_hSpotlight->SetFadeLength(m_flSpotlightCurLength);
	}

	// Adjust end width to keep beam width constant
	float flNewWidth = SPOTLIGHT_WIDTH * ( flBeamLength/m_flSpotlightMaxLength);
	
	m_hSpotlight->SetWidth(flNewWidth);
	m_hSpotlight->SetEndWidth(flNewWidth);

	m_hSpotlightTarget->m_flLightScale = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: Called just before we are deleted.
//-----------------------------------------------------------------------------
void CNPC_CScanner::UpdateOnRemove( void )
{
	SpotlightDestroy();
	BaseClass::UpdateOnRemove();
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::TakePhoto(void)
{
	ScannerEmitSound( "TakePhoto" );
	
	m_pEyeFlash->SetScale( 1.4 );
	m_pEyeFlash->SetBrightness( 255 );
	m_pEyeFlash->SetColor(255,255,255);

	Vector vRawPos		= InspectTargetPosition();
	Vector vLightPos	= vRawPos;

	// If taking picture of entity, aim at feet
	if ( GetTarget() )
	{
		if ( GetTarget()->IsPlayer() )
		{
			m_OnPhotographPlayer.FireOutput( GetTarget(), this );
			BlindFlashTarget( GetTarget() );
		}
		
		if ( GetTarget()->MyNPCPointer() != NULL )
		{
			m_OnPhotographNPC.FireOutput( GetTarget(), this );
			GetTarget()->MyNPCPointer()->DispatchInteraction( g_interactionScannerInspectBegin, NULL, this );
		}
	}

	SetIdealActivity( (Activity) ACT_SCANNER_FLARE_START );

	m_bPhotoTaken = true;
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::AttackPreFlash(void)
{
	ScannerEmitSound( "TakePhoto" );

	// If off turn on, if on turn off
	if (m_pEyeFlash->GetBrightness() == 0)
	{
		m_pEyeFlash->SetScale( 0.5 );
		m_pEyeFlash->SetBrightness( 255 );
		m_pEyeFlash->SetColor(255,0,0);
	}
	else
	{
		m_pEyeFlash->SetBrightness( 0 );
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::AttackFlash(void)
{
	ScannerEmitSound( "AttackFlash" );
	m_pEyeFlash->SetScale( 1.8 );
	m_pEyeFlash->SetBrightness( 255 );
	m_pEyeFlash->SetColor(255,255,255);

	if (GetEnemy() != NULL)
	{
		Vector pos = GetEnemyLKP();
		CBroadcastRecipientFilter filter;
		te->DynamicLight( filter, 0.0, &pos, 200, 200, 255, 0, 300, 0.2, 50 );

		if (GetEnemy()->IsPlayer())
		{
			m_OnPhotographPlayer.FireOutput(GetTarget(), this);
		}
		else if( GetEnemy()->MyNPCPointer() )
		{
			m_OnPhotographNPC.FireOutput(GetTarget(), this);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::BlindFlashTarget( CBaseEntity *pTarget )
{
	Vector vecSrc = GetAbsOrigin();
	Vector vecEnd = pTarget->WorldSpaceCenter();
	Vector vecAim = vecEnd - vecSrc;
	VectorNormalize(vecAim);

	float flSpd = g_pGameRules->SkillAdjustValue(sk_scanner_projectile_speed.GetFloat());

	CHLRScannerProjectile *pPew = (CHLRScannerProjectile*)CreateEntityByName("hlr_scannerprojectile");
	pPew->SetAbsOrigin(vecSrc);
	pPew->Spawn();

	pPew->SetAbsVelocity(vecAim * flSpd);
	pPew->SetOwnerEntity(this);
	
	/*// Tell all the striders this person is here!
	CAI_BaseNPC **	ppAIs 	= g_AI_Manager.AccessAIs();
	int 			nAIs 	= g_AI_Manager.NumAIs();
	
	if( IsStriderScout() )
	{
		for ( int i = 0; i < nAIs; i++ )
		{
			if( FClassnameIs( ppAIs[ i ], "npc_strider" ) )
			{
				ppAIs[ i ]->UpdateEnemyMemory( pTarget, pTarget->GetAbsOrigin(), this );
			}
		}
	}

	// Only bother with player
	if ( pTarget->IsPlayer() == false )
		return;

	// Scale the flash value by how closely the player is looking at me
	Vector vFlashDir = GetAbsOrigin() - pTarget->EyePosition();
	VectorNormalize(vFlashDir);
	
	Vector vFacing;
	AngleVectors( pTarget->EyeAngles(), &vFacing );

	float dotPr	= DotProduct( vFlashDir, vFacing );

	// Not if behind us
	if ( dotPr > 0.5f )
	{
		// Make sure nothing in the way
		trace_t tr;
		AI_TraceLine ( GetAbsOrigin(), pTarget->EyePosition(), MASK_OPAQUE, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.startsolid == false && tr.fraction == 1.0)
		{
			color32 white = { 255, 255, 255, SCANNER_FLASH_MAX_VALUE * dotPr };

			if ( ( g_pMaterialSystemHardwareConfig != NULL ) && ( g_pMaterialSystemHardwareConfig->GetHDRType() != HDR_TYPE_NONE ) )
			{
				white.a = ( byte )( ( float )white.a * 0.9f );
			}

			float flFadeTime = ( IsX360() ) ? 0.5f : 3.0f;
			UTIL_ScreenFade( pTarget, white, flFadeTime, 0.5, FFADE_IN );
		}
	}*/
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::AttackFlashBlind(void)
{
	if( GetEnemy() )
	{
		BlindFlashTarget( GetEnemy() );
	}

	m_pEyeFlash->SetBrightness( 0 );

	float fAttackDelay = random->RandomFloat(SCANNER_ATTACK_MIN_DELAY,SCANNER_ATTACK_MAX_DELAY);

	if( IsStriderScout() )
	{
		// Make strider scouts more snappy.
		fAttackDelay *= 0.5;
	}

	m_flNextAttack	= gpGlobals->curtime + fAttackDelay;
	m_fNextSpotlightTime = gpGlobals->curtime + 1.0f;
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_CScanner::AttackDivebomb( void )
{
	if (m_hSpotlight)
	{
		SpotlightDestroy();
	}

	BaseClass::AttackDivebomb();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::StartTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
	case TASK_CSCANNER_GET_PATH_TO_INSPECT_TARGET:
		{
			// Must have somewhere to fly to
			if ( HaveInspectTarget() == false )
			{
				TaskFail( "No inspection target to fly to!\n" );
				return;
			}

			if ( GetTarget() )
			{	
				//FIXME: Tweak
				//Vector idealPos = IdealGoalForMovement( InspectTargetPosition(), GetAbsOrigin(), 128.0f, 128.0f );
				
				AI_NavGoal_t goal( GOALTYPE_TARGETENT, vec3_origin );
			
				if ( GetNavigator()->SetGoal( goal ) )
				{
					TaskComplete();
					return;
				}
			}
			else
			{
				AI_NavGoal_t goal( GOALTYPE_LOCATION, InspectTargetPosition() );
			
				if ( GetNavigator()->SetGoal( goal ) )
				{
					TaskComplete();
					return;
				}
			}

			// Don't try and inspect this target again for a few seconds
			CNPC_Citizen *pCitizen = dynamic_cast<CNPC_Citizen *>( GetTarget() );
			if ( pCitizen )
			{
				pCitizen->SetNextScannerInspectTime( gpGlobals->curtime + 5.0 );
			}

			TaskFail("No route to inspection target!\n");
		}
		break;

	case TASK_CSCANNER_SPOT_INSPECT_ON:
	{
		if (GetTarget() == NULL)
		{
			TaskFail(FAIL_NO_TARGET);
		}
		else
		{
			CAI_BaseNPC* pNPC = GetTarget()->MyNPCPointer();
			if (!pNPC)
			{
				TaskFail(FAIL_NO_TARGET);
			}
			else
			{
				pNPC->DispatchInteraction(g_interactionScannerInspectBegin,NULL,this);
				
				// Now we need some time to inspect
				m_fInspectEndTime = gpGlobals->curtime + SCANNER_CIT_INSPECT_LENGTH;
				TaskComplete();
			}
		}
		break;
	}
	case TASK_CSCANNER_SPOT_INSPECT_WAIT:
	{
		if (GetTarget() == NULL)
		{
			TaskFail(FAIL_NO_TARGET);
		}
		else
		{
			CAI_BaseNPC* pNPC = GetTarget()->MyNPCPointer();
			if (!pNPC)
			{
				SetTarget( NULL );
				TaskFail(FAIL_NO_TARGET);
			}
			else
			{
				//<<TEMP>>//<<TEMP>> armband too!
				pNPC->DispatchInteraction(g_interactionScannerInspectHandsUp,NULL,this);
			}
			TaskComplete();
		}
		break;
	}
	case TASK_CSCANNER_SPOT_INSPECT_OFF:
	{
		if (GetTarget() == NULL)
		{
			TaskFail(FAIL_NO_TARGET);
		}
		else
		{
			CAI_BaseNPC* pNPC = GetTarget()->MyNPCPointer();
			if (!pNPC)
			{
				TaskFail(FAIL_NO_TARGET);
			}
			else
			{
				pNPC->DispatchInteraction(g_interactionScannerInspectDone,NULL,this);

				// Clear target entity and don't inspect again for a while
				SetTarget( NULL );
				m_fCheckCitizenTime = gpGlobals->curtime + SCANNER_CIT_INSPECT_DELAY;
				TaskComplete();
			}
		}
		break;
	}
	case TASK_CSCANNER_CLEAR_INSPECT_TARGET:
	{
		ClearInspectTarget();

		TaskComplete();
		break;
	}

	case TASK_CSCANNER_SET_FLY_SPOT:
	{
		m_nFlyMode = SCANNER_FLY_SPOT;
		TaskComplete();
		break;
	}

	case TASK_CSCANNER_SET_FLY_PHOTO:
	{
		m_nFlyMode = SCANNER_FLY_PHOTO;
		m_bPhotoTaken = false;

		// Leave spotlight off for a while
		m_fNextSpotlightTime = gpGlobals->curtime + 2.0;

		TaskComplete();
		break;
	}

	case TASK_CSCANNER_PHOTOGRAPH:
	{
		TakePhoto();
		SetWait( 0.1 );
		break;
	}

	case TASK_CSCANNER_ATTACK_PRE_FLASH:
	{
		if( IsStriderScout() )
		{
			Vector vecScare = GetEnemy()->EarPosition();
			Vector vecDir = WorldSpaceCenter() - vecScare;
			VectorNormalize( vecDir );
			vecScare += vecDir * 64.0f;

			CSoundEnt::InsertSound( SOUND_DANGER, vecScare, 256, 1.0, this );
		}

		if (m_pEyeFlash)
		{
			AttackPreFlash();
			// Flash red for a while
			SetWait( 1.0f );
		}
		else
		{
			TaskFail("No Flash");
		}
		break;
	}

	case TASK_CSCANNER_ATTACK_FLASH:
	{
		AttackFlash();
		// Blinding occurs slightly later
		SetWait( 0.05 );
		break;
	}

	// Override to go to inspect target position whether or not is an entity
	case TASK_GET_PATH_TO_TARGET:
	{
		if (!HaveInspectTarget())
		{
			TaskFail(FAIL_NO_TARGET);
		}
		else if (GetHintNode())
		{
			Vector vNodePos;
			GetHintNode()->GetPosition(this,&vNodePos);

			GetNavigator()->SetGoal( vNodePos );
		}
		else 
		{
			AI_NavGoal_t goal( (const Vector &)InspectTargetPosition() );
			goal.pTarget = GetTarget();
			GetNavigator()->SetGoal( goal );
		}
		break;
	}

	///beamtest
	case TASK_RANGE_ATTACK1:
	{
		CBaseEntity *pEnemy = GetEnemy();
		if (pEnemy)
		{
			m_vLaserTargetPos = GetEnemyLKP() + pEnemy->GetViewOffset();

			// Never hit target on first try
			Vector missPos = m_vLaserTargetPos;

			if (pEnemy->Classify() == CLASS_BULLSEYE && hl2_episodic.GetBool())
			{
				missPos.x += 60 + 120 * random->RandomInt(-1, 1);
				missPos.y += 60 + 120 * random->RandomInt(-1, 1);
			}
			else
			{
				missPos.x += 80 * random->RandomInt(-1, 1);
				missPos.y += 80 * random->RandomInt(-1, 1);
			}

			// ----------------------------------------------------------------------
			// If target is facing me and not running towards me shoot below his feet
			// so he can see the laser coming
			// ----------------------------------------------------------------------
			CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(pEnemy);
			if (pBCC)
			{
				Vector targetToMe = (pBCC->GetAbsOrigin() - GetAbsOrigin());
				Vector vBCCFacing = pBCC->BodyDirection2D();
				if ((DotProduct(vBCCFacing, targetToMe) < 0) &&
					(pBCC->GetSmoothedVelocity().Length() < 50))
				{
					missPos.z -= 150;
				}
				// --------------------------------------------------------
				// If facing away or running towards laser,
				// shoot above target's head 
				// --------------------------------------------------------
				else
				{
					missPos.z += 60;
				}
			}
			m_vLaserDir = missPos - LaserStartPosition(GetAbsOrigin());
			VectorNormalize(m_vLaserDir);
		}
		else
		{
			TaskFail(FAIL_NO_ENEMY);
			return;
		}

		StartAttackBeam();
		SetActivity(ACT_RANGE_ATTACK1);
		break;
	}
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CNPC_CScanner::StartAttackBeam(void)
{
	if (m_fBeamEndTime > gpGlobals->curtime || m_fBeamRechargeTime > gpGlobals->curtime)
	{
		// UNDONE: Debug this and fix!?!?!
		m_fBeamRechargeTime = gpGlobals->curtime;
	}
	// ---------------------------------------------
	//  If I don't have a beam yet, create one
	// ---------------------------------------------
	// UNDONE: Why would I ever have a beam already?!?!?!
	if (!m_pBeam)
	{
		Vector vecSrc = LaserStartPosition(GetAbsOrigin());
		trace_t tr;
		AI_TraceLine(vecSrc, vecSrc + m_vLaserDir * MAX_STALKER_FIRE_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction >= 1.0)
		{
			// too far
			TaskComplete();
			return;
		}

		m_pBeam = CBeam::BeamCreate("sprites/laser.vmt", 2.0);
		m_pBeam->PointEntInit(tr.endpos, this);
		m_pBeam->SetEndAttachment(STALKER_LASER_ATTACHMENT);
		m_pBeam->SetBrightness(255);
		m_pBeam->SetNoise(0);

		switch (m_eBeamPower)
		{
		case STALKER_BEAM_LOW:
			m_pBeam->SetColor(255, 0, 0);
			m_pLightGlow = CSprite::SpriteCreate("sprites/redglow1.vmt", GetAbsOrigin(), FALSE);
			break;
		case STALKER_BEAM_MED:
			m_pBeam->SetColor(255, 50, 0);
			m_pLightGlow = CSprite::SpriteCreate("sprites/orangeglow1.vmt", GetAbsOrigin(), FALSE);
			break;
		case STALKER_BEAM_HIGH:
			m_pBeam->SetColor(255, 150, 0);
			m_pLightGlow = CSprite::SpriteCreate("sprites/yellowglow1.vmt", GetAbsOrigin(), FALSE);
			break;
		}

		// ----------------------------
		// Light myself in a red glow
		// ----------------------------
		m_pLightGlow->SetTransparency(kRenderGlow, 255, 200, 200, 0, kRenderFxNoDissipation);
		m_pLightGlow->SetAttachment(this, 1);
		m_pLightGlow->SetBrightness(255);
		m_pLightGlow->SetScale(0.65);

#if 0
		CBaseEntity *pEnemy = GetEnemy();
		// --------------------------------------------------------
		// Play start up sound - client should always hear this!
		// --------------------------------------------------------
		if (pEnemy != NULL && (pEnemy->IsPlayer()))
		{
			EmitAmbientSound(0, pEnemy->GetAbsOrigin(), "NPC_Stalker.AmbientLaserStart");
		}
		else
		{
			EmitAmbientSound(0, GetAbsOrigin(), "NPC_Stalker.AmbientLaserStart");
		}
#endif
	}

	SetThink(&CNPC_CScanner::BeamThink);

	m_flNextNPCThink = GetNextThink();
	SetNextThink(gpGlobals->curtime + g_StalkerBeamThinkTime);
	m_fBeamEndTime = gpGlobals->curtime + STALKER_LASER_DURATION;
}
void CNPC_CScanner::DrawAttackBeam(void)
{
	if (!m_pBeam)
		return;

	// ---------------------------------------------
	//	Get beam end point
	// ---------------------------------------------
	Vector vecSrc = LaserStartPosition(GetAbsOrigin());
	trace_t tr;
	AI_TraceLine(vecSrc, vecSrc + m_vLaserDir * MAX_STALKER_FIRE_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	CalcBeamPosition();

	bool bInWater = (UTIL_PointContents(tr.endpos) & MASK_WATER) ? true : false;
	// ---------------------------------------------
	//	Update the beam position
	// ---------------------------------------------
	m_pBeam->SetStartPos(tr.endpos);
	m_pBeam->RelinkBeam();

	Vector vAttachPos;
	GetAttachment(STALKER_LASER_ATTACHMENT, vAttachPos);

	Vector vecAimDir = tr.endpos - vAttachPos;
	VectorNormalize(vecAimDir);

	SetAim(vecAimDir);

	// --------------------------------------------
	//  Play burn sounds
	// --------------------------------------------
	CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(tr.m_pEnt);
	if (pBCC)
	{
		if (gpGlobals->curtime > m_fNextDamageTime)
		{
			ClearMultiDamage();

			float damage = 0.0;
			switch (m_eBeamPower)
			{
			case STALKER_BEAM_LOW:
				damage = 1;
				break;
			case STALKER_BEAM_MED:
				damage = 3;
				break;
			case STALKER_BEAM_HIGH:
				damage = 10;
				break;
			}

			CTakeDamageInfo info(this, this, damage, DMG_SHOCK);
			CalculateMeleeDamageForce(&info, m_vLaserDir, tr.endpos);
			pBCC->DispatchTraceAttack(info, m_vLaserDir, &tr);
			ApplyMultiDamage();
			m_fNextDamageTime = gpGlobals->curtime + 0.1;
		}
		if (pBCC->Classify() != CLASS_BULLSEYE)
		{
			if (!m_bPlayingHitFlesh)
			{
				CPASAttenuationFilter filter(m_pBeam, "NPC_Stalker.BurnFlesh");
				filter.MakeReliable();

				EmitSound(filter, m_pBeam->entindex(), "NPC_Stalker.BurnFlesh");
				m_bPlayingHitFlesh = true;
			}
			if (m_bPlayingHitWall)
			{
				StopSound(m_pBeam->entindex(), "NPC_Stalker.BurnWall");
				m_bPlayingHitWall = false;
			}

			tr.endpos.z -= 24.0f;
			if (!bInWater)
			{
				//DoSmokeEffect(tr.endpos + tr.plane.normal * 8);
			}
		}
	}

	if (!pBCC || pBCC->Classify() == CLASS_BULLSEYE)
	{
		if (!m_bPlayingHitWall)
		{
			CPASAttenuationFilter filter(m_pBeam, "NPC_Stalker.BurnWall");
			filter.MakeReliable();

			EmitSound(filter, m_pBeam->entindex(), "NPC_Stalker.BurnWall");
			m_bPlayingHitWall = true;
		}
		if (m_bPlayingHitFlesh)
		{
			StopSound(m_pBeam->entindex(), "NPC_Stalker.BurnFlesh");
			m_bPlayingHitFlesh = false;
		}

		//UTIL_DecalTrace(&tr, "RedGlowFade");
		//UTIL_DecalTrace(&tr, "FadingScorch");

		tr.endpos.z -= 24.0f;
		if (!bInWater)
		{
			//DoSmokeEffect(tr.endpos + tr.plane.normal * 8);
		}
	}

	if (bInWater)
	{
		UTIL_Bubbles(tr.endpos - Vector(3, 3, 3), tr.endpos + Vector(3, 3, 3), 10);
	}

	/*
	CBroadcastRecipientFilter filter;
	TE_DynamicLight( filter, 0.0, EyePosition(), 255, 0, 0, 5, 0.2, 0 );
	*/
}
void CNPC_CScanner::UpdateAttackBeam(void)
{
	CBaseEntity *pEnemy = GetEnemy();
	// If not burning at a target 
	if (pEnemy)
	{
		if (gpGlobals->curtime > m_fBeamEndTime)
		{
			TaskComplete();
		}
		else
		{
			Vector enemyLKP = GetEnemyLKP();
			m_vLaserTargetPos = enemyLKP + pEnemy->GetViewOffset();

			// Face my enemy
			GetMotor()->SetIdealYawToTargetAndUpdate(enemyLKP);

			// ---------------------------------------------
			//	Get beam end point
			// ---------------------------------------------
			Vector vecSrc = LaserStartPosition(GetAbsOrigin());
			Vector targetDir = m_vLaserTargetPos - vecSrc;
			VectorNormalize(targetDir);
			// --------------------------------------------------------
			//	If beam position and laser dir are way off, end attack
			// --------------------------------------------------------
			if (DotProduct(targetDir, m_vLaserDir) < 0.5)
			{
				TaskComplete();
				return;
			}

			trace_t tr;
			AI_TraceLine(vecSrc, vecSrc + m_vLaserDir * MAX_STALKER_FIRE_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
			// ---------------------------------------------
			//  If beam not long enough, stop attacking
			// ---------------------------------------------
			if (tr.fraction == 1.0)
			{
				TaskComplete();
				return;
			}

			CSoundEnt::InsertSound(SOUND_DANGER, tr.endpos, 60, 0.025, this);
		}
	}
	else
	{
		TaskFail(FAIL_NO_ENEMY);
	}
}
int CNPC_CScanner::RangeAttack1Conditions(float flDot, float flDist)
{
	if (gpGlobals->curtime < m_fBeamRechargeTime)
	{
		return COND_NONE;
	}
	if (flDist <= MIN_STALKER_FIRE_RANGE)
	{
		return COND_TOO_CLOSE_TO_ATTACK;
	}
	else if (flDist > (MAX_STALKER_FIRE_RANGE * 0.66f))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}
	return COND_CAN_RANGE_ATTACK1;
}
bool CNPC_CScanner::InnateWeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions)
{
	// --------------------
	// Check for occlusion
	// --------------------
	// Base class version assumes innate weapon position is at eye level
	Vector barrelPos = LaserStartPosition(ownerPos);
	trace_t tr;
	AI_TraceLine(barrelPos, targetPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction == 1.0)
	{
		return true;
	}

	CBaseEntity *pBE = tr.m_pEnt;
	CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(pBE);
	if (pBE == GetEnemy())
	{
		return true;
	}
	else if (pBCC)
	{
		if (IRelationType(pBCC) == D_HT)
		{
			return true;
		}
		else if (bSetConditions)
		{
			SetCondition(COND_WEAPON_BLOCKED_BY_FRIEND);
		}
	}
	else if (bSetConditions)
	{
		SetCondition(COND_WEAPON_SIGHT_OCCLUDED);
		SetEnemyOccluder(pBE);
	}

	return false;
}
Vector CNPC_CScanner::LaserStartPosition(Vector vStalkerPos)
{
	// Get attachment position
	Vector vAttachPos;
	GetAttachment(STALKER_LASER_ATTACHMENT, vAttachPos);

	// Now convert to vStalkerPos
	vAttachPos = vAttachPos - GetAbsOrigin() + vStalkerPos;
	return vAttachPos;
}
void CNPC_CScanner::KillAttackBeam(void)
{
	if (!m_pBeam)
		return;

	// Kill sound
	StopSound(m_pBeam->entindex(), "NPC_Stalker.BurnWall");
	StopSound(m_pBeam->entindex(), "NPC_Stalker.BurnFlesh");

	UTIL_Remove(m_pLightGlow);
	UTIL_Remove(m_pBeam);
	m_pBeam = NULL;
	m_bPlayingHitWall = false;
	m_bPlayingHitFlesh = false;

	SetThink(&CNPC_CScanner::CallNPCThink);
	if (m_flNextNPCThink > gpGlobals->curtime)
	{
		SetNextThink(m_flNextNPCThink);
	}

	// Beam has to recharge
	m_fBeamRechargeTime = gpGlobals->curtime + STALKER_LASER_RECHARGE;

	ClearCondition(COND_CAN_RANGE_ATTACK1);

	RelaxAim();
}
void CNPC_CScanner::BeamThink(void)
{
	DrawAttackBeam();
	if (gpGlobals->curtime >= m_flNextNPCThink)
	{
		NPCThink();
		m_flNextNPCThink = GetNextThink();
	}

	if (m_pBeam)
	{
		SetNextThink(gpGlobals->curtime + g_StalkerBeamThinkTime);

		// sanity check?!
		const Task_t *pTask = GetTask();
		if (!pTask || pTask->iTask != TASK_RANGE_ATTACK1 || !TaskIsRunning())
		{
			KillAttackBeam();
		}
	}
	else
	{
		DevMsg(2, "In StalkerThink() but no stalker beam found?\n");
		SetNextThink(m_flNextNPCThink);
	}
}
void CNPC_CScanner::CalcBeamPosition(void)
{
	Vector targetDir = m_vLaserTargetPos - LaserStartPosition(GetAbsOrigin());
	VectorNormalize(targetDir);

	// ---------------------------------------
	//  Otherwise if burning towards an enemy
	// ---------------------------------------
	if (GetEnemy())
	{
		// ---------------------------------------
		//  Integrate towards target position
		// ---------------------------------------
		float	iRate = 0.95;

		if (GetEnemy()->Classify() == CLASS_BULLSEYE)
		{
			// Seek bullseyes faster
			iRate = 0.8;
		}

		m_vLaserDir.x = (iRate * m_vLaserDir.x + (1 - iRate) * targetDir.x);
		m_vLaserDir.y = (iRate * m_vLaserDir.y + (1 - iRate) * targetDir.y);
		m_vLaserDir.z = (iRate * m_vLaserDir.z + (1 - iRate) * targetDir.z);
		VectorNormalize(m_vLaserDir);

		// -----------------------------------------
		// Add time-coherent noise to the position
		// Must be scaled with distance 
		// -----------------------------------------
		float fTargetDist = (GetAbsOrigin() - m_vLaserTargetPos).Length();
		float noiseScale = atan(0.2 / fTargetDist);
		float m_fNoiseModX = 5;
		float m_fNoiseModY = 5;
		float m_fNoiseModZ = 5;

		m_vLaserDir.x += 5 * noiseScale*sin(m_fNoiseModX * gpGlobals->curtime + m_fNoiseModX);
		m_vLaserDir.y += 5 * noiseScale*sin(m_fNoiseModY * gpGlobals->curtime + m_fNoiseModY);
		m_vLaserDir.z += 5 * noiseScale*sin(m_fNoiseModZ * gpGlobals->curtime + m_fNoiseModZ);
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
char *CNPC_CScanner::GetScannerSoundPrefix( void )
{
	if( m_bIsClawScanner )
		return "NPC_SScanner";

	return "NPC_CScanner";
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
float CNPC_CScanner::MinGroundDist( void )
{
	if ( m_nFlyMode == SCANNER_FLY_SPOT && !GetHintNode() )
	{
		return SCANNER_SPOTLIGHT_FLY_HEIGHT;
	}

	return SCANNER_NOSPOTLIGHT_FLY_HEIGHT;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CScanner::AdjustScannerVelocity( void )
{
	if ( m_bIsClawScanner )
	{
		m_vCurrentVelocity *= ( 1 + sin( ( gpGlobals->curtime + m_flFlyNoiseBase ) * 2.5f ) * .1 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_CScanner::OverrideMove( float flInterval )
{
	// ----------------------------------------------
	//	If dive bombing
	// ----------------------------------------------
	if (m_nFlyMode == SCANNER_FLY_DIVE)
	{
		MoveToDivebomb( flInterval );
	}
	else
	{
		Vector vMoveTargetPos(0,0,0);
		CBaseEntity *pMoveTarget = NULL;
		
		// The original line of code was, due to the accidental use of '|' instead of
		// '&', always true. Replacing with 'true' to suppress the warning without changing
		// the (long-standing) behavior.
		if ( true ) //!GetNavigator()->IsGoalActive() || ( GetNavigator()->GetCurWaypointFlags() | bits_WP_TO_PATHCORNER ) )
		{
			// Select move target 
			if ( GetTarget() != NULL )
			{
				pMoveTarget = GetTarget();
			}
			else if ( GetEnemy() != NULL )
			{
				pMoveTarget = GetEnemy();
			}
			
			// Select move target position 
			if ( HaveInspectTarget() )
			{
				vMoveTargetPos = InspectTargetPosition(); 
			}
			else if ( GetEnemy() != NULL )
			{
				vMoveTargetPos = GetEnemy()->GetAbsOrigin();
			}
		}
		else
		{
			vMoveTargetPos = GetNavigator()->GetCurWaypointPos();
		}

		ClearCondition( COND_SCANNER_FLY_CLEAR );
		ClearCondition( COND_SCANNER_FLY_BLOCKED );

		// See if we can fly there directly
		if ( pMoveTarget || HaveInspectTarget() )
		{
			trace_t tr;
			AI_TraceHull( GetAbsOrigin(), vMoveTargetPos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

			float fTargetDist = (1.0f-tr.fraction)*(GetAbsOrigin() - vMoveTargetPos).Length();
			
			if ( ( tr.m_pEnt == pMoveTarget ) || ( fTargetDist < 50 ) )
			{
				if ( g_debug_cscanner.GetBool() )
				{
					NDebugOverlay::Line(GetLocalOrigin(), vMoveTargetPos, 0,255,0, true, 0);
					NDebugOverlay::Cross3D(tr.endpos,Vector(-5,-5,-5),Vector(5,5,5),0,255,0,true,0.1);
				}

				SetCondition( COND_SCANNER_FLY_CLEAR );
			}
			else		
			{
				//HANDY DEBUG TOOL	
				if ( g_debug_cscanner.GetBool() )
				{
					NDebugOverlay::Line(GetLocalOrigin(), vMoveTargetPos, 255,0,0, true, 0);
					NDebugOverlay::Cross3D(tr.endpos,Vector(-5,-5,-5),Vector(5,5,5),255,0,0,true,0.1);
				}

				SetCondition( COND_SCANNER_FLY_BLOCKED );
			}
		}

		// If I have a route, keep it updated and move toward target
		if ( GetNavigator()->IsGoalActive() )
		{
			if ( OverridePathMove( pMoveTarget, flInterval ) )
			{
				BlendPhyscannonLaunchSpeed();
				return true;
			}
		}	
		else if (m_nFlyMode == SCANNER_FLY_SPOT)
		{
			MoveToSpotlight( flInterval );
		}
		// If photographing
		else if ( m_nFlyMode == SCANNER_FLY_PHOTO )
		{
			MoveToPhotograph( flInterval );
		}
		else if ( m_nFlyMode == SCANNER_FLY_FOLLOW )
		{
			MoveToSpotlight( flInterval );
		}
		// ----------------------------------------------
		//	If attacking
		// ----------------------------------------------
		else if (m_nFlyMode == SCANNER_FLY_ATTACK)
		{
			if ( m_hSpotlight )
			{
				SpotlightDestroy();
			}
			
			MoveToAttack( flInterval );
		}
		// -----------------------------------------------------------------
		// If I don't have a route, just decelerate
		// -----------------------------------------------------------------
		else if (!GetNavigator()->IsGoalActive())
		{
			float	myDecay	 = 9.5;
			Decelerate( flInterval, myDecay);
		}
	}
		
	MoveExecute_Alive( flInterval );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Accelerates toward a given position.
// Input  : flInterval - Time interval over which to move.
//			vecMoveTarget - Position to move toward.
//-----------------------------------------------------------------------------
void CNPC_CScanner::MoveToTarget( float flInterval, const Vector &vecMoveTarget )
{
	// Don't move if stalling
	if ( m_flEngineStallTime > gpGlobals->curtime )
		return;
	
	// Look at our inspection target if we have one
	if ( GetEnemy() != NULL )
	{
		// Otherwise at our enemy
		TurnHeadToTarget( flInterval, GetEnemy()->EyePosition() );
	}
	else if ( HaveInspectTarget() )
	{
		TurnHeadToTarget( flInterval, InspectTargetPosition() );
	}
	else
	{
		// Otherwise face our motion direction
		TurnHeadToTarget( flInterval, vecMoveTarget );
	}

	// -------------------------------------
	// Move towards our target
	// -------------------------------------
	float myAccel;
	float myZAccel = 400.0f;
	float myDecay  = 0.15f;

	Vector vecCurrentDir;

	// Get the relationship between my current velocity and the way I want to be going.
	vecCurrentDir = GetCurrentVelocity();
	VectorNormalize( vecCurrentDir );

	Vector targetDir = vecMoveTarget - GetAbsOrigin();
	float flDist = VectorNormalize(targetDir);

	float flDot;
	flDot = DotProduct( targetDir, vecCurrentDir );

	if( flDot > 0.25 )
	{
		// If my target is in front of me, my flight model is a bit more accurate.
		myAccel = 250;
	}
	else
	{
		// Have a harder time correcting my course if I'm currently flying away from my target.
		myAccel = 128;
	}

	if ( myAccel > flDist / flInterval )
	{
		myAccel = flDist / flInterval;
	}

	if ( myZAccel > flDist / flInterval )
	{
		myZAccel = flDist / flInterval;
	}

	MoveInDirection( flInterval, targetDir, myAccel, myZAccel, myDecay );

	// calc relative banking targets
	Vector forward, right, up;
	GetVectors( &forward, &right, &up );

	m_vCurrentBanking.x	= targetDir.x;
	m_vCurrentBanking.z	= 120.0f * DotProduct( right, targetDir );
	m_vCurrentBanking.y	= 0;

	float speedPerc = SimpleSplineRemapVal( GetCurrentVelocity().Length(), 0.0f, GetMaxSpeed(), 0.0f, 1.0f );

	speedPerc = clamp( speedPerc, 0.0f, 1.0f );

	m_vCurrentBanking *= speedPerc;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::MoveToSpotlight( float flInterval )
{
	if ( flInterval <= 0 )
		return;

	Vector vTargetPos;

	if ( HaveInspectTarget() )
	{
		vTargetPos = InspectTargetPosition();
	}
	else if ( GetEnemy() != NULL )
	{
		vTargetPos = GetEnemyLKP();
	}
	else
	{
		return;
	}

	//float flDesiredDist = SCANNER_SPOTLIGHT_NEAR_DIST + ( ( SCANNER_SPOTLIGHT_FAR_DIST - SCANNER_SPOTLIGHT_NEAR_DIST ) / 2 );
	
	float flIdealHeightDiff = SCANNER_SPOTLIGHT_NEAR_DIST;
	if( IsEnemyPlayerInSuit() )
	{
		flIdealHeightDiff *= 0.5;
	}

	Vector idealPos = IdealGoalForMovement( vTargetPos, GetAbsOrigin(), GetGoalDistance(), flIdealHeightDiff );

	MoveToTarget( flInterval, idealPos );

	//TODO: Re-implement?

	/*
	// ------------------------------------------------
	//  Also keep my distance from other squad members
	//  unless I'm inspecting
	// ------------------------------------------------
	if (m_pSquad &&
		gpGlobals->curtime > m_fInspectEndTime)
	{
		CBaseEntity*	pNearest	= m_pSquad->NearestSquadMember(this);
		if (pNearest)
		{
			Vector			vNearestDir = (pNearest->GetLocalOrigin() - GetLocalOrigin());
			if (vNearestDir.Length() < SCANNER_SQUAD_FLY_DIST) 
			{
				vNearestDir		= pNearest->GetLocalOrigin() - GetLocalOrigin();
				VectorNormalize(vNearestDir);
				vFlyDirection  -= 0.5*vNearestDir;
			}
		}
	}

	// ---------------------------------------------------------
	//  Add evasion if I have taken damage recently
	// ---------------------------------------------------------
	if ((m_flLastDamageTime + SCANNER_EVADE_TIME) > gpGlobals->curtime)
	{
		vFlyDirection = vFlyDirection + VelocityToEvade(GetEnemyCombatCharacterPointer());
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CNPC_CScanner::GetGoalDistance( void )
{
	if ( m_flGoalOverrideDistance != 0.0f )
		return m_flGoalOverrideDistance;

	switch ( m_nFlyMode )
	{
	case SCANNER_FLY_PHOTO:
		return ( SCANNER_PHOTO_NEAR_DIST + ( ( SCANNER_PHOTO_FAR_DIST - SCANNER_PHOTO_NEAR_DIST ) / 2 ) );
		break;

	case SCANNER_FLY_SPOT:
		{
			float goalDist = ( SCANNER_SPOTLIGHT_NEAR_DIST + ( ( SCANNER_SPOTLIGHT_FAR_DIST - SCANNER_SPOTLIGHT_NEAR_DIST ) / 2 ) );
			if( IsEnemyPlayerInSuit() )
			{
				goalDist *= 0.5;
			}
			return goalDist;
		}
		break;
	
	case SCANNER_FLY_FOLLOW:
		return ( SCANNER_FOLLOW_DIST );
		break;
	}

	return BaseClass::GetGoalDistance();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_CScanner::MoveToPhotograph(float flInterval)
{
	if ( HaveInspectTarget() == false )
		return;

	//float flDesiredDist = SCANNER_PHOTO_NEAR_DIST + ( ( SCANNER_PHOTO_FAR_DIST - SCANNER_PHOTO_NEAR_DIST ) / 2 );
	
	Vector idealPos = IdealGoalForMovement( InspectTargetPosition(), GetAbsOrigin(), GetGoalDistance(), 32.0f );

	MoveToTarget( flInterval, idealPos );

	//FIXME: Re-implement?

	/*
	// ------------------------------------------------
	//  Also keep my distance from other squad members
	//  unless I'm inspecting
	// ------------------------------------------------
	if (m_pSquad &&
		gpGlobals->curtime > m_fInspectEndTime)
	{
		CBaseEntity*	pNearest	= m_pSquad->NearestSquadMember(this);
		if (pNearest)
		{
			Vector			vNearestDir = (pNearest->GetLocalOrigin() - GetLocalOrigin());
			if (vNearestDir.Length() < SCANNER_SQUAD_FLY_DIST) 
			{
				vNearestDir		= pNearest->GetLocalOrigin() - GetLocalOrigin();
				VectorNormalize(vNearestDir);
				vFlyDirection  -= 0.5*vNearestDir;
			}
		}
	}
	*/
}


//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CNPC_CScanner::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* pSourceEnt)
{
	//	TODO:: - doing this by just an interrupt contition would be a lot better!
	if (interactionType ==	g_interactionScannerSupportEntity)
	{
		// Only accept help request if I'm not already busy
		if (GetEnemy() == NULL && !HaveInspectTarget())
		{
			// Only accept if target is a reasonable distance away
			CBaseEntity* pTarget = (CBaseEntity*)data;
			float fTargetDist = (pTarget->GetLocalOrigin() - GetLocalOrigin()).Length();

			if (fTargetDist < SCANNER_SQUAD_HELP_DIST)
			{
				float fInspectTime = (((CNPC_CScanner*)pSourceEnt)->m_fInspectEndTime - gpGlobals->curtime);
				SetInspectTargetToEnt(pTarget,fInspectTime);

				if (random->RandomInt(0,2)==0)
				{
					SetSchedule(SCHED_CSCANNER_PHOTOGRAPH_HOVER);
				}
				else
				{
					SetSchedule(SCHED_CSCANNER_SPOTLIGHT_HOVER);
				}
				return true;
			}
		}
	}
	else if (interactionType ==	g_interactionScannerSupportPosition)
	{
		// Only accept help request if I'm not already busy
		if (GetEnemy() == NULL && !HaveInspectTarget())
		{
			// Only accept if target is a reasonable distance away
			Vector vInspectPos;
			vInspectPos.x = ((Vector *)data)->x;
			vInspectPos.y = ((Vector *)data)->y;
			vInspectPos.z = ((Vector *)data)->z;

			float fTargetDist = (vInspectPos - GetLocalOrigin()).Length();

			if (fTargetDist < SCANNER_SQUAD_HELP_DIST)
			{
				float fInspectTime = (((CNPC_CScanner*)pSourceEnt)->m_fInspectEndTime - gpGlobals->curtime);
				SetInspectTargetToPos(vInspectPos,fInspectTime);

				if (random->RandomInt(0,2)==0)
				{
					SetSchedule(SCHED_CSCANNER_PHOTOGRAPH_HOVER);
				}
				else
				{
					SetSchedule(SCHED_CSCANNER_SPOTLIGHT_HOVER);
				}
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputDisableSpotlight( inputdata_t &inputdata )
{
	m_bNoLight = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CNPC_CScanner::GetHeadTurnRate( void ) 
{ 
	if ( GetEnemy() )
		return 800.0f;

	if ( HaveInspectTarget() )
		return 500.0f;

	return BaseClass::GetHeadTurnRate();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputSetFollowTarget( inputdata_t &inputdata )
{
	InspectTarget( inputdata, SCANNER_FLY_FOLLOW );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CNPC_CScanner::InputClearFollowTarget( inputdata_t &inputdata )
{
	SetInspectTargetToEnt( NULL, 0 );
	
	m_nFlyMode = SCANNER_FLY_PATROL;
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_cscanner, CNPC_CScanner )
	DECLARE_TASK(TASK_CSCANNER_SET_FLY_PHOTO)
	DECLARE_TASK(TASK_CSCANNER_SET_FLY_SPOT)
	DECLARE_TASK(TASK_CSCANNER_PHOTOGRAPH)
	DECLARE_TASK(TASK_CSCANNER_ATTACK_PRE_FLASH)
	DECLARE_TASK(TASK_CSCANNER_ATTACK_FLASH)
	DECLARE_TASK(TASK_CSCANNER_SPOT_INSPECT_ON)
	DECLARE_TASK(TASK_CSCANNER_SPOT_INSPECT_WAIT)
	DECLARE_TASK(TASK_CSCANNER_SPOT_INSPECT_OFF)
	DECLARE_TASK(TASK_CSCANNER_CLEAR_INSPECT_TARGET)
	DECLARE_TASK(TASK_CSCANNER_GET_PATH_TO_INSPECT_TARGET)

	DECLARE_CONDITION(COND_CSCANNER_HAVE_INSPECT_TARGET)
	DECLARE_CONDITION(COND_CSCANNER_INSPECT_DONE)
	DECLARE_CONDITION(COND_CSCANNER_CAN_PHOTOGRAPH)
	DECLARE_CONDITION(COND_CSCANNER_SPOT_ON_TARGET)

	DECLARE_ACTIVITY(ACT_SCANNER_SMALL_FLINCH_ALERT)
	DECLARE_ACTIVITY(ACT_SCANNER_SMALL_FLINCH_COMBAT)
	DECLARE_ACTIVITY(ACT_SCANNER_INSPECT)
	DECLARE_ACTIVITY(ACT_SCANNER_WALK_ALERT)
	DECLARE_ACTIVITY(ACT_SCANNER_WALK_COMBAT)
	DECLARE_ACTIVITY(ACT_SCANNER_FLARE)
	DECLARE_ACTIVITY(ACT_SCANNER_RETRACT)
	DECLARE_ACTIVITY(ACT_SCANNER_FLARE_PRONGS)
	DECLARE_ACTIVITY(ACT_SCANNER_RETRACT_PRONGS)
	DECLARE_ACTIVITY(ACT_SCANNER_FLARE_START)

	DECLARE_ANIMEVENT( AE_SCANNER_CLOSED )

	DECLARE_INTERACTION(g_interactionScannerInspect)
	DECLARE_INTERACTION(g_interactionScannerInspectBegin)
	DECLARE_INTERACTION(g_interactionScannerInspectDone)
	DECLARE_INTERACTION(g_interactionScannerInspectHandsUp)
	DECLARE_INTERACTION(g_interactionScannerInspectShowArmband)
	DECLARE_INTERACTION(g_interactionScannerSupportEntity)
	DECLARE_INTERACTION(g_interactionScannerSupportPosition)

	//=========================================================
	// > SCHED_CSCANNER_PATROL
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_PATROL,

		"	Tasks"
		"		TASK_CSCANNER_CLEAR_INSPECT_TARGET	0"
		"		TASK_SCANNER_SET_FLY_PATROL			0"
		"		TASK_SET_TOLERANCE_DISTANCE			32"
		"		TASK_SET_ROUTE_SEARCH_TIME			5"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE		2000"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_SEE_FEAR"
		"		COND_HEAR_COMBAT"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_PLAYER"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_PROVOKED"
		"		COND_CSCANNER_HAVE_INSPECT_TARGET"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

	//=========================================================
	// > SCHED_CSCANNER_SPOTLIGHT_HOVER
	//
	// Hover above target entity, trying to get spotlight
	// on my target
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_SPOTLIGHT_HOVER,

		"	Tasks"
		"		TASK_CSCANNER_SET_FLY_SPOT			0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_WALK  "
		"		TASK_WAIT							1"
		""
		"	Interrupts"
		"		COND_CSCANNER_SPOT_ON_TARGET"
		"		COND_CSCANNER_INSPECT_DONE"
		"		COND_SCANNER_FLY_BLOCKED"
		"		COND_NEW_ENEMY"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

	//=========================================================
	// > SCHED_CSCANNER_SPOTLIGHT_INSPECT_POS
	//
	// Inspect a position once spotlight is on it
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_SPOTLIGHT_INSPECT_POS,

		"	Tasks"
		"		TASK_CSCANNER_SET_FLY_SPOT			0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_SCANNER_INSPECT"
		"		TASK_SPEAK_SENTENCE					3"	// Curious sound
		"		TASK_WAIT							5"
		"		TASK_CSCANNER_CLEAR_INSPECT_TARGET	0"
		""
		"	Interrupts"
		"		COND_CSCANNER_INSPECT_DONE"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
		"		COND_NEW_ENEMY"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

	//=========================================================
	// > SCHED_CSCANNER_SPOTLIGHT_INSPECT_CIT
	//
	// Inspect a citizen once spotlight is on it
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_SPOTLIGHT_INSPECT_CIT,

		"	Tasks"
		"		TASK_CSCANNER_SET_FLY_SPOT			0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_SCANNER_INSPECT"
		"		TASK_SPEAK_SENTENCE					0"	// Stop!
		"		TASK_WAIT							1"
		"		TASK_CSCANNER_SPOT_INSPECT_ON		0"
		"		TASK_WAIT							2"
		"		TASK_SPEAK_SENTENCE					1"	// Hands on head or Show Armband!
		"		TASK_WAIT							1"
		"		TASK_CSCANNER_SPOT_INSPECT_WAIT		0"
		"		TASK_WAIT							5"
		"		TASK_SPEAK_SENTENCE					2"	// Free to go!
		"		TASK_WAIT							1"
		"		TASK_CSCANNER_SPOT_INSPECT_OFF		0"
		"		TASK_CSCANNER_CLEAR_INSPECT_TARGET	0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

	//=========================================================
	// > SCHED_CSCANNER_PHOTOGRAPH_HOVER
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_PHOTOGRAPH_HOVER,

		"	Tasks"
		"		TASK_CSCANNER_SET_FLY_PHOTO			0"
		"		TASK_WAIT							2"
		""
		"	Interrupts"
		"		COND_CSCANNER_INSPECT_DONE"
		"		COND_CSCANNER_CAN_PHOTOGRAPH"
		"		COND_SCANNER_FLY_BLOCKED"
		"		COND_NEW_ENEMY"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

	//=========================================================
	// > SCHED_CSCANNER_PHOTOGRAPH
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_PHOTOGRAPH,

		"	Tasks"
		"		TASK_CSCANNER_SET_FLY_PHOTO			0"
		"		TASK_CSCANNER_PHOTOGRAPH				0"
		""
		"	Interrupts"
		"		COND_CSCANNER_INSPECT_DONE"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

	//=========================================================
	// > SCHED_CSCANNER_ATTACK_FLASH
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_ATTACK_FLASH,

		"	Tasks"
		"		TASK_SCANNER_SET_FLY_ATTACK			0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		"		TASK_CSCANNER_ATTACK_PRE_FLASH		0 "
		"		TASK_CSCANNER_ATTACK_FLASH			0"
		"		TASK_WAIT							0.5"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

	//=========================================================
	// > SCHED_CSCANNER_MOVE_TO_INSPECT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CSCANNER_MOVE_TO_INSPECT,

		"	Tasks"
		"		 TASK_SET_FAIL_SCHEDULE						SCHEDULE:SCHED_SCANNER_PATROL"
		"		 TASK_SET_TOLERANCE_DISTANCE				128"
		"		 TASK_CSCANNER_GET_PATH_TO_INSPECT_TARGET	0"
		"		 TASK_RUN_PATH								0"
		"		 TASK_WAIT_FOR_MOVEMENT						0"
		""
		"	Interrupts"
		"		COND_SCANNER_FLY_CLEAR"
		"		COND_NEW_ENEMY"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)
	
AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------
// Claw Scanner
//
// Scanner that always spawns as a claw scanner
//-----------------------------------------------------------------------------
	
class CNPC_ClawScanner : public CNPC_CScanner
{
DECLARE_CLASS( CNPC_ClawScanner, CNPC_CScanner );

public:
	CNPC_ClawScanner();
	DECLARE_DATADESC();
};

BEGIN_DATADESC( CNPC_ClawScanner )
END_DATADESC()


LINK_ENTITY_TO_CLASS(npc_clawscanner, CNPC_ClawScanner);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_ClawScanner::CNPC_ClawScanner()
{
	// override our superclass's setting
	BecomeClawScanner();
}
