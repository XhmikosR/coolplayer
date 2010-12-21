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



CPs_InStream* CP_CreateInStream_LocalFile(const char* pcFlexiURL, HWND hWndOwner);
CPs_InStream* CP_CreateInStream_Internet(const char* pcFlexiURL, HWND hWndOwner);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CPs_InStream* CP_CreateInStream(const char* pcFlexiURL, HWND hWndOwner)
{
	CPs_InStream* pNewStream = NULL;
	int iURLLen = strlen(pcFlexiURL);
	
	if (iURLLen > 5)
	{
		char cHeader[6];
		memcpy(cHeader, pcFlexiURL, 5);
		cHeader[5] = '\0';
		
		if (stricmp(cHeader, "http:") == 0)
		{
			pNewStream = CP_CreateInStream_Internet(pcFlexiURL, hWndOwner);
			
			if (pNewStream)
				return pNewStream;
		}
	}
	
	// Try the local file system
	pNewStream = CP_CreateInStream_LocalFile(pcFlexiURL, hWndOwner);
	
	if (pNewStream)
		return pNewStream;
		
	return NULL;
}

//
//
//
