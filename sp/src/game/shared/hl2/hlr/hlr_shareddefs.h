#include "cbase.h"
#include "filesystem.h"

#ifdef CLIENT_DLL
#include "c_basehlplayer.h"
#endif


#ifndef HLR_SHAREDDEFS_H
#define HLR_SHAREDDEFS_H


#ifdef _WIN32
#pragma once
#endif


static ConVar g_ultragibs("g_ultragibs", "0", FCVAR_REPLICATED | FCVAR_GAMEDLL | FCVAR_CLIENTDLL, "ultragibs");
static ConVar g_guts_and_glory("g_guts_and_glory", "0", FCVAR_REPLICATED | FCVAR_GAMEDLL | FCVAR_CLIENTDLL, "Guts and Glory!");
static ConVar g_masochist_mode("g_masochist_mode", "0", FCVAR_NONE, "WARNING: Death will delete your entire save folder.");
static ConVar g_classic_weapon_pos("g_classic_weapon_pos", "0", FCVAR_REPLICATED | FCVAR_GAMEDLL | FCVAR_CLIENTDLL, "Classic Weapon Positions");
static ConVar g_thirdperson("g_thirdperson", "0", FCVAR_REPLICATED | FCVAR_GAMEDLL | FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar hud_weapon_selection_slowmo("hud_weapon_selection_slowmo", "1", FCVAR_REPLICATED | FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Slowmo during weapon selection");
static ConVar g_draw_fury_effects("g_draw_fury_effects", "1", FCVAR_REPLICATED | FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Draws fury screen effects");
static ConVar g_rocket_jump_mania("g_rocket_jump_mania", "0", FCVAR_REPLICATED | FCVAR_GAMEDLL | FCVAR_ARCHIVE, "No explosive self-damage. Rocket-jump away!");
static ConVar mat_blurdarken("mat_blurdarken", "0");
static ConVar g_thirdperson_aimmode("g_thirdperson_aimmode", "0", FCVAR_REPLICATED | FCVAR_GAMEDLL | FCVAR_ARCHIVE, "Sets third-person aiming mode. 0 = Crosshair follows aim, 1 = aim follows crosshair");

static ConVar mat_taa("mat_taa", "0", FCVAR_REPLICATED | FCVAR_CLIENTDLL, "");
static ConVar g_custom_nightvision("g_custom_nightvision", "0", FCVAR_ARCHIVE | FCVAR_CLIENTDLL | FCVAR_GAMEDLL);
static ConVar mat_glitch("mat_glitch", "0", FCVAR_REPLICATED | FCVAR_GAMEDLL | FCVAR_CLIENTDLL, "");

static ConVar r_efficient_particles("r_efficient_particles", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED | FCVAR_CLIENTDLL | FCVAR_GAMEDLL);


static float easeInOut(float t) {
	return t < 0.5f ? 0.5f * pow(2 * t, 2.0f) : 0.5f * (2 - pow(2 * (1 - t), 2.0f));
}

static void CC_TestKillSave(void)
{
	FileFindHandle_t fhsav;
	FileFindHandle_t fhhl1;
	FileFindHandle_t fhhl2;
	FileFindHandle_t fhhl3;
	if (const char *pszFileName = g_pFullFileSystem->FindFirstEx("save/*.sav", "MOD", &fhsav))
	{
		char szFileExt[4];
		char szFullFileName[MAX_PATH];
		do
		{
			if (pszFileName[0] != '.')
			{
				V_ExtractFileExtension(pszFileName, szFileExt, sizeof(szFileExt));
				if (!V_stricmp(szFileExt, "sav"))
				{
					V_strcpy_safe(szFullFileName, "save/");
					V_strcat_safe(szFullFileName, pszFileName);
					g_pFullFileSystem->RemoveFile(szFullFileName, "MOD");
				}
			}

			pszFileName = g_pFullFileSystem->FindNext(fhsav);
		} while (pszFileName);

		g_pFullFileSystem->FindClose(fhsav);
	}
	if (const char *pszFileName = g_pFullFileSystem->FindFirstEx("save/*.hl1", "MOD", &fhhl1))
	{
		char szFileExt[4];
		char szFullFileName[MAX_PATH];
		do
		{
			if (pszFileName[0] != '.')
			{
				V_ExtractFileExtension(pszFileName, szFileExt, sizeof(szFileExt));
				if (!V_stricmp(szFileExt, "hl1"))
				{
					V_strcpy_safe(szFullFileName, "save/");
					V_strcat_safe(szFullFileName, pszFileName);
					g_pFullFileSystem->RemoveFile(szFullFileName, "MOD");
				}
			}

			pszFileName = g_pFullFileSystem->FindNext(fhhl1);
		} while (pszFileName);

		g_pFullFileSystem->FindClose(fhhl1);
	}
	if (const char *pszFileName = g_pFullFileSystem->FindFirstEx("save/*.hl2", "MOD", &fhhl2))
	{
		char szFileExt[4];
		char szFullFileName[MAX_PATH];
		do
		{
			if (pszFileName[0] != '.')
			{
				V_ExtractFileExtension(pszFileName, szFileExt, sizeof(szFileExt));
				if (!V_stricmp(szFileExt, "hl2"))
				{
					V_strcpy_safe(szFullFileName, "save/");
					V_strcat_safe(szFullFileName, pszFileName);
					g_pFullFileSystem->RemoveFile(szFullFileName, "MOD");
				}
			}

			pszFileName = g_pFullFileSystem->FindNext(fhhl2);
		} while (pszFileName);

		g_pFullFileSystem->FindClose(fhhl2);
	}
	if (const char *pszFileName = g_pFullFileSystem->FindFirstEx("save/*.hl3", "MOD", &fhhl3))
	{
		char szFileExt[4];
		char szFullFileName[MAX_PATH];
		do
		{
			if (pszFileName[0] != '.')
			{
				V_ExtractFileExtension(pszFileName, szFileExt, sizeof(szFileExt));
				if (!V_stricmp(szFileExt, "hl3"))
				{
					V_strcpy_safe(szFullFileName, "save/");
					V_strcat_safe(szFullFileName, pszFileName);
					g_pFullFileSystem->RemoveFile(szFullFileName, "MOD");
				}
			}

			pszFileName = g_pFullFileSystem->FindNext(fhhl3);
		} while (pszFileName);

		g_pFullFileSystem->FindClose(fhhl3);
	}
}




static ConCommand testkillsave("testkillsave", CC_TestKillSave, "testkillsave", FCVAR_REPLICATED | FCVAR_HIDDEN | FCVAR_CLIENTCMD_CAN_EXECUTE);

#endif