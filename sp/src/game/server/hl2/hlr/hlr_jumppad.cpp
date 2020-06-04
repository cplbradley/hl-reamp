#include "cbase.h"
#include "soundent.h"

#define JUMPPAD_MODEL "models/props_combine/combine_mine01.mdl"
class CJumppad : public CBaseEntity
{
	DECLARE_CLASS(CJumppad, CBaseEntity);
public:
	void Spawn(void);
	void Precache(void);
	void Reenable(void);
	void SetForce(float flpushforce);
	void TouchThink(CBaseEntity *pOther);
	unsigned int PhysicsSolidMaskForEntity() const;
	float m_flpushforce;
DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(hlr_jumppad, CJumppad);
BEGIN_DATADESC(CJumppad)
DEFINE_KEYFIELD(m_flpushforce, FIELD_FLOAT, "pushforce"),
// Function Pointers
DEFINE_FUNCTION(TouchThink),
END_DATADESC()
unsigned int CJumppad::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CJumppad::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(20.0f, 20.0f, 20.0f), Vector(20.0f, 20.0f, 20.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	//SetModel(JUMPPAD_MODEL);
	SetTouch(&CJumppad::TouchThink);
}
void CJumppad::Precache(void)
{
	PrecacheModel(JUMPPAD_MODEL);
	PrecacheScriptSound("Weapon_Mortar.Single");
}
void CJumppad::SetForce(float flpushforce)
{
	m_flpushforce = flpushforce;
}
void CJumppad::TouchThink(CBaseEntity *pOther) //something touched me
{
	QAngle angSrc = this->GetAbsAngles(); //get angles from hammer
	Vector vecDir;
	Vector vecdnspd;
	Vector veccurvel;
	float fldnspd;
	float flpunchforce;
	AngleVectors(angSrc, &vecDir); //transform angles into a vector
	vecdnspd = pOther->GetAbsVelocity(); //get velocity vector
	fldnspd = vecdnspd[2]; //isolate y value of said vector
	if (m_flpushforce == NULL) //if no value is set in hammer
	{
		m_flpushforce = 1000;
	}
	flpunchforce = (m_flpushforce + abs(fldnspd)); //calculate punch force by adding abs of downward velocity to defined push force
	if (pOther->IsSolid()) //was the thing that touched me solid?
	{
		veccurvel = pOther->GetAbsVelocity(); //whats its current velocity
		veccurvel[2] += flpunchforce; //add the punch force to the y value of the current velocity
		pOther->SetAbsVelocity(veccurvel); //apply the new velocity
		//pOther->VelocityPunch(vecDir * flpunchforce); //do punch
		SetTouch(NULL); //disable temporarily
		SetThink(&CJumppad::Reenable);//schedule re-enable
		SetNextThink(gpGlobals->curtime + 0.1f);//after 0.1 seconds
		EmitSound("Weapon_Mortar.Single");//emit sound
	}

}
void CJumppad::Reenable(void)
{
	SetTouch(&CJumppad::TouchThink);//re-enable
}
///
///
///
///
class CLaunchpad : public CBaseEntity
{
	DECLARE_CLASS(CLaunchpad, CBaseEntity);
public:
	void Spawn(void);
	void Precache(void);
	void Reenable(void);
	void SetForce(float flpushforce);
	void TouchThink(CBaseEntity *pOther);
	unsigned int PhysicsSolidMaskForEntity() const;
	float m_flpushforce;
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(hlr_launchpad, CLaunchpad);
BEGIN_DATADESC(CLaunchpad)
DEFINE_KEYFIELD(m_flpushforce, FIELD_FLOAT, "pushforce"),
// Function Pointers
DEFINE_FUNCTION(TouchThink),
END_DATADESC()
unsigned int CLaunchpad::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}
void CLaunchpad::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(20.0f, 20.0f, 20.0f), Vector(20.0f, 20.0f, 20.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	//SetModel(JUMPPAD_MODEL);
	SetTouch(&CLaunchpad::TouchThink);
}
void CLaunchpad::Precache(void)
{
	PrecacheModel(JUMPPAD_MODEL);
	PrecacheScriptSound("Weapon_Mortar.Single");
}
void CLaunchpad::SetForce(float flpushforce)
{
	m_flpushforce = flpushforce;
}
void CLaunchpad::TouchThink(CBaseEntity *pOther) //something touched me
{
	QAngle angSrc = this->GetAbsAngles(); //get angles from hammer
	Vector vecDir;
	Vector vecdnspd;
	Vector veccurvel;
	float fldnspd;
	float flpunchforce;
	AngleVectors(angSrc, &vecDir); //transform angles into a vector
	vecdnspd = pOther->GetAbsVelocity(); //get velocity vector
	fldnspd = vecdnspd[2]; //isolate y value of said vector
	if (m_flpushforce == NULL) //if no value is set in hammer
	{
		m_flpushforce = 1000;
	}
	flpunchforce = (m_flpushforce + abs(fldnspd)); //calculate punch force by adding abs of downward velocity to defined push force
	if (pOther->IsSolid()) //was the thing that touched me solid?
	{
		pOther->VelocityPunch(vecDir * m_flpushforce);
		//pOther->VelocityPunch(vecDir * flpunchforce); //do punch
		SetTouch(NULL); //disable temporarily
		SetThink(&CLaunchpad::Reenable);//schedule re-enable
		SetNextThink(gpGlobals->curtime + 0.1f);//after 0.1 seconds
		EmitSound("Weapon_Mortar.Single");//emit sound
	}

}
void CLaunchpad::Reenable(void)
{
	SetTouch(&CLaunchpad::TouchThink);//re-enable
}