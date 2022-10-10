//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_frag.h"
#include "basegrenade_shared.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_TIMER	3.0f //Seconds

#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f // inches

ConVar sk_plr_grenade_launch_speed("sk_plr_grenade_launch_speed", "900", FCVAR_REPLICATED);
ConVar sk_plr_grenade_vert_factor("sk_plr_grenade_vert_factor", "250", FCVAR_REPLICATED);

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponFrag: public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponFrag, CBaseHLCombatWeapon );
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	CWeaponFrag();

	void	Precache( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );


	void	WeaponIdle(void);

	Activity		GetPrimaryAttackActivity(void) override;
	Activity		GetIdleActivity(void);

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	
	bool	Reload( void );

	bool	ShouldDisplayHUDHint() { return true; }

	int m_iLastFireAct;

private:
	void	ThrowGrenade( CBasePlayer *pPlayer );
	void	RollGrenade( CBasePlayer *pPlayer );
	void	LobGrenade( CBasePlayer *pPlayer );
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	bool	m_bRedraw;	//Draw the weapon again after throwing a grenade
	
	int		m_AttackPaused;
	bool	m_fDrawbackFinished;
protected:
	CHandle<CBaseGrenade> m_Grenada;
	DECLARE_ACTTABLE();


};


BEGIN_DATADESC( CWeaponFrag )
	DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_Grenada, FIELD_EHANDLE ),
	DEFINE_FIELD( m_AttackPaused, FIELD_INTEGER ),
	DEFINE_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN ),
END_DATADESC()

acttable_t	CWeaponFrag::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HLR_IDLE_GRENADELAUNCHER, false },
	{ ACT_HL2MP_RUN, ACT_HLR_RUN_GRENADELAUNCHER, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_AR2, false },
};

IMPLEMENT_ACTTABLE(CWeaponFrag);


LINK_ENTITY_TO_CLASS( weapon_frag, CWeaponFrag );
PRECACHE_WEAPON_REGISTER(weapon_frag);

IMPLEMENT_SERVERCLASS_ST(CWeaponFrag, DT_WeaponFrag)
END_SEND_TABLE()


CWeaponFrag::CWeaponFrag() :
	CBaseHLCombatWeapon(),
	m_bRedraw( false )
{
	NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther( "npc_grenade_frag" );

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponFrag::Deploy( void )
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponFrag::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	m_iLastFireAct = 0;

	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponFrag::WeaponIdle(void)
{
	//Idle again if we've finished
	if (HasWeaponIdleTimeElapsed())
	{
		SendWeaponAnim(GetIdleActivity());
	}
}

Activity CWeaponFrag::GetPrimaryAttackActivity(void)
{
	switch (m_iLastFireAct)
	{
	case 0:
	{
		m_iLastFireAct = 1;
		DevMsg("Sending Primary Attack Activity # %i\n", ACT_VM_GRENADELAUNCHER_FIRE1);
		return ACT_VM_GRENADELAUNCHER_FIRE1;
	}
	case 1:
	{
		m_iLastFireAct = 2;
		DevMsg("Sending Primary Attack Activity # %i\n", ACT_VM_GRENADELAUNCHER_FIRE2);
		return ACT_VM_GRENADELAUNCHER_FIRE2;
	}
	case 2:
	{
		m_iLastFireAct = 3;
		DevMsg("Sending Primary Attack Activity # %i\n", ACT_VM_GRENADELAUNCHER_FIRE3);
		return ACT_VM_GRENADELAUNCHER_FIRE3;
	}
	case 3:
	{
		m_iLastFireAct = 0;
		DevMsg("Sending Primary Attack Activity # %i\n", ACT_VM_GRENADELAUNCHER_FIRE4);
		return ACT_VM_GRENADELAUNCHER_FIRE4;
	}
	default:
		return ACT_VM_GRENADELAUNCHER_FIRE1;
	}
}
Activity CWeaponFrag::GetIdleActivity(void)
{
	switch (m_iLastFireAct)
	{
	case 0:
		return ACT_VM_GRENADELAUNCHER_IDLE0;
	case 1:
		return ACT_VM_GRENADELAUNCHER_IDLE1;
	case 2:
		return ACT_VM_GRENADELAUNCHER_IDLE2;
	case 3:
		return ACT_VM_GRENADELAUNCHER_IDLE3;
	default:
		return ACT_VM_GRENADELAUNCHER_IDLE0;
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponFrag::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	bool fThrewGrenade = false;

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_fDrawbackFinished = true;
			break;

		case EVENT_WEAPON_THROW:
			ThrowGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW2:
			RollGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW3:
			LobGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}

#define RETHROW_DELAY	0.5
	if( fThrewGrenade )
	{
		m_flNextPrimaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!

		// Make a sound designed to scare snipers back into their holes!
		CBaseCombatCharacter *pOwner = GetOwner();

		if( pOwner )
		{
			Vector vecSrc = pOwner->Weapon_ShootPosition();
			Vector	vecDir;

			AngleVectors( pOwner->EyeAngles(), &vecDir );

			trace_t tr;

			UTIL_TraceLine( vecSrc, vecSrc + vecDir * 1024, MASK_SOLID_BRUSHONLY, pOwner, COLLISION_GROUP_NONE, &tr );

			CSoundEnt::InsertSound( SOUND_DANGER_SNIPERONLY, tr.endpos, 384, 0.2, pOwner );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponFrag::Reload( void )
{
	if ( !HasPrimaryAmmo() )
		return false;

	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight, vUp;
	Vector vecAng;
	pPlayer->EyeVectors(&vecAng);
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);
	QAngle qEyeAng = pPlayer->EyeAngles();
	Vector	muzzlePoint = pPlayer->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector(0, 0, -8);
	//CheckThrowPosition(pPlayer, vecEye, vecSrc);

	float vertfactor = sk_plr_grenade_vert_factor.GetFloat();
	Vector vecThrow = vecAng * sk_plr_grenade_launch_speed.GetFloat() + Vector(0, 0, vertfactor);
	Fraggrenade_Create(muzzlePoint, vec3_angle, vecThrow, AngularImpulse(random->RandomInt(-600, 600), random->RandomInt(-600, 600),0), pPlayer, 3.0f, false);
	
	if (pPlayer->HasOverdrive())
	{
		for (int i = 0; i < 2; i++)
		{
			if (i == 0)
			{
				QAngle qAng = QAngle(qEyeAng[0] + 10, qEyeAng[1] + 10, qEyeAng[2]);
				Vector vecAimAng;
				AngleVectors(qAng, &vecAimAng);
				Vector vecThrow = vecAimAng * sk_plr_grenade_launch_speed.GetFloat() + Vector(0, 0, vertfactor);
				Fraggrenade_Create(muzzlePoint, vec3_angle, vecThrow, AngularImpulse(200, random->RandomInt(-600, 600), 0), pPlayer, 3.0f, false);
			}
			if (i == 1)
			{
				QAngle qAng2 = QAngle(qEyeAng[0] + 10, qEyeAng[1] - 10, qEyeAng[2]);
				Vector vecAimAng;
				AngleVectors(qAng2, &vecAimAng);
				Vector vecThrow = vecAimAng * sk_plr_grenade_launch_speed.GetFloat() + Vector(0, 0, vertfactor);
				Fraggrenade_Create(muzzlePoint, vec3_angle, vecThrow, AngularImpulse(200, random->RandomInt(-600, 600), 0), pPlayer, 3.0f, false);
			}
		}
	}
	
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.4f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.4f;

	WeaponSound(SINGLE);
	pPlayer->SetAnimation(PLAYER_ATTACK1);
	DevMsg("Should Be Sending Attack Activity\n");
	SendWeaponAnim(GetPrimaryAttackActivity());

	//m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	DecrementAmmo(GetOwner());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponFrag::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::SecondaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight, vUp;
	Vector vecAng;
	pPlayer->EyeVectors(&vecAng);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector(0, 0, -8);

	Vector	muzzlePoint = pPlayer->Weapon_ShootPosition();
	//CheckThrowPosition(pPlayer, vecEye, vecSrc);

	float vertfactor = sk_plr_grenade_vert_factor.GetFloat();
	Vector vecThrow = vecAng * sk_plr_grenade_launch_speed.GetFloat() + Vector(0, 0, vertfactor);
	Stickynade_Create(muzzlePoint, vec3_angle, vecThrow, AngularImpulse(200, random->RandomInt(-600, 600), 0), pPlayer);

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.4f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 2.0f;

	WeaponSound(WPN_DOUBLE);
	SendWeaponAnim(GetPrimaryAttackActivity());

	//m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	DecrementAmmo(GetOwner());
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFrag::ItemPostFrame( void )
{
	if( m_fDrawbackFinished )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		if (pOwner)
		{
			switch( m_AttackPaused )
			{
			case GRENADE_PAUSED_PRIMARY:
				if( !(pOwner->m_nButtons & IN_ATTACK) )
				{
					SendWeaponAnim( ACT_VM_THROW );
					m_fDrawbackFinished = false;
				}
				break;

			case GRENADE_PAUSED_SECONDARY:
				if( !(pOwner->m_nButtons & IN_ATTACK2) )
				{
					//See if we're ducking
					if ( pOwner->m_nButtons & IN_DUCK )
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_SECONDARYATTACK );
					}
					else
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_HAULBACK );
					}

					m_fDrawbackFinished = false;
				}
				break;

			default:
				break;
			}
		}
	}

	BaseClass::ItemPostFrame();

	if ( m_bRedraw )
	{
		if ( IsViewModelSequenceFinished() )
		{
			Reload();
		}
	}
}

	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponFrag::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFrag::ThrowGrenade( CBasePlayer *pPlayer )
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 500;
	Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, GRENADE_TIMER, false );

	m_bRedraw = true;

	WeaponSound( SINGLE );

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFrag::LobGrenade( CBasePlayer *pPlayer )
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector( 0, 0, -8 );
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	
	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 350 + Vector( 0, 0, 50 );
	Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(200,random->RandomInt(-600,600),0), pPlayer, GRENADE_TIMER, false );

	WeaponSound( WPN_DOUBLE );

	m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFrag::RollGrenade( CBasePlayer *pPlayer )
{
	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &vecSrc );
	vecSrc.z += GRENADE_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D( );
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize( vecFacing );
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc - Vector(0,0,16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct( vecFacing, tr.plane.normal, tangent );
		CrossProduct( tr.plane.normal, tangent, vecFacing );
	}
	vecSrc += (vecFacing * 18.0);
	CheckThrowPosition( pPlayer, pPlayer->WorldSpaceCenter(), vecSrc );

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vecFacing * 700;
	// put it on its side
	QAngle orientation(0,pPlayer->GetLocalAngles().y,-90);
	// roll it
	AngularImpulse rotSpeed(0,0,720);
	Fraggrenade_Create( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER, false );

	WeaponSound( SPECIAL1 );

	m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

