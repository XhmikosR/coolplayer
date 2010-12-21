
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
// Skin Support
//
////////////////////////////////////////////////////////////////////////////////


#define CPC_COMMANDTARGET_ALIGN_LEFT				0x1
#define CPC_COMMANDTARGET_ALIGN_RIGHT				0x2
#define CPC_COMMANDTARGET_ALIGN_TOP					0x4
#define CPC_COMMANDTARGET_ALIGN_BOTTOM				0x8
////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_CommandTarget
{
	CPs_Image_WithState* m_pStateImage;
	
	DWORD m_dwAlign;
	POINT m_ptOffset;
	wp_Verb m_pfnVerb;
	
	void* m_pNext;
	
} CPs_CommandTarget;

//
//

typedef struct _CPs_Indicator
{
	DWORD m_dwAlign;
	RECT m_rAlign;
	char* m_pcName;
	
	void* m_pNext;
	
} CPs_Indicator;

//
////////////////////////////////////////////////////////////////////////////////



#define CPC_SKINVERSION_200							200L
////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_Skin
{
	DWORD m_dwSkinVersion;
	COLORREF m_clrTransparent;
	
	// Playlist
	HFONT mpl_hfFont;
	CPs_Image* mpl_pBackground;
	RECT mpl_rBackground_SourceTile;
	RECT mpl_rList_Border;
	
	// List control colours
	COLORREF mpl_ListTextColour;
	COLORREF mpl_ListTextColour_Selected;
	COLORREF mpl_ListTextColour_HotItem;
	COLORREF mpl_ListHeaderColour;
	
	// List control
	CPs_Image* mpl_pListBackground;
	RECT mpl_rListBackground_SourceTile;
	CPs_Image* mpl_pListHeader_Up;
	CPs_Image* mpl_pListHeader_Down;
	RECT mpl_rListHeader_SourceTile;
	
	// Scrollbar - Horiz
	CPs_Image* mpl_pHScrollBar_Bk;
	RECT mpl_rHScrollBar_Bk_Tile;
	CPs_Image* mpl_pHScrollBar_TrackUp;
	CPs_Image* mpl_pHScrollBar_TrackDn;
	RECT mpl_rHScrollBar_Track_Tile;
	
	CPs_Image_WithState* mpl_pHScrollBar_Left;
	CPs_Image_WithState* mpl_pHScrollBar_Right;
	
	// Scrollbar - Vert
	CPs_Image* mpl_pVScrollBar_Bk;
	RECT mpl_rVScrollBar_Bk_Tile;
	CPs_Image* mpl_pVScrollBar_TrackUp;
	CPs_Image* mpl_pVScrollBar_TrackDn;
	RECT mpl_rVScrollBar_Track_Tile;
	
	CPs_Image_WithState* mpl_pVScrollBar_Up;
	CPs_Image_WithState* mpl_pVScrollBar_Down;
	
	// Selection
	CPs_Image* mpl_pSelection;
	RECT mpl_rSelection_Tile;
	CPs_Image* mpl_pFocus;
	RECT mpl_rFocus_Tile;
	
	CPs_CommandTarget* mpl_pCommandTargets;
	SIZE mpl_szMinWindow;
	
	CPs_Indicator* mpl_pIndicators;
} CPs_Skin;

//
////////////////////////////////////////////////////////////////////////////////
extern CPs_Skin* glb_pSkin;


////////////////////////////////////////////////////////////////////////////////
//
void CPSK_Initialise();
void CPSK_Uninitialise();
//
////////////////////////////////////////////////////////////////////////////////
