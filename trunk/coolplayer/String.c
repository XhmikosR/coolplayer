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


////////////////////////////////////////////////////////////////////////////////
//
//
//
unsigned int STR_AllocSetString(char** ppcDest, const char* pcSource, const BOOL bFreeExisting)
{
	if (bFreeExisting == TRUE && *ppcDest)
		free(*ppcDest);
		
	if (pcSource)
	{
		unsigned int uStringLength;
		
		uStringLength = strlen(pcSource) + 1;
		*ppcDest = (char*)malloc(uStringLength);
		
		if (!*ppcDest)
		{
			// Failed to allocate memory, a memcpy here would be fatal.
			return 0;
		}
		
		memcpy(*ppcDest, pcSource, uStringLength);
		
		return uStringLength;
	}
	
	*ppcDest = NULL;
	
	return 0;
}

//
//
//

