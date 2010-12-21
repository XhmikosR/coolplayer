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
// Cooler CoDec functions
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

typedef struct _CPs_CoDecOptions
{
	// public options
	int m_iPretendOption;
	
} CPs_CoDecOptions;

//
//

// Forward reference
struct _CPs_CoDecModule;

////////////////////////////////////////////////////////////////////////////////
// CoDec functions

typedef struct _CPs_CoDecModule* CP_HCODECMODULE;
typedef void (*pfn_Uninitialise)(CP_HCODECMODULE hCoDec);
//
typedef BOOL (*pfn_OpenFile)(CP_HCODECMODULE hCoDec, const char* pcFilename, DWORD dwCookie, HWND hWndOwner);
typedef void (*pfn_CloseFile)(CP_HCODECMODULE hCoDec);
typedef void (*pfn_Seek)(CP_HCODECMODULE hCoDec, const int iNumerator, const int iDenominator);
typedef void (*pfn_GetFileInfo)(CP_HCODECMODULE hCoDec, CPs_FileInfo* pInfo);
//
typedef BOOL (*pfn_GetPCMBlock)(CP_HCODECMODULE hCoDec, void* pBlock, DWORD* pdwBlockSize);
typedef int (*pfn_GetCurrentPos_secs)(CP_HCODECMODULE hCoDec);
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// CoDec module

typedef struct _CPs_CoDecModule
{
	// Public functions
	pfn_Uninitialise Uninitialise;
	
	pfn_OpenFile OpenFile;
	pfn_CloseFile CloseFile;
	pfn_Seek Seek;
	pfn_GetFileInfo GetFileInfo;
	
	pfn_GetPCMBlock GetPCMBlock;
	pfn_GetCurrentPos_secs GetCurrentPos_secs;
	
	// Public variables
	void* m_pModuleCookie;  // This is a pointer to any private data the module may want to maintain
	
	// Private variables
	void* m_pFileAssociationCookie;
} CPs_CoDecModule;

//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// File association helper functions
void CPFA_InitialiseFileAssociations(CPs_CoDecModule* pCoDec);
void CPFA_EmptyFileAssociations(CPs_CoDecModule* pCoDec);
void CPFA_AddFileAssociation(CPs_CoDecModule* pCoDec, const char* pcExtension, DWORD dwCookie);
BOOL CPFA_IsAssociated(CPs_CoDecModule* pCoDec, const char* pcExtension, DWORD* pdwCookie);
void CPFA_AssociateWithEXE(CPs_CoDecModule* pCoDec);
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// CoDec initialisers
void CP_InitialiseCodec_MPEG(CPs_CoDecModule* pCoDec);
void CP_InitialiseCodec_WAV(CPs_CoDecModule* pCoDec);
void CP_InitialiseCodec_OGG(CPs_CoDecModule* pCoDec);
void CP_InitialiseCodec_WinAmpPlugin(CPs_CoDecModule* pCoDec);
////////////////////////////////////////////////////////////////////////////////
