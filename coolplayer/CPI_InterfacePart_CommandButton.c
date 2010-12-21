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

typedef struct _CPs_IPCommandButton
{
	CPs_Image_WithState* m_pStateImage;
	CPe_ImageState m_enCurrentState;
	wp_Verb m_pfnVerb;
	BOOL m_bDown;
	
} CPs_IPCommandButton;

//
////////////////////////////////////////////////////////////////////////////////



void IPCB_Destroy_PrivateData(CP_HINTERFACEPART hPart);
void IPCB_Draw(CP_HINTERFACEPART hPart, CPs_DrawContext* pContext);
void IPCB_onMouseIn(CP_HINTERFACEPART hPart);
void IPCB_onMouseOut(CP_HINTERFACEPART hPart);
void IPCB_onMouseButton_LDown(CP_HINTERFACEPART hPart, const POINTS ptMouse);
void IPCB_onMouseButton_LUp(CP_HINTERFACEPART hPart, const POINTS ptMouse);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CP_HINTERFACEPART IP_Create_CommandButton(wp_Verb pfnVerb, CPs_Image_WithState* pImageWS)
{
	CPs_InterfacePart* pNewPart;
	CPs_IPCommandButton* pCustomData;
	
	// Setup custom data
	pCustomData = (CPs_IPCommandButton*)malloc(sizeof(*pCustomData));
	pCustomData->m_pStateImage = pImageWS;
	pCustomData->m_enCurrentState = igsQuiescent;
	pCustomData->m_pfnVerb = pfnVerb;
	pCustomData->m_bDown = FALSE;
	
	// Create new part and setup callbacks
	pNewPart = (CPs_InterfacePart*)malloc(sizeof(*pNewPart));
	memset(pNewPart, 0, sizeof(*pNewPart));
	pNewPart->Destroy_PrivateData = IPCB_Destroy_PrivateData;
	pNewPart->Draw = IPCB_Draw;
	pNewPart->onMouseIn = IPCB_onMouseIn;
	pNewPart->onMouseOut = IPCB_onMouseOut;
	pNewPart->onMouseButton_LDown = IPCB_onMouseButton_LDown;
	pNewPart->onMouseButton_LUp = IPCB_onMouseButton_LUp;
	pNewPart->m_pPrivateData = pCustomData;
	
	return pNewPart;
}

//
//
//
void IPCB_Draw(CP_HINTERFACEPART hPart, CPs_DrawContext* pContext)
{
	CPs_InterfacePart* pIP;
	CPs_IPCommandButton* pIPCB;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPCB = (CPs_IPCommandButton*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPCB);
	
	// Perform drawing
	CPIG_DrawStateImage(pContext,
						pIP->m_rLocation.left, pIP->m_rLocation.top,
						pIPCB->m_pStateImage, pIPCB->m_enCurrentState);
}

//
//
//
void IPCB_Destroy_PrivateData(CP_HINTERFACEPART hPart)
{
	CPs_InterfacePart* pIP;
	CPs_IPCommandButton* pIPCB;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPCB = (CPs_IPCommandButton*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPCB);
	
	free(pIPCB);
}

//
//
//
void IPCB_onMouseIn(CP_HINTERFACEPART hPart)
{
	CPs_InterfacePart* pIP;
	CPs_IPCommandButton* pIPCB;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPCB = (CPs_IPCommandButton*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPCB);
	
	// Handler
	
	if (pIPCB->m_bDown)
		pIPCB->m_enCurrentState = igsActive;
	else
		pIPCB->m_enCurrentState = igsFloatActive;
		
	IP_Invalidate(hPart);
}

//
//
//
void IPCB_onMouseOut(CP_HINTERFACEPART hPart)
{
	CPs_InterfacePart* pIP;
	CPs_IPCommandButton* pIPCB;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPCB = (CPs_IPCommandButton*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPCB);
	
	// Handler
	pIPCB->m_enCurrentState = igsQuiescent;
	IP_Invalidate(hPart);
}

//
//
//
void IPCB_onMouseButton_LDown(CP_HINTERFACEPART hPart, const POINTS ptMouse)
{
	CPs_InterfacePart* pIP;
	CPs_IPCommandButton* pIPCB;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPCB = (CPs_IPCommandButton*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPCB);
	
	// Handler
	pIPCB->m_bDown = TRUE;
	
	if (pIPCB->m_enCurrentState == igsFloatActive)
		pIPCB->m_enCurrentState = igsActive;
		
	IP_Invalidate(hPart);
}

//
//
//
void IPCB_onMouseButton_LUp(CP_HINTERFACEPART hPart, const POINTS ptMouse)
{
	CPs_InterfacePart* pIP;
	CPs_IPCommandButton* pIPCB;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPCB = (CPs_IPCommandButton*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPCB);
	
	// Handler
	pIPCB->m_bDown = FALSE;
	
	if (pIPCB->m_enCurrentState == igsActive)
	{
		// Mouse is still inside control
		pIPCB->m_enCurrentState = igsFloatActive;
		
		if (pIPCB->m_pfnVerb)
			pIPCB->m_pfnVerb(vaDoVerb, NULL);
	}
	
	IP_Invalidate(hPart);
}

//
//
//

