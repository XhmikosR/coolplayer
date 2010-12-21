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
#include "resource.h"
#include "CompositeFile.h"


void CPSK_DestroySkin(CPs_Skin* pSkin);
CPs_Skin* CPSK_LoadSkin(CP_COMPOSITEFILE hComposite, const char* pcSkinFile, const unsigned int iFileSize);
CPs_Skin* glb_pSkin = NULL;
////////////////////////////////////////////////////////////////////////////////
//
//
//
void CPSK_Initialise()
{
	char* pcSkinFile;
	unsigned int iFileSize;
	
	CP_COMPOSITEFILE hComposite;
	
	hComposite = CF_Create_FromResource(NULL, IDR_DEFAULTSKIN, "SKIN");
	//hComposite = CF_Create_FromFile("P:\\Skin\\Default.CPSkin");
	
	CF_GetSubFile(hComposite, "Skin.def", (void **) &pcSkinFile, &iFileSize);
	glb_pSkin = CPSK_LoadSkin(hComposite, pcSkinFile, iFileSize);
	
	free(pcSkinFile);
	CF_Destroy(hComposite);
}

//
//
//
void CPSK_Uninitialise()
{
	if (glb_pSkin)
		CPSK_DestroySkin(glb_pSkin);
		
	glb_pSkin = NULL;
}

//
//
//
void CPSK_DestroySkin(CPs_Skin* pSkin)
{
	if (pSkin->mpl_hfFont)
		DeleteObject(pSkin->mpl_hfFont);
		
	CPIG_DestroyImage(pSkin->mpl_pBackground);
	CPIG_DestroyImage(pSkin->mpl_pListBackground);
	CPIG_DestroyImage(pSkin->mpl_pListHeader_Up);
	CPIG_DestroyImage(pSkin->mpl_pListHeader_Down);
	CPIG_DestroyImage(pSkin->mpl_pHScrollBar_Bk);
	CPIG_DestroyImage(pSkin->mpl_pHScrollBar_TrackUp);
	CPIG_DestroyImage(pSkin->mpl_pHScrollBar_TrackDn);
	CPIG_DestroyImage_WithState(pSkin->mpl_pHScrollBar_Left);
	CPIG_DestroyImage_WithState(pSkin->mpl_pHScrollBar_Right);
	CPIG_DestroyImage(pSkin->mpl_pVScrollBar_Bk);
	CPIG_DestroyImage(pSkin->mpl_pVScrollBar_TrackUp);
	CPIG_DestroyImage(pSkin->mpl_pVScrollBar_TrackDn);
	CPIG_DestroyImage_WithState(pSkin->mpl_pVScrollBar_Up);
	CPIG_DestroyImage_WithState(pSkin->mpl_pVScrollBar_Down);
	CPIG_DestroyImage(pSkin->mpl_pSelection);
	
	// Remove the command targets
	{
		CPs_CommandTarget* pCT_Cursor = pSkin->mpl_pCommandTargets;
		CPs_CommandTarget* pCT_Cursor_Next;
		
		while (pCT_Cursor)
		{
			CPIG_DestroyImage_WithState(pCT_Cursor->m_pStateImage);
			pCT_Cursor_Next = (CPs_CommandTarget*)pCT_Cursor->m_pNext;
			free(pCT_Cursor);
			pCT_Cursor = pCT_Cursor_Next;
		}
	}
	
	// Remove the indicators
	{
		CPs_Indicator* pCT_Cursor = pSkin->mpl_pIndicators;
		CPs_Indicator* pCT_Cursor_Next;
		
		while (pCT_Cursor)
		{
			pCT_Cursor_Next = (CPs_Indicator*)pCT_Cursor->m_pNext;
			free(pCT_Cursor->m_pcName);
			free(pCT_Cursor);
			pCT_Cursor = pCT_Cursor_Next;
		}
	}
	
	free(pSkin);
}

//
//
//
COLORREF CPSK_DecodeColour(const char* pcColour)
{
	unsigned long dwColour;
	
	if (sscanf(pcColour, "#%lx", &dwColour) == 1)
	{
		// Swap red and blue bytes
		return ((dwColour&0x0000FF) << 16) | (dwColour&0x00FF00) | ((dwColour&0xFF0000) >> 16);
	}
	
	return 0;
}

//
//
//
void CPSK_ReadSkinCommand_Define(CP_COMPOSITEFILE hComposite, CPs_Skin* pSkin, const char* pcParams)
{
	char cSymbol[34];
	char cValue[130];
	
	// Decode params
	
	if (sscanf(pcParams, " %32[A-Za-z_] = \"%128[^\"]\"", cSymbol, cValue) != 2)
		return;
		
	if (stricmp(cSymbol, "CoolSkinVersion") == 0)
	{
		unsigned long version;
		
		sscanf(cValue, " %lu ", &version);
		pSkin->m_dwSkinVersion = version;
	}
	
	else if (stricmp(cSymbol, "Transparent_MaskColour") == 0)
	{
		pSkin->m_clrTransparent = CPSK_DecodeColour(cValue);
	}
	
	else if (stricmp(cSymbol, "Playlist_Font") == 0)
	{
		// Clean up old object if the skindef has more than one font
		if (pSkin->mpl_hfFont)
			DeleteObject(pSkin->mpl_hfFont);
			
		pSkin->mpl_hfFont = CreateFont(-12, 0, 0, 0, FW_NORMAL,
									   FALSE, FALSE, FALSE,
									   ANSI_CHARSET,
									   OUT_TT_PRECIS,
									   CLIP_DEFAULT_PRECIS,
									   ANTIALIASED_QUALITY,
									   DEFAULT_PITCH | FF_SWISS,
									   cValue);
	}
	
	else if (stricmp(cSymbol, "Playlist_Colour_Text") == 0)
	{
		pSkin->mpl_ListTextColour = CPSK_DecodeColour(cValue);
	}
	
	else if (stricmp(cSymbol, "Playlist_Colour_Selected") == 0)
	{
		pSkin->mpl_ListTextColour_Selected = CPSK_DecodeColour(cValue);
	}
	
	else if (stricmp(cSymbol, "Playlist_Colour_Playing") == 0)
	{
		pSkin->mpl_ListTextColour_HotItem = CPSK_DecodeColour(cValue);
	}
	
	else if (stricmp(cSymbol, "Playlist_Colour_HeaderText") == 0)
	{
		pSkin->mpl_ListHeaderColour = CPSK_DecodeColour(cValue);
	}
	
	else if (stricmp(cSymbol, "Playlist_ListBorders") == 0)
	{
		long left, top, right, bottom;
		
		if (sscanf(cValue, " %ld , %ld ,  %ld , %ld ",
				   &left,
				   &right,
				   &top,
				   &bottom) != 4)
		{
			top = 0;
			left = 0;
			right = 0;
			bottom = 0;
		}
		
		pSkin->mpl_rList_Border.top = top;
		pSkin->mpl_rList_Border.left = left;
		pSkin->mpl_rList_Border.right = right;
		pSkin->mpl_rList_Border.bottom = bottom;
	}
	
	else if (stricmp(cSymbol, "Playlist_MinSize") == 0)
	{
		long cx, cy;
		
		if (sscanf(cValue, " %ld , %ld ", &cx, &cy) != 2)
		{
			cx = 400;
			cy = 200;
		}
		
		pSkin->mpl_szMinWindow.cx = cx;
		pSkin->mpl_szMinWindow.cy = cy;
	}
}

//
//
//
void CPSK_ReadSkinCommand_TiledDraw(CP_COMPOSITEFILE hComposite, CPs_Skin* pSkin, const char* pcParams)
{
	char cElement[34];
	char cFile[130];
	RECT rTileBorders;
	CPs_Image** ppImage;
	RECT* pRect;
	long left, top, right, bottom;
	
	// Decode params
	
	if (sscanf(pcParams, " %32[A-Za-z_-] , \"%128[^\"]\" , %ld , %ld , %ld , %ld ",
			   cElement, cFile, &left, &top, &right, &bottom) != 6)
	{
		return;
	}
	
	rTileBorders.top = top;
	rTileBorders.left = left;
	rTileBorders.right = right;
	rTileBorders.bottom = bottom;
	
	// Decide which data members are affected
	
	if (stricmp(cElement, "Playlist_Background") == 0)
	{
		pRect = &pSkin->mpl_rBackground_SourceTile;
		ppImage = &pSkin->mpl_pBackground;
	}
	
	else if (stricmp(cElement, "Playlist_ListBackground") == 0)
	{
		pRect = &pSkin->mpl_rListBackground_SourceTile;
		ppImage = &pSkin->mpl_pListBackground;
	}
	
	else if (stricmp(cElement, "Playlist_Header_up") == 0)
	{
		pRect = &pSkin->mpl_rListHeader_SourceTile;
		ppImage = &pSkin->mpl_pListHeader_Up;
	}
	
	else if (stricmp(cElement, "Playlist_Header_dn") == 0)
	{
		pRect = &pSkin->mpl_rListHeader_SourceTile;
		ppImage = &pSkin->mpl_pListHeader_Down;
	}
	
	else if (stricmp(cElement, "Playlist_Selection") == 0)
	{
		pRect = &pSkin->mpl_rSelection_Tile;
		ppImage = &pSkin->mpl_pSelection;
	}
	
	else if (stricmp(cElement, "Playlist_Focus") == 0)
	{
		pRect = &pSkin->mpl_rFocus_Tile;
		ppImage = &pSkin->mpl_pFocus;
	}
	
	else if (stricmp(cElement, "HScrollBar_Background") == 0)
	{
		pRect = &pSkin->mpl_rHScrollBar_Bk_Tile;
		ppImage = &pSkin->mpl_pHScrollBar_Bk;
	}
	
	else if (stricmp(cElement, "HScrollBar_Tracker_up") == 0)
	{
		pRect = &pSkin->mpl_rHScrollBar_Track_Tile;
		ppImage = &pSkin->mpl_pHScrollBar_TrackUp;
	}
	
	else if (stricmp(cElement, "HScrollBar_Tracker_dn") == 0)
	{
		pRect = &pSkin->mpl_rHScrollBar_Track_Tile;
		ppImage = &pSkin->mpl_pHScrollBar_TrackDn;
	}
	
	else if (stricmp(cElement, "VScrollBar_Background") == 0)
	{
		pRect = &pSkin->mpl_rVScrollBar_Bk_Tile;
		ppImage = &pSkin->mpl_pVScrollBar_Bk;
	}
	
	else if (stricmp(cElement, "VScrollBar_Tracker_up") == 0)
	{
		pRect = &pSkin->mpl_rVScrollBar_Track_Tile;
		ppImage = &pSkin->mpl_pVScrollBar_TrackUp;
	}
	
	else if (stricmp(cElement, "VScrollBar_Tracker_dn") == 0)
	{
		pRect = &pSkin->mpl_rVScrollBar_Track_Tile;
		ppImage = &pSkin->mpl_pVScrollBar_TrackDn;
	}
	
	else
		return;
		
	// Set data members
	if (*ppImage)
		CPIG_DestroyImage(*ppImage);
		
	*ppImage = CPIG_CreateImage_FromSubFile(hComposite, cFile);
	
	*pRect = rTileBorders;
	
	if (*ppImage)
	{
		pRect->right = (*ppImage)->m_szSize.cx - rTileBorders.right;
		pRect->bottom = (*ppImage)->m_szSize.cy - rTileBorders.bottom;
	}
}

//
//
//
void CPSK_ReadSkinCommand_ButtonDraw(CP_COMPOSITEFILE hComposite, CPs_Skin* pSkin, const char* pcParams)
{
	char cElement[34];
	char cStates[14];
	char cFile[130];
	CPs_Image_WithState** ppImage;
	int iNumStates;
	
	// Decode params
	
	if (sscanf(pcParams, " %32[A-Za-z_-] , \"%128[^\"]\" , %12s ",
			   cElement, cFile, cStates) != 3)
	{
		return;
	}
	
	// Setup number of states
	
	if (stricmp(cStates, "2State") == 0)
		iNumStates = 2;
	else
		iNumStates = 2;
		
	// Decide which data members are affected by this command
	if (stricmp(cElement, "HScrollBar_Left") == 0)
	{
		ppImage = &pSkin->mpl_pHScrollBar_Left;
	}
	
	else if (stricmp(cElement, "HScrollBar_Right") == 0)
	{
		ppImage = &pSkin->mpl_pHScrollBar_Right;
	}
	
	else if (stricmp(cElement, "VScrollBar_Up") == 0)
	{
		ppImage = &pSkin->mpl_pVScrollBar_Up;
	}
	
	else if (stricmp(cElement, "VScrollBar_Down") == 0)
	{
		ppImage = &pSkin->mpl_pVScrollBar_Down;
	}
	
	else
		return;
		
	// Set data members
	if (*ppImage)
		CPIG_DestroyImage_WithState(*ppImage);
		
	*ppImage = CPIG_CreateStateImage(CPIG_CreateImage_FromSubFile(hComposite, cFile), iNumStates);
}

//
//
//
DWORD CPSK_DecodeAlign(const char* pcAlign)
{
	char cAlign[1024];
	char cAlign_Remains[130];
	char cAlignFlag[130];
	DWORD dwAlignFlag = 0;
	
	strcpy(cAlign, pcAlign);
	// while(sscanf(cAlign, " %128[a-zA-Z_] | %[^\0]", cAlignFlag, cAlign_Remains) > 0)
	
	while (sscanf(cAlign, " %128[a-zA-Z_] | %128s", cAlignFlag, cAlign_Remains) > 0)
	{
		strcpy(cAlign, cAlign_Remains);
		cAlign_Remains[0] = '\0';
		
		if (stricmp(cAlignFlag, "ALIGN_LEFT") == 0)
			dwAlignFlag |= CPC_COMMANDTARGET_ALIGN_LEFT;
		else if (stricmp(cAlignFlag, "ALIGN_TOP") == 0)
			dwAlignFlag |= CPC_COMMANDTARGET_ALIGN_TOP;
		else if (stricmp(cAlignFlag, "ALIGN_RIGHT") == 0)
			dwAlignFlag |= CPC_COMMANDTARGET_ALIGN_RIGHT;
		else if (stricmp(cAlignFlag, "ALIGN_BOTTOM") == 0)
			dwAlignFlag |= CPC_COMMANDTARGET_ALIGN_BOTTOM;
	}
	
	return dwAlignFlag;
}

//
//
//
void CPSK_ReadSkinCommand_AddVerb(CP_COMPOSITEFILE hComposite, CPs_CommandTarget** ppCommandTarget, const char* pcParams)
{
	char cElement[34];
	char cStates[14];
	char cAlign[130];
	char cFile[130];
	int iNumStates;
	POINT ptOffset;
	wp_Verb pfnVerb = NULL;
	DWORD dwAlignFlag;
	CPs_CommandTarget* pNext;
	long x, y;
	
	// Decode params
	
	if (sscanf(pcParams, " %32[A-Za-z_-] , \"%128[^\"]\" , %12[0-9a-zA-Z] , %ld , %ld , \"%128[^\"]\" ",
			   cElement, cFile, cStates, &x, &y, cAlign) != 6)
	{
		return;
	}
	
	ptOffset.x = x;
	ptOffset.y = y;
	
	// Setup number of states
	
	if (stricmp(cStates, "3State") == 0)
		iNumStates = 3;
	else
		iNumStates = 2;
		
	// Find the verb with this name
	{
		int iVerbIDX;
		CPs_VerbQueryName queryname;
		
		queryname.m_pcName = cElement;
		queryname.m_bNameMatched = FALSE;
		
		for (iVerbIDX = 0; glb_pfnAllVerbs[iVerbIDX]; iVerbIDX++)
		{
			glb_pfnAllVerbs[iVerbIDX](vaQueryName, &queryname);
			
			if (queryname.m_bNameMatched)
			{
				pfnVerb = glb_pfnAllVerbs[iVerbIDX];
				break;
			}
		}
	}
	
	// Build the align flags
	dwAlignFlag = CPSK_DecodeAlign(cAlign);
	
	// Load state image
	pNext = *ppCommandTarget;
	
	*ppCommandTarget = (CPs_CommandTarget*)malloc(sizeof(CPs_CommandTarget));
	(*ppCommandTarget)->m_pStateImage = CPIG_CreateStateImage(CPIG_CreateImage_FromSubFile(hComposite, cFile), iNumStates);
	(*ppCommandTarget)->m_ptOffset = ptOffset;
	(*ppCommandTarget)->m_dwAlign = dwAlignFlag;
	(*ppCommandTarget)->m_pfnVerb = pfnVerb;
	(*ppCommandTarget)->m_pNext = pNext;
}

//
//
//
void CPSK_ReadSkinCommand_AddIndicator(CP_COMPOSITEFILE hComposite, CPs_Skin* pSkin, const char* pcParams)
{
	char cElement[34];
	char cAlign[130];
	RECT rOffset;
	DWORD dwAlignFlag;
	CPs_Indicator* pNext;
	long left, top, right, bottom;
	
	// Decode params
	
	if (sscanf(pcParams, " %32[A-Za-z_-] , %ld , %ld , %ld , %ld , \"%128[^\"]\" ",
			   cElement, &left, &top, &right, &bottom, cAlign) != 6)
	{
		return;
	}
	
	rOffset.top = top;
	rOffset.left = left;
	rOffset.right = right;
	rOffset.bottom = bottom;
	
	// Build the align flags
	dwAlignFlag = CPSK_DecodeAlign(cAlign);
	
	// Load state image
	pNext = pSkin->mpl_pIndicators;
	pSkin->mpl_pIndicators = (CPs_Indicator*)malloc(sizeof(CPs_Indicator));
	pSkin->mpl_pIndicators->m_pNext = pNext;
	pSkin->mpl_pIndicators->m_dwAlign = dwAlignFlag;
	pSkin->mpl_pIndicators->m_rAlign = rOffset;
	STR_AllocSetString(&pSkin->mpl_pIndicators->m_pcName, cElement, FALSE);
	
}

void CPSK_ReadSkinCommand_AddPlaylistVerb(CP_COMPOSITEFILE hComposite, CPs_Skin* pSkin, const char* pcParams)
{
	CPSK_ReadSkinCommand_AddVerb(hComposite, &pSkin->mpl_pCommandTargets, pcParams);
}

//
//
//
void CPSK_ReadSkinLine(CP_COMPOSITEFILE hComposite, CPs_Skin* pSkin, const char* pcLine)
{
	char cCommand[34];
	const char *cParams;
	
	// Decode command
	// if(sscanf(pcLine, " %32s %480[^\0]", cCommand, cParams) != 2)
	
	if (sscanf(pcLine, " %32s", cCommand) != 1)
		return;
		
		
	// Skip comments
	if (cCommand[0] == '#')
		return;
		
	cParams = pcLine + strlen(cCommand) + 1;
	
	CP_TRACE2("Command:\"%s\" Params:\"%s\"", cCommand, cParams);
	
	if (stricmp(cCommand, "define") == 0)
		CPSK_ReadSkinCommand_Define(hComposite, pSkin, cParams);
	else if (stricmp(cCommand, "tileddraw") == 0)
		CPSK_ReadSkinCommand_TiledDraw(hComposite, pSkin, cParams);
	else if (stricmp(cCommand, "buttondraw") == 0)
		CPSK_ReadSkinCommand_ButtonDraw(hComposite, pSkin, cParams);
	else if (stricmp(cCommand, "addplaylistverb") == 0)
		CPSK_ReadSkinCommand_AddPlaylistVerb(hComposite, pSkin, cParams);
	else if (stricmp(cCommand, "addplaylistindicator") == 0)
		CPSK_ReadSkinCommand_AddIndicator(hComposite, pSkin, cParams);
}

//
//
//
CPs_Skin* CPSK_LoadSkin(CP_COMPOSITEFILE hComposite, const char* pcSkinFile, const unsigned int iFileSize)
{
	unsigned int iLastLineStartIDX, iCharIDX;
	CPs_Skin* pNewSkin = (CPs_Skin*)malloc(sizeof(CPs_Skin));
	memset(pNewSkin, 0, sizeof(*pNewSkin));
	
	// Read in the file line by line
	iLastLineStartIDX = 0;
	
	for (iCharIDX = 0; iCharIDX < iFileSize + 1; iCharIDX++)
	{
		if ((pcSkinFile[iCharIDX] == '\r'
				|| pcSkinFile[iCharIDX] == '\n'
				|| iCharIDX == iFileSize)
				&& iLastLineStartIDX < iCharIDX)
		{
			char cBuffer[512];
			int iBytesOnLine;
			
			iBytesOnLine = iCharIDX - iLastLineStartIDX;
			
			if (iBytesOnLine >= 512)
				iBytesOnLine = 511;
				
			memcpy(cBuffer, pcSkinFile + iLastLineStartIDX, iBytesOnLine);
			
			cBuffer[iBytesOnLine] = '\0';
			
			CPSK_ReadSkinLine(hComposite, pNewSkin, cBuffer);
			
			// Set the line start for the next line
			if (pcSkinFile[iCharIDX + 1] == '\n')
				iCharIDX++;
				
			iLastLineStartIDX = iCharIDX + 1;
		}
	}
	
	return pNewSkin;
}

//
//
//
