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
// Cooler Circle buffer
//
////////////////////////////////////////////////////////////////////////////////

// Forward reference
struct _CPs_CircleBuffer;

////////////////////////////////////////////////////////////////////////////////
// Stream functions

typedef struct _CPs_CircleBuffer* CP_HCIRCLEBUFFER;
typedef void (*pfn_CircleBufferUninitialise)(CP_HCIRCLEBUFFER bBuffer);
//
typedef void (*pfn_CircleBufferWrite)(CP_HCIRCLEBUFFER bBuffer, const void* pSourceBuffer, const unsigned int iNumBytes);
typedef BOOL (*pfn_CircleBufferRead)(CP_HCIRCLEBUFFER bBuffer, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* pbBytesRead);
typedef unsigned int (*pfn_CircleGetUsedSpace)(CP_HCIRCLEBUFFER bBuffer);
typedef unsigned int (*pfn_CircleGetFreeSpace)(CP_HCIRCLEBUFFER bBuffer);
typedef void (*pfn_CircleFlush)(CP_HCIRCLEBUFFER bBuffer);
typedef void (*pfn_CircleSetComplete)(CP_HCIRCLEBUFFER bBuffer);
typedef BOOL (*pfn_CircleIsComplete)(CP_HCIRCLEBUFFER bBuffer);
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Circle Buffer
typedef struct _CPs_CircleBuffer
{
	// Public functions
	pfn_CircleBufferUninitialise Uninitialise;
	pfn_CircleBufferWrite Write;
	pfn_CircleBufferRead Read;
	pfn_CircleFlush Flush;
	pfn_CircleGetUsedSpace GetUsedSize;
	pfn_CircleGetFreeSpace GetFreeSize;
	pfn_CircleSetComplete SetComplete;
	pfn_CircleIsComplete IsComplete;
	
	// Private variables
	BYTE* m_pBuffer;
	unsigned int m_iBufferSize;
	unsigned int m_iReadCursor;
	unsigned int m_iWriteCursor;
	HANDLE m_evtDataAvailable;
	CRITICAL_SECTION m_csCircleBuffer;
	BOOL m_bComplete;
	
} CPs_CircleBuffer;

//
////////////////////////////////////////////////////////////////////////////////


CPs_CircleBuffer* CP_CreateCircleBuffer(const unsigned int iBufferSize);
