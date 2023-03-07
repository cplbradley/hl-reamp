#pragma once
#ifndef FMOD_SYSTEM_H
#define FMOD_SYSTEM_H


#include "fmod.hpp"
#include "cbase.h"

class CFMODSystem
{
public:
	CFMODSystem();
	//~CFMODSystem();

	void InitFMOD();
	void ExitFMOD();
	int DynamicLoop();

};

extern CFMODSystem* FMODSystem();

#endif //FMOD_SYSTEM_H