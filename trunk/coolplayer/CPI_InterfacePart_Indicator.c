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
#include "CPI_Indicators.h"


////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_IPIndicator
{
	char* m_pcName;
	
} CPs_IPIndicator;

//
////////////////////////////////////////////////////////////////////////////////



void IPIC_Destroy_PrivateData(CP_HINTERFACEPART hPart);
void IPIC_Draw(CP_HINTERFACEPART hPart, CPs_DrawContext* pContext);
////////////////////////////////////////////////////////////////////////////////
//
//
//
CP_HINTERFACEPART IP_Create_Indicator(const char* pcName)
{
	CPs_InterfacePart* pNewPart;
	CPs_IPIndicator* pCustomData;
	
	// Setup custom data
	pCustomData = (CPs_IPIndicator*)malloc(sizeof(*pCustomData));
	STR_AllocSetString(&pCustomData->m_pcName, pcName, FALSE);
	
	// Create new part and setup callbacks
	pNewPart = (CPs_InterfacePart*)malloc(sizeof(*pNewPart));
	memset(pNewPart, 0, sizeof(*pNewPart));
	pNewPart->Destroy_PrivateData = IPIC_Destroy_PrivateData;
	pNewPart->Draw = IPIC_Draw;
	pNewPart->m_pPrivateData = pCustomData;
	
	CPIC_BindIndicatorToControl(pcName, pNewPart);
	
	return pNewPart;
}

//
//
//
void IPIC_Draw(CP_HINTERFACEPART hPart, CPs_DrawContext* pContext)
{
	CPs_InterfacePart* pIP;
	const char* pcValue;
	CPs_IPIndicator* pIPIC;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPIC = (CPs_IPIndicator*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPIC);
	
	// Draw the window background
	CPIG_TiledFill(pContext, &pIP->m_rLocation, &glb_pSkin->mpl_rListHeader_SourceTile, glb_pSkin->mpl_pListHeader_Down, CIC_TILEDFILOPTIONS_NONE);
	
	pcValue = CPIC_GetIndicatorValue(pIPIC->m_pcName);
	
	if (pcValue)
	{
		HFONT hfOld;
		RECT rText;
		
		rText = pIP->m_rLocation;
		rText.left += glb_pSkin->mpl_rListHeader_SourceTile.left;
		rText.right -= glb_pSkin->mpl_rListHeader_SourceTile.right;
		
		hfOld = (HFONT)SelectObject(pContext->m_dcDraw, glb_pSkin->mpl_hfFont);
		SetTextColor(pContext->m_dcDraw, glb_pSkin->mpl_ListHeaderColour);
		SetBkMode(pContext->m_dcDraw, TRANSPARENT);
		DrawText(pContext->m_dcDraw, pcValue, -1, &rText, DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_VCENTER);
		SelectObject(pContext->m_dcDraw, hfOld);
	}
}

//
//
//
void IPIC_Destroy_PrivateData(CP_HINTERFACEPART hPart)
{
	CPs_InterfacePart* pIP;
	CPs_IPIndicator* pIPIC;
	
	// Init
	pIP = (CPs_InterfacePart*)hPart;
	CP_CHECKOBJECT(pIP);
	pIPIC = (CPs_IPIndicator*)pIP->m_pPrivateData;
	CP_CHECKOBJECT(pIPIC);
	
	free(pIPIC->m_pcName);
	free(pIPIC);
	CPIC_UnBindControl(hPart);
}

//
//
//
