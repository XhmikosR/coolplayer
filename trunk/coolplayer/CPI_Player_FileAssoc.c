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




#include "stdafx.h"
#include "globals.h"
#include "CPI_Player_CoDec.h"

////////////////////////////////////////////////////////////////////////////////
//
// These are a bunch of functions to help with the manipulation of the file
// association structs.
//
// The associations are a linked list.
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_FileAssociation
{
	char* m_pcExtension;
	DWORD m_dwCookie;
	void* m_pNext;
	
} CPs_FileAssociation;

//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPFA_InitialiseFileAssociations(CPs_CoDecModule* pCoDec)
{
	pCoDec->m_pFileAssociationCookie = NULL;
}

//
//
//
void CPFA_EmptyFileAssociations(CPs_CoDecModule* pCoDec)
{
	CPs_FileAssociation* pCursor = (CPs_FileAssociation*)pCoDec->m_pFileAssociationCookie;
	CPs_FileAssociation* pNext;
	
	// Free list contents
	
	while (pCursor)
	{
		CP_CHECKOBJECT(pCursor);
		pNext = (CPs_FileAssociation*)pCursor->m_pNext;
		
		free(pCursor->m_pcExtension);
		free(pCursor);
		pCursor = pNext;
	}
}

//
//
//
void CPFA_AddFileAssociation(CPs_CoDecModule* pCoDec, const char* pcExtension, DWORD dwCookie)
{
	CPs_FileAssociation* pNewAssociation = (CPs_FileAssociation*)malloc(sizeof(CPs_FileAssociation));
	
	// Check that this extension is not already used
	CPs_FileAssociation* pCursor = (CPs_FileAssociation*)pCoDec->m_pFileAssociationCookie;
	
	while (pCursor)
	{
		if (stricmp(pcExtension, pCursor->m_pcExtension) == 0)
		{
			CP_TRACE1("** Extension \"%s\" already registered here", pcExtension);
			return;
		}
		
		pCursor = (CPs_FileAssociation*)pCursor->m_pNext;
	}
	
	// Make copy of extension sting - just in case it's ever called with a stack or
	// dynamic buffer
	STR_AllocSetString(&pNewAssociation->m_pcExtension, pcExtension, FALSE);
	
	pNewAssociation->m_dwCookie = dwCookie;
	
	// Link to start (order is not important here!)
	pNewAssociation->m_pNext = pCoDec->m_pFileAssociationCookie;
	
	pCoDec->m_pFileAssociationCookie = pNewAssociation;
}

//
//
//
BOOL CPFA_IsAssociated(CPs_CoDecModule* pCoDec, const char* pcExtension, DWORD* pdwCookie)
{
	// Walk list looking for the extension
	CPs_FileAssociation* pCursor = (CPs_FileAssociation*)pCoDec->m_pFileAssociationCookie;
	
	while (pCursor)
	{
		CP_CHECKOBJECT(pCursor);
		
		if (stricmp(pcExtension, pCursor->m_pcExtension) == 0)
		{
			*pdwCookie = pCursor->m_dwCookie;
			return TRUE;
		}
		
		pCursor = (CPs_FileAssociation*)pCursor->m_pNext;
	}
	
	return FALSE;
}

//
//
//
void CPFA_AssociateWithEXE(CPs_CoDecModule* pCoDec)
{
	// Walk list creating as we go
	CPs_FileAssociation* pCursor = (CPs_FileAssociation*)pCoDec->m_pFileAssociationCookie;
	HKEY hKey;
	DWORD dwDisposition;
	
	while (pCursor)
	{
		char* pDotExt;
		CP_CHECKOBJECT(pCursor);
		
		// Build a string of the format .ext
		pDotExt = (char*)malloc(strlen(pCursor->m_pcExtension) + 2);
		pDotExt[0] = '.';
		strcpy(pDotExt + 1, pCursor->m_pcExtension);
		
		CP_TRACE1("Associating extension: \"%s\"", pCursor->m_pcExtension);
		
		// Setup registry
		RegCreateKeyEx(HKEY_CLASSES_ROOT, pDotExt, 0, NULL,
					   REG_OPTION_NON_VOLATILE,
					   KEY_ALL_ACCESS, NULL, &hKey,
					   &dwDisposition);
		RegSetValueEx(hKey, NULL, 0, REG_SZ, CIC_COOLPLAYER_FILETYPE, sizeof(CIC_COOLPLAYER_FILETYPE));
		RegCloseKey(hKey);
		
		// Cleanup
		free(pDotExt);
		
		// Move to next extension
		pCursor = (CPs_FileAssociation*)pCursor->m_pNext;
	}
}

//
//
//
