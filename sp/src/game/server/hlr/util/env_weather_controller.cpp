#include "cbase.h"
#include "effects.h"
#include "tier0/memdbgon.h"


class CEnvWeatherController : public CPointEntity
{
	DECLARE_CLASS(CEnvWeatherController, CPointEntity);

public:
	void Spawn();
	void ApplySpawnSettings();
	void SetRainLength(float length);
	void SetRainWidth(float width);
	void SetRainSpeed(float speed);
	void SetRainSideVel(float vel);
	void SetRainRadius(float radius);
	void SetRainDensity(float density);
	void SetWindSpeed(int windspeed);
	void SetWindDirection(int winddir);


	void OnRestore();


	

private:
	void InputSetRainLength(inputdata_t& inputdata);
	void InputSetRainWidth(inputdata_t& inputdata);
	void InputSetRainSpeed(inputdata_t& inputdata);
	void InputSetRainSideVel(inputdata_t& inputdata);
	void InputSetRainRadius(inputdata_t& inputdata);
	void InputSetRainDensity(inputdata_t& inputdata);
	void InputSetPrecipitationType(inputdata_t& inputdata);
	void InputSetWindSpeed(inputdata_t& inputdata);
	void InputSetWindDir(inputdata_t& inputdata);

	float fLength;
	float fWidth;
	float fSpeed;
	float fVel;
	float fRadius;
	float fDensity;
	int iType;
	int iWindSpeed;
	int iWindDir;

	DECLARE_DATADESC();

};

LINK_ENTITY_TO_CLASS(env_weather_controller,CEnvWeatherController)

BEGIN_DATADESC(CEnvWeatherController)

DEFINE_KEYFIELD(fLength,FIELD_FLOAT,"rainlength"),
DEFINE_KEYFIELD(fWidth, FIELD_FLOAT, "rainwidth"),
DEFINE_KEYFIELD(fSpeed, FIELD_FLOAT, "rainspeed"),
DEFINE_KEYFIELD(fVel, FIELD_FLOAT, "rainsidevel"),
DEFINE_KEYFIELD(fRadius, FIELD_FLOAT, "raindrawradius"),
DEFINE_KEYFIELD(fDensity, FIELD_FLOAT, "raindensity"),
DEFINE_KEYFIELD(iWindSpeed,FIELD_INTEGER,"windspeed"),
DEFINE_KEYFIELD(iWindDir,FIELD_INTEGER,"winddir"),
DEFINE_INPUTFUNC(FIELD_FLOAT,"SetRainLength", InputSetRainLength),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetRainWidth", InputSetRainWidth),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetRainSpeed", InputSetRainSpeed),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetRainRandomVelocity", InputSetRainSideVel),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetRainDrawRadius", InputSetRainRadius),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetRainDensity", InputSetRainDensity),
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetPrecipitationType", InputSetPrecipitationType),
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetWindSpeed", InputSetWindSpeed),
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetWindDirection",InputSetWindDir),
END_DATADESC()


void CEnvWeatherController::Spawn()
{
	ApplySpawnSettings();
}
void CEnvWeatherController::ApplySpawnSettings()
{
	SetRainLength(fLength);
	SetRainWidth(fWidth);
	SetRainSpeed(fSpeed);
	SetRainSideVel(fVel);
	SetRainRadius(fRadius);
	SetRainDensity(fDensity);
	SetWindSpeed(iWindSpeed);
	SetWindDirection(iWindDir);
}
void CEnvWeatherController::OnRestore()
{
	BaseClass::OnRestore();
	//ApplySpawnSettings();
}
void CEnvWeatherController::InputSetRainLength(inputdata_t& inputdata)
{
	SetRainLength(inputdata.value.Float());
}
void CEnvWeatherController::SetRainLength(float length)
{
	ConVarRef rainlength("r_rainlength");
	rainlength.SetValue(length);
	fLength = length;
}
void CEnvWeatherController::InputSetRainWidth(inputdata_t& inputdata)
{
	SetRainWidth(inputdata.value.Float());
}
void CEnvWeatherController::SetRainWidth(float width)
{
	ConVarRef rainwidth("r_rainwidth");
	fWidth = width;
	rainwidth.SetValue(width);
}
void CEnvWeatherController::InputSetRainSpeed(inputdata_t& inputdata)
{
	SetRainSpeed(inputdata.value.Float());
}
void CEnvWeatherController::SetRainSpeed(float speed)
{
	ConVarRef rainspeed("r_rainspeed");
	fSpeed = speed;
	rainspeed.SetValue(speed);
}
void CEnvWeatherController::InputSetRainSideVel(inputdata_t& inputdata)
{
	SetRainSideVel(inputdata.value.Float());
}
void CEnvWeatherController::SetRainSideVel(float vel)
{
	ConVarRef rainsidevel("r_rainsidevel");
	fVel = vel;
	rainsidevel.SetValue(vel);
}
void CEnvWeatherController::InputSetRainRadius(inputdata_t& inputdata)
{
	SetRainRadius(inputdata.value.Float());
}
void CEnvWeatherController::SetRainRadius(float radius)
{
	ConVarRef rainradius("r_rainradius");
	fRadius = radius;
	rainradius.SetValue(radius);
}
void CEnvWeatherController::InputSetRainDensity(inputdata_t& inputdata)
{
	SetRainDensity(inputdata.value.Float());
}
void CEnvWeatherController::SetRainDensity(float density)
{
	CBaseEntity* pStart = NULL;

	while ((pStart = gEntList.FindEntityByClassname(pStart, "func_precipitation")) != NULL)
	{
		pStart->SetRenderColorA(density);
	}
	fDensity = density;
}
void CEnvWeatherController::InputSetPrecipitationType(inputdata_t& inputdata)
{
	CBaseEntity* pStart = NULL;

	while ((pStart = gEntList.FindEntityByClassname(pStart, "func_precipitation")) != NULL)
	{
		CPrecipitation* precip = assert_cast<CPrecipitation*>(pStart);

		if (precip)
			precip->m_nPrecipType = inputdata.value.Int();
	}
}

void CEnvWeatherController::InputSetWindSpeed(inputdata_t& inputdata)
{
	SetWindSpeed(inputdata.value.Int());
}
void CEnvWeatherController::SetWindSpeed(int speed)
{
	ConVarRef windspeed("cl_windspeed");
	iWindSpeed = speed;
	windspeed.SetValue(speed);
}

void CEnvWeatherController::InputSetWindDir(inputdata_t& inputdata)
{
	SetWindDirection(inputdata.value.Int());
}
void CEnvWeatherController::SetWindDirection(int dir)
{
	ConVarRef winddir("cl_winddir");
	iWindDir = dir;
	winddir.SetValue(dir);
}