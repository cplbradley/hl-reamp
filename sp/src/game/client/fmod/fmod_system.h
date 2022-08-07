#pragma once
#ifndef FMOD_SYSTEM_H
#define FMOD_SYSTEM_H


#include "fmod.hpp"

class CFMODSystem
{
public:
	CFMODSystem();
	//~CFMODSystem();

	void InitFMOD();
	void ExitFMOD();

};

extern CFMODSystem* FMODSystem();

#endif //FMOD_SYSTEM_H