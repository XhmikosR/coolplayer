/*
 * CoolPlayer - Blazing fast audio player.
 * Copyright (C) 2000-2001 Niek Albers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////
//
// Cooler output module
//
////////////////////////////////////////////////////////////////////////////////


// Forward reference
struct _CPs_OutputModule;

////////////////////////////////////////////////////////////////////////////////
// Output functions

typedef struct _CPs_OutputModule* CP_HOUTPUTMODULE;
typedef void (*pfnOP_Initialise)(CP_HOUTPUTMODULE pModule, const CPs_FileInfo* pFileInfo, CP_HEQUALISER hEqualiser);
typedef void (*pfnOP_Uninitialise)(CP_HOUTPUTMODULE pModule);
typedef void (*pfnOP_RefillBuffers)(CP_HOUTPUTMODULE pModule);
typedef void (*pfnOP_SetPause)(CP_HOUTPUTMODULE pModule, const BOOL bPause);
typedef BOOL (*pfnOP_IsOutputComplete)(CP_HOUTPUTMODULE pModule);
typedef void (*pfnOP_Flush)(CP_HOUTPUTMODULE pModule);
typedef void (*pfnOP_OnEQChanged)(CP_HOUTPUTMODULE pModule);
typedef void (*pfnOP_SetInternalVolume)(CP_HOUTPUTMODULE pModule, const int iNewInternalVolume);
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Output module

typedef struct _CPs_OutputModule
{
	// Public functions
	pfnOP_Initialise Initialise;
	pfnOP_Uninitialise Uninitialise;
	pfnOP_RefillBuffers RefillBuffers;
	pfnOP_SetPause SetPause;
	pfnOP_IsOutputComplete IsOutputComplete;
	pfnOP_Flush Flush;
	pfnOP_OnEQChanged OnEQChanged;
	pfnOP_SetInternalVolume SetInternalVolume;
	
	// Public variables
	CPs_CoDecModule* m_pCoDec;  // NULL when the stream is exhausted
	HANDLE m_evtBlockFree;
	const char* m_pcModuleName;
	CP_HEQUALISER m_pEqualiser;
	
	// Private variables
	void* m_pModuleCookie;  // This is a pointer to any private data the module may want to maintain
	
} CPs_OutputModule;

//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Output initialisers
void CPI_Player_Output_Initialise_WaveMapper(CPs_OutputModule* pModule);
void CPI_Player_Output_Initialise_DirectSound(CPs_OutputModule* pModule);
void CPI_Player_Output_Initialise_File(CPs_OutputModule* pModule);

////////////////////////////////////////////////////////////////////////////////
