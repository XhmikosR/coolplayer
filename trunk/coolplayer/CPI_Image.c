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
#include "CompositeFile.h"


////////////////////////////////////////////////////////////////////////////////
//
//
//
CPs_Image* CPIG_CreateImage_FromFile(const char* pcFilename)
{
	HBITMAP hbmLoad;
	BITMAP bmLoad;
	CPs_Image* pNewImage;
	char cFilename[MAX_PATH];
	
	strcpy(cFilename, "P:\\Skin\\");
	strcat(cFilename, pcFilename);
	
	hbmLoad = LoadImage(NULL, cFilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	
	if (!hbmLoad)
		return NULL;
		
	// Get bitmap properties
	GetObject(hbmLoad, sizeof(bmLoad), &bmLoad);
	
	// Setup Image struct
	pNewImage = (CPs_Image*)malloc(sizeof(CPs_Image));
	
	pNewImage->m_hbmImage = hbmLoad;
	pNewImage->m_szSize.cx = bmLoad.bmWidth;
	pNewImage->m_szSize.cy = bmLoad.bmHeight;
	
	return pNewImage;
}

//
//
//
CPs_Image* CPIG_CreateImage_FromSubFile(CP_COMPOSITEFILE hmComposite, const char* pcSubFilename)
{
	void* pFileData;
	unsigned int iLength;
	BOOL bSucceeded;
	HBITMAP hbmLoad;
	SIZE szBitmap;
	CPs_Image* pNewImage;
	
	bSucceeded = CF_GetSubFile(hmComposite, pcSubFilename, &pFileData, &iLength);
	
	if (!bSucceeded)
		return NULL;
		
	// Decode bitmap
	{
		BITMAPFILEHEADER* pFileHeader = (BITMAPFILEHEADER*)pFileData;
		BITMAPINFO* pBitmapInfo;
		void* pBitmapBytes;
		HDC dcDisplay, dcDraw;
		
		// Check bitmap cookie
		
		if (pFileHeader->bfType != 0x4D42)
		{
			free(pFileData);
			return NULL;
		}
		
		// Upload bitmap to a DDB
		pBitmapInfo = (BITMAPINFO*)(((BYTE*)pFileData) + sizeof(BITMAPFILEHEADER));
		
		pBitmapBytes = ((BYTE*)pFileData) + pFileHeader->bfOffBits;
		
		dcDisplay = GetDC(NULL);
		
		szBitmap.cx = pBitmapInfo->bmiHeader.biWidth;
		szBitmap.cy = pBitmapInfo->bmiHeader.biHeight;
		
		hbmLoad = CreateCompatibleBitmap(dcDisplay, szBitmap.cx, szBitmap.cy);
		
		dcDraw = CreateCompatibleDC(dcDisplay);
		
		ReleaseDC(NULL, dcDisplay);
		
		SetDIBits(dcDraw, hbmLoad, 0, pBitmapInfo->bmiHeader.biHeight,
				  pBitmapBytes, pBitmapInfo, DIB_RGB_COLORS);
		          
		// Cleanup
		DeleteDC(dcDraw);
	}
	
	free(pFileData);
	
	// Setup Image struct
	pNewImage = (CPs_Image*)malloc(sizeof(CPs_Image));
	pNewImage->m_hbmImage = hbmLoad;
	pNewImage->m_szSize = szBitmap;
	return pNewImage;
}

//
//
//
CPs_Image* CPIG_CreateImage_FromResource(const UINT uiResourceID)
{
	HBITMAP hbmLoad;
	BITMAP bmLoad;
	CPs_Image* pNewImage;
	
	hbmLoad = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(uiResourceID));
	
	if (!hbmLoad)
		return NULL;
		
	// Get bitmap properties
	GetObject(hbmLoad, sizeof(bmLoad), &bmLoad);
	
	// Setup Image struct
	pNewImage = (CPs_Image*)malloc(sizeof(CPs_Image));
	
	pNewImage->m_hbmImage = hbmLoad;
	pNewImage->m_szSize.cx = bmLoad.bmWidth;
	pNewImage->m_szSize.cy = bmLoad.bmHeight;
	
	return pNewImage;
}

//
//
//
CPs_Image_WithState* CPIG_CreateStateImage(CPs_Image* pSource, const int iNumStates)
{
	CPs_Image_WithState* pNewIS;
	int iStateIDX;
	
	CP_ASSERT((iNumStates - 1) <= igsLast);
	
	pNewIS = (CPs_Image_WithState*)malloc(sizeof(*pNewIS));
	pNewIS->m_pImage = pSource;
	pNewIS->m_iStateHeight = pSource->m_szSize.cy / iNumStates;
	memset(pNewIS->m_ptSource, 0, sizeof(pNewIS->m_ptSource));
	
	for (iStateIDX = 0; iStateIDX < iNumStates; iStateIDX++)
	{
		pNewIS->m_ptSource[iStateIDX].x = 0;
		pNewIS->m_ptSource[iStateIDX].y = iStateIDX * pNewIS->m_iStateHeight;
	}
	
	return pNewIS;
}

//
//
//
void CPIG_DestroyImage(CPs_Image* pImage)
{
	if (!pImage)
		return;
		
	CP_ASSERT(pImage->m_hbmImage);
	
	DeleteObject(pImage->m_hbmImage);
	
	free(pImage);
}

//
//
//
void CPIG_DestroyImage_WithState(CPs_Image_WithState* pImageWithState)
{
	if (!pImageWithState)
		return;
		
	CP_ASSERT(pImageWithState->m_pImage);
	
	CPIG_DestroyImage(pImageWithState->m_pImage);
	
	free(pImageWithState);
}

//
//
//
void CPIG_DrawStateImage(CPs_DrawContext* pDC, const int iX, const int iY, CPs_Image_WithState* pSource, const CPe_ImageState enState)
{
	HDC dcCompat;
	HBITMAP bmOld;
	CP_CHECKOBJECT(pSource);
	
	dcCompat = CreateCompatibleDC(pDC->m_dcDraw);
	bmOld = (HBITMAP)SelectObject(dcCompat, pSource->m_pImage->m_hbmImage);
	SetBkColor(dcCompat, glb_pSkin->m_clrTransparent);
	BitBlt(pDC->m_dcDraw, iX + pDC->m_ptOffset.x, iY + pDC->m_ptOffset.y, pSource->m_pImage->m_szSize.cx, pSource->m_iStateHeight,
		   dcCompat, pSource->m_ptSource[enState].x, pSource->m_ptSource[enState].y, SRCCOPY);
	SelectObject(dcCompat, bmOld);
	DeleteDC(dcCompat);
}

//
//
//
void CPIG_DrawImage(CPs_DrawContext* pDC, const int iX, const int iY, CPs_Image* pSource)
{
	HDC dcCompat;
	HBITMAP bmOld;
	CP_CHECKOBJECT(pSource);
	
	dcCompat = CreateCompatibleDC(pDC->m_dcDraw);
	bmOld = (HBITMAP)SelectObject(dcCompat, pSource->m_hbmImage);
	SetBkColor(dcCompat, glb_pSkin->m_clrTransparent);
	BitBlt(pDC->m_dcDraw, iX + pDC->m_ptOffset.x, iY + pDC->m_ptOffset.y, pSource->m_szSize.cx, pSource->m_szSize.cy, dcCompat, 0, 0, SRCCOPY);
	SelectObject(dcCompat, bmOld);
	DeleteDC(dcCompat);
}

//
//
//
void CPIG_TiledFill(CPs_DrawContext* pDC, const RECT* _prTarget, const RECT* prSourceRect, CPs_Image* pSourceImage, const DWORD dwOptions)
{
	HDC dcCompat;
	HBITMAP bmOld;
	RECT rSourceBorders;
	RECT rTarget;
	RECT rLocalClip;
	CP_CHECKOBJECT(pSourceImage);
	
	// Skip this draw if we are totally clipped
	
	if (_prTarget->right < pDC->m_rClip.left
			|| _prTarget->bottom < pDC->m_rClip.top
			|| _prTarget->left > pDC->m_rClip.right
			|| _prTarget->top > pDC->m_rClip.bottom)
	{
		return;
	}
	
	// Prepare local rects
	rTarget = *_prTarget;
	OffsetRect(&rTarget, pDC->m_ptOffset.x, pDC->m_ptOffset.y);
	
	rLocalClip = pDC->m_rClip;
	OffsetRect(&rLocalClip, pDC->m_ptOffset.x, pDC->m_ptOffset.y);
	
	dcCompat = CreateCompatibleDC(pDC->m_dcDraw);
	
	bmOld = (HBITMAP)SelectObject(dcCompat, pSourceImage->m_hbmImage);
	
	SetBkColor(dcCompat, glb_pSkin->m_clrTransparent);
	
	// Prepare source border widths
	rSourceBorders.left = prSourceRect->left;
	rSourceBorders.right = pSourceImage->m_szSize.cx - prSourceRect->right;
	rSourceBorders.top = prSourceRect->top;
	rSourceBorders.bottom = pSourceImage->m_szSize.cy - prSourceRect->bottom;
	
	// Draw top row
	if (rSourceBorders.top > 0
			&& ((rTarget.top + rSourceBorders.top) >= rLocalClip.top && rTarget.top <= rLocalClip.bottom))
	{
		// - corners
		BitBlt(pDC->m_dcDraw, rTarget.left, rTarget.top, rSourceBorders.left, rSourceBorders.top, dcCompat, 0, 0, SRCCOPY);
		BitBlt(pDC->m_dcDraw, rTarget.right - rSourceBorders.right, rTarget.top,
			   rSourceBorders.right, rSourceBorders.top,
			   dcCompat, prSourceRect->right, 0, SRCCOPY);
		       
		// - tiled middle bit
		
		if (prSourceRect->left < prSourceRect->right)
		{
			int iSourceCursorPos;
			int iTileWidth;
			int iEndCol;
			
			iSourceCursorPos = rSourceBorders.left + rTarget.left;
			iEndCol = rTarget.right - rSourceBorders.right;
			
			while (iSourceCursorPos < iEndCol)
			{
				// Decide tile size
				iTileWidth = (prSourceRect->right - prSourceRect->left);
				
				if ((iSourceCursorPos + iTileWidth) > iEndCol)
					iTileWidth = iEndCol - iSourceCursorPos;
					
				// Draw tile
				BitBlt(pDC->m_dcDraw, iSourceCursorPos, rTarget.top,
					   iTileWidth, rSourceBorders.top,
					   dcCompat, prSourceRect->left, 0, SRCCOPY);
				       
				iSourceCursorPos += iTileWidth;
			}
		}
	}
	
	// Draw bottom row
	
	if (rSourceBorders.bottom > 0
			&& (rTarget.bottom >= rLocalClip.top && (rTarget.bottom - rSourceBorders.bottom) <= rLocalClip.bottom))
	{
		// - corners
		BitBlt(pDC->m_dcDraw, rTarget.left, rTarget.bottom - rSourceBorders.bottom,
			   rSourceBorders.left, rSourceBorders.bottom, dcCompat, 0, prSourceRect->bottom, SRCCOPY);
		BitBlt(pDC->m_dcDraw, rTarget.right - rSourceBorders.right, rTarget.bottom - rSourceBorders.bottom,
			   rSourceBorders.right, rSourceBorders.bottom,
			   dcCompat, prSourceRect->right, prSourceRect->bottom, SRCCOPY);
		// - tiled middle bit
		
		if (prSourceRect->left < prSourceRect->right)
		{
			int iSourceCursorPos;
			int iTileWidth;
			int iEndCol;
			
			iSourceCursorPos = rSourceBorders.left + rTarget.left;
			iEndCol = rTarget.right - rSourceBorders.right;
			
			while (iSourceCursorPos < iEndCol)
			{
				// Decide tile size
				iTileWidth = (prSourceRect->right - prSourceRect->left);
				
				if ((iSourceCursorPos + iTileWidth) > iEndCol)
					iTileWidth = iEndCol - iSourceCursorPos;
					
				// Draw tile
				BitBlt(pDC->m_dcDraw, iSourceCursorPos, rTarget.bottom - rSourceBorders.bottom,
					   iTileWidth, rSourceBorders.bottom,
					   dcCompat, prSourceRect->left, prSourceRect->bottom, SRCCOPY);
				       
				iSourceCursorPos += iTileWidth;
			}
		}
	}
	
	// Draw intermediate rows
	
	if (prSourceRect->top < prSourceRect->bottom
			&& prSourceRect->bottom > prSourceRect->top)
	{
		int iSourceCursorPos_Y;
		int iTileHeight;
		int iEndRow;
		iSourceCursorPos_Y = rSourceBorders.top + rTarget.top;
		iEndRow = rTarget.bottom - rSourceBorders.bottom;
		
		while (iSourceCursorPos_Y < iEndRow)
		{
			// Decide tile height
			iTileHeight = (prSourceRect->bottom - prSourceRect->top);
			
			if ((iSourceCursorPos_Y + iTileHeight) > iEndRow)
				iTileHeight = iEndRow - iSourceCursorPos_Y;
				
			// Draw row
			if ((iSourceCursorPos_Y + iTileHeight) >= rLocalClip.top && iSourceCursorPos_Y <= rLocalClip.bottom)
			{
				// - left and right bands
				BitBlt(pDC->m_dcDraw, rTarget.left, iSourceCursorPos_Y, rSourceBorders.left, iTileHeight, dcCompat, 0, rSourceBorders.top, SRCCOPY);
				BitBlt(pDC->m_dcDraw, rTarget.right - rSourceBorders.right, iSourceCursorPos_Y,
					   rSourceBorders.right, iTileHeight,
					   dcCompat, prSourceRect->right, rSourceBorders.top, SRCCOPY);
				       
				// Middle tiles
				
				if (prSourceRect->left < prSourceRect->right
						&& (dwOptions & CIC_TILEDFILOPTIONS_NOCENTRE) == 0)
				{
					int iSourceCursorPos;
					int iTileWidth;
					int iEndCol;
					
					iSourceCursorPos = rSourceBorders.left + rTarget.left;
					iEndCol = rTarget.right - rSourceBorders.right;
					
					while (iSourceCursorPos < iEndCol)
					{
						// Decide tile size
						iTileWidth = (prSourceRect->right - prSourceRect->left);
						
						if ((iSourceCursorPos + iTileWidth) > iEndCol)
							iTileWidth = iEndCol - iSourceCursorPos;
							
						// Draw tile
						BitBlt(pDC->m_dcDraw, iSourceCursorPos, iSourceCursorPos_Y,
							   iTileWidth, iTileHeight,
							   dcCompat, prSourceRect->left, rSourceBorders.top, SRCCOPY);
						       
						iSourceCursorPos += iTileWidth;
					}
				}
			}
			
			
			iSourceCursorPos_Y += iTileHeight;
		} // end while
	}
	
	SelectObject(dcCompat, bmOld);
	
	DeleteDC(dcCompat);
}

//
//
//
