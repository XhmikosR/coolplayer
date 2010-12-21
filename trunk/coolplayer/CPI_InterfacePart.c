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
#include "CPI_InterfacePart.h"


////////////////////////////////////////////////////////////////////////////////
//
//
//
void IP_Invalidate(CP_HINTERFACEPART hPart)
{
	CPs_InterfacePart* pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	
	InvalidateRect(IF_GetHWnd(pIP->m_hOwner),
				   &pIP->m_rLocation, FALSE);
}

//
//
//
void IP_Destroy(CP_HINTERFACEPART hPart)
{
	CPs_InterfacePart* pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	
	if (pIP->Destroy_PrivateData)
		pIP->Destroy_PrivateData(pIP);
		
	free(pIP);
}

//
//
//

