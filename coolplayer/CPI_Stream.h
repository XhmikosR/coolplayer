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
// Cooler Stream functions
//
////////////////////////////////////////////////////////////////////////////////


// Forward reference
struct _CPs_InStream;

////////////////////////////////////////////////////////////////////////////////
// Stream functions

typedef struct _CPs_InStream* CP_HINSTREAM;
typedef void (*pfn_StreamUninitialise)(CP_HINSTREAM hStream);
typedef BOOL (*pfn_StreamRead)(CP_HINSTREAM hStream, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* piBytesRead);
typedef void (*pfn_StreamSeek)(CP_HINSTREAM hStream, const unsigned int iNewOffset);
typedef UINT(*pfn_StreamTell)(CP_HINSTREAM hStream);
typedef UINT(*pfn_StreamGetLength)(CP_HINSTREAM hStream);
typedef BOOL (*pfn_StreamIsSeakable)(CP_HINSTREAM hStream);
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// CoDec module

typedef struct _CPs_InStream
{
	// Public functions
	pfn_StreamUninitialise Uninitialise;
	pfn_StreamRead Read;
	pfn_StreamSeek Seek;
	pfn_StreamTell Tell;
	pfn_StreamGetLength GetLength;
	pfn_StreamIsSeakable IsSeekable;
	
	// Public variables
	void* m_pModuleCookie;  // This is a pointer to any private data the module may want to maintain
	
} CPs_InStream;

//
////////////////////////////////////////////////////////////////////////////////
CPs_InStream* CP_CreateInStream(const char* pcFlexiURL, HWND hWndOwner);
