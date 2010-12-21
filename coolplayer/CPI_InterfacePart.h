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
// Player verbs
//
// - This will contain all the verbs (commands) for the player - the verb handlers
// are also capable of returning it's skin def name and it's legacy skin def name
//
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//
typedef void (*wp_Part_Destroy_PrivateData)(CP_HINTERFACEPART hPart);
typedef void (*wp_Part_Draw)(CP_HINTERFACEPART hPart, CPs_DrawContext* pContext);
typedef void (*wp_Part_onMouseIn)(CP_HINTERFACEPART hPart);
typedef void (*wp_Part_onMouseOut)(CP_HINTERFACEPART hPart);
typedef void (*wp_Part_onMouseMove)(CP_HINTERFACEPART hPart, const POINTS ptMouse);
typedef void (*wp_Part_onMouseButton_LDown)(CP_HINTERFACEPART hPart, const POINTS ptMouse);
typedef void (*wp_Part_onMouseButton_LUp)(CP_HINTERFACEPART hPart, const POINTS ptMouse);
typedef void (*wp_Part_onMouseButton_RDown)(CP_HINTERFACEPART hPart, const POINTS ptMouse);
typedef void (*wp_Part_onMouseButton_RUp)(CP_HINTERFACEPART hPart, const POINTS ptMouse);
typedef void (*wp_Part_onTimer)(CP_HINTERFACEPART hPart);
typedef void (*wp_Part_onSongChange)(CP_HINTERFACEPART hPart);
//
#define CPC_IP_ALIGN_LEFT 0x1
#define CPC_IP_ALIGN_RIGHT 0x2
#define CPC_IP_ALIGN_TOP 0x4
#define CPC_IP_ALIGN_BOTTOM 0x8
//

typedef struct _CPs_InterfacePart
{
	// Methods
	wp_Part_Destroy_PrivateData Destroy_PrivateData;
	wp_Part_Draw Draw;
	
	// Notifies
	wp_Part_onMouseIn onMouseIn;
	wp_Part_onMouseOut onMouseOut;
	wp_Part_onMouseMove onMouseMove;
	wp_Part_onMouseButton_LDown onMouseButton_LDown;
	wp_Part_onMouseButton_LUp onMouseButton_LUp;
	wp_Part_onMouseButton_RDown onMouseButton_RDown;
	wp_Part_onMouseButton_RUp onMouseButton_RUp;
	wp_Part_onTimer onTimer;
	wp_Part_onSongChange onSongChange;
	
	// Data
	RECT m_rLocation;
	DWORD m_dwAlign;
	
	BOOL m_bRectAlignMode;
	RECT m_rPosition;
	POINT m_ptOffset;
	SIZE m_szSize;
	
	CP_HINTERFACE m_hOwner;
	
	void* m_pPrivateData;
	
	// Link to next part
	CP_HINTERFACEPART m_hNext;
	
} CPs_InterfacePart;

//
//
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
void IP_Invalidate(CP_HINTERFACEPART hPart);
void IP_Destroy(CP_HINTERFACEPART hPart);
CP_HINTERFACEPART IP_Create_CommandButton(wp_Verb pfnVerb, CPs_Image_WithState* pImageWS);
CP_HINTERFACEPART IP_Create_Indicator(const char* pcName);
//
////////////////////////////////////////////////////////////////////////////////
