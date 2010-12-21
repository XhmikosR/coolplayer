
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
// Image Support
//
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//

typedef struct _CPs_Image
{
	HBITMAP m_hbmImage;
	SIZE m_szSize;
	
} CPs_Image;

//
typedef enum _CPe_ImageState
{
	igsQuiescent = 0,
	igsActive = 1,
	igsFloatActive = 2,
	
	igsLast = 2
	
} CPe_ImageState;
//

typedef struct _CPs_Image_WithState
{
	CPs_Image* m_pImage;
	int m_iStateHeight;
	POINT m_ptSource[igsLast+1];
	
} CPs_Image_WithState;

//

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
#define CIC_TILEDFILOPTIONS_NONE  0
#define CIC_TILEDFILOPTIONS_NOCENTRE 1
//
CPs_Image* CPIG_CreateImage_FromFile(const char* pcFilename);
CPs_Image* CPIG_CreateImage_FromSubFile(CP_COMPOSITEFILE hmComposite, const char* pcSubFilename);
CPs_Image* CPIG_CreateImage_FromResource(const UINT uiResourceID);
CPs_Image_WithState* CPIG_CreateStateImage(CPs_Image* pSource, const int iNumStates);
void CPIG_DestroyImage(CPs_Image* pImage);
void CPIG_DestroyImage_WithState(CPs_Image_WithState* pImage);
void CPIG_TiledFill(CPs_DrawContext* pDC, const RECT* prTarget, const RECT* prSourceRect, CPs_Image* pSourceImage, const DWORD dwOptions);
void CPIG_DrawStateImage(CPs_DrawContext* pDC, const int iX, const int iY, CPs_Image_WithState* pSource, const CPe_ImageState enState);
void CPIG_DrawImage(CPs_DrawContext* pDC, const int iX, const int iY, CPs_Image* pSource);
//
////////////////////////////////////////////////////////////////////////////////
