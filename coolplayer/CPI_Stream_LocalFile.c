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
#include "CPI_Stream.h"



////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_InStream_File
{
	HANDLE m_hFile;
	
} CPs_InStream_File;

//
//
////////////////////////////////////////////////////////////////////////////////



void CPSLOCAL_Uninitialise(CPs_InStream* pStream);
BOOL CPSLOCAL_Read(CPs_InStream* pStream, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* piBytesRead);
void CPSLOCAL_Seek(CPs_InStream* pStream, const unsigned int iNewOffset);
unsigned int CPSLOCAL_GetLength(CPs_InStream* pStream);
BOOL CPSLOCAL_IsSeekable(CPs_InStream* pStream);
unsigned int CPSLOCAL_Tell(CPs_InStream* pStream);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CPs_InStream* CP_CreateInStream_LocalFile(const char* pcFlexiURL, HWND hWndOwner)
{
	// Check that we can open this file
	HANDLE hFile;
	
	hFile = CreateFile(pcFlexiURL, GENERIC_READ,
					   FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
					   OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	                   
	// Cannot open
	
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;
		
	{
		CPs_InStream* pNewStream = (CPs_InStream*)malloc(sizeof(CPs_InStream));
		CPs_InStream_File* pContext = (CPs_InStream_File*)malloc(sizeof(CPs_InStream_File));
		
		pNewStream->Uninitialise = CPSLOCAL_Uninitialise;
		pNewStream->Read = CPSLOCAL_Read;
		pNewStream->Seek = CPSLOCAL_Seek;
		pNewStream->Tell = CPSLOCAL_Tell;
		pNewStream->GetLength = CPSLOCAL_GetLength;
		pNewStream->IsSeekable = CPSLOCAL_IsSeekable;
		pNewStream->m_pModuleCookie = pContext;
		
		pContext->m_hFile = hFile;
		
		return pNewStream;
	}
}

//
//
//
void CPSLOCAL_Uninitialise(CPs_InStream* pStream)
{
	CPs_InStream_File* pContext = (CPs_InStream_File*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	CloseHandle(pContext->m_hFile);
	free(pContext);
	free(pStream);
}

//
//
//
BOOL CPSLOCAL_Read(CPs_InStream* pStream, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* piBytesRead)
{
	DWORD bytes;
	BOOL reply;
	
	CPs_InStream_File* pContext = (CPs_InStream_File*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	reply = ReadFile(pContext->m_hFile, pDestBuffer, iBytesToRead, &bytes, 0);
	*piBytesRead = (unsigned int) bytes;
	return reply;
}

//
//
//
void CPSLOCAL_Seek(CPs_InStream* pStream, const unsigned int iNewOffset)
{
	CPs_InStream_File* pContext = (CPs_InStream_File*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	SetFilePointer(pContext->m_hFile, iNewOffset, 0, FILE_BEGIN);
}

//
//
//
unsigned int CPSLOCAL_GetLength(CPs_InStream* pStream)
{
	CPs_InStream_File* pContext = (CPs_InStream_File*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	return GetFileSize(pContext->m_hFile, NULL);
}

//
//
//
BOOL CPSLOCAL_IsSeekable(CPs_InStream* pStream)
{
	return TRUE;
}

//
//
//
#define GetFilePointer(hFile) SetFilePointer(hFile, 0, NULL, FILE_CURRENT)

unsigned int CPSLOCAL_Tell(CPs_InStream* pStream)
{
	CPs_InStream_File* pContext = (CPs_InStream_File*)pStream->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	return GetFilePointer(pContext->m_hFile);
}
