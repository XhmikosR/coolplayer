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

#include "stdafx.h"
#include "globals.h"
#define CPC_RECT_QUANTISE	4096

HRESULT path_create_link(LPCSTR lpszPathObj, LPSTR lpszPathLink, LPSTR lpszDesc)
{
	HRESULT hres;
	IShellLink *psl;
	
	// Get a pointer to the IShellLink interface.
	hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (void **) &psl);
	if (SUCCEEDED(hres)) 
	{
		IPersistFile *ppf;
		// Set the path to the shortcut target and add the
		// description.
		psl->lpVtbl->SetPath(psl, lpszPathObj);
		psl->lpVtbl->SetDescription(psl, lpszDesc);
		// Query IShellLink for the IPersistFile interface for saving the
		// shortcut in persistent storage.
		hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (PVOID *) &ppf);
		if (SUCCEEDED(hres)) 
		{
			WORD    wsz[MAX_PATH];	// Ensure that the string is ANSI.
			MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz,
			MAX_PATH);
			// Save the link by calling IPersistFile::Save.
			hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
			ppf->lpVtbl->Release(ppf);
		}
		psl->lpVtbl->Release(psl);
	}
	return hres;
}
//
//
//
HRGN main_bitmap_to_region(HBITMAP hBmp, COLORREF cTransparentColor)
{
	HRGN hRgn = NULL;
	DWORD* pBitmapBits = NULL;
	DWORD* pBitmapCursor;
	BITMAP bitmap;
	RGNDATA* pRGNData = NULL;
	// RECT* pRectArray = NULL;
	int iLastRectIDX;
	int iRGNDataSize_Rects;
	int iRowIDX, iColIDX;
	DWORD dwTransMasked;
	BOOL bDetectedTransparentPixel;

	CP_ASSERT(hBmp);

	// Get the size of the source
	GetObject(hBmp, sizeof(bitmap), &bitmap);
	pBitmapBits = (DWORD*)malloc(sizeof(DWORD) * bitmap.bmWidth * bitmap.bmHeight);

	// Extract the bits of the bitmap
	{
		BITMAPINFO bmi;
		HDC dc;

		memset(&bmi, 0, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biWidth = bitmap.bmWidth;
		bmi.bmiHeader.biHeight = -bitmap.bmHeight;

		dc = CreateCompatibleDC(NULL);
		GetDIBits(dc, hBmp, 0, bitmap.bmHeight, pBitmapBits, &bmi, DIB_RGB_COLORS);
		DeleteDC(dc);
	}

	// Step through bitmap row by row - building rects for the rows that arn't the
	// transparent colour
	dwTransMasked = cTransparentColor & 0x00FFFFFF;
	pBitmapCursor = pBitmapBits;
	iLastRectIDX = 0;
	iRGNDataSize_Rects = 0;
	bDetectedTransparentPixel = FALSE;
	for (iRowIDX =0; iRowIDX < bitmap.bmHeight; iRowIDX++)
	{
		BOOL bInStrip = FALSE;
		for (iColIDX =0; iColIDX < bitmap.bmWidth; iColIDX++, pBitmapCursor++)
		{
			// Is the current pixel transparent?
			if ((((*pBitmapCursor)&0x00FFFFFF)^dwTransMasked) == 0L)
			{
				bDetectedTransparentPixel = TRUE;
				// If we are in a strip - close it
				if(bInStrip == TRUE)
				{
					bInStrip = FALSE;
					((RECT*)pRGNData->Buffer)[iLastRectIDX].right = iColIDX;
					iLastRectIDX++;
				}
			}
			else
			{
				// Open a new strip if we need to
				if (bInStrip == FALSE)
				{
					bInStrip = TRUE;

					// Ensure that we have enough memory allocated
					if (iLastRectIDX == iRGNDataSize_Rects)
					{
						iRGNDataSize_Rects += CPC_RECT_QUANTISE;
						pRGNData = (RGNDATA*)realloc(pRGNData, sizeof(RGNDATAHEADER) + (iRGNDataSize_Rects * sizeof(RECT)));
					}
					((RECT*)pRGNData->Buffer)[iLastRectIDX].left = iColIDX;
					((RECT*)pRGNData->Buffer)[iLastRectIDX].top = iRowIDX;
					((RECT*)pRGNData->Buffer)[iLastRectIDX].bottom = iRowIDX+1;
				}
			}
		} // end for column

		// Close any open rects
		if (bInStrip == TRUE)
		{
			((RECT*)pRGNData->Buffer)[iLastRectIDX].right = bitmap.bmWidth;
			iLastRectIDX++;
		}
	} // end for row
	free(pBitmapBits);

	// If there are some rects in this region - create the GDI object
	if (bDetectedTransparentPixel == TRUE)
	{
		pRGNData->rdh.dwSize = sizeof(RGNDATAHEADER);
		pRGNData->rdh.iType = RDH_RECTANGLES;
		pRGNData->rdh.nCount = iLastRectIDX;
		pRGNData->rdh.nRgnSize = sizeof(RGNDATAHEADER) + (iLastRectIDX * sizeof(RECT));
		pRGNData->rdh.rcBound.left = 0;
		pRGNData->rdh.rcBound.top = 0;
		pRGNData->rdh.rcBound.right = bitmap.bmWidth;
		pRGNData->rdh.rcBound.bottom = bitmap.bmHeight;
		hRgn = ExtCreateRegion(NULL, pRGNData->rdh.nRgnSize, pRGNData);
	}

	// Cleanup
	if (pRGNData)
		free(pRGNData);
	return hRgn;
}
//
//
//
HRGN main_bitmap_to_region_1bit(HBITMAP hBmp, COLORREF cTransparentColor)
{
	HRGN hRgn = NULL;
	BYTE* pBitmapBits = NULL;
	BYTE* pBitmapCursor;
	BITMAP bitmap;
	RGNDATA* pRGNData = NULL;
	// RECT* pRectArray = NULL;
	int iStride;
	int iLastRectIDX;
	int iRGNDataSize_Rects;
	int iEndofLineCorrection;
	int iRowIDX, iColIDX;
	DWORD dwTransMask;
	BOOL bDetectedTransparentPixel;

	CP_ASSERT(hBmp);

	// Get the size of the source
	GetObject(hBmp, sizeof(bitmap), &bitmap);

	// Work out the byte aligned stride of the bitmap
	iStride = bitmap.bmWidth >> 3;
	if (bitmap.bmWidth & 0x7)
		iStride++;
	if (iStride&0x3)
	{
		iEndofLineCorrection = 0x4 - (iStride&0x3);
		iStride += iEndofLineCorrection;
	}
	else
		iEndofLineCorrection = 0;

	pBitmapBits = (BYTE*)malloc(iStride * bitmap.bmHeight);

	// Extract the bits of the bitmap
	{
		BITMAPINFO* pBMI;
		HDC dc;

		pBMI = malloc(sizeof(BITMAPINFO) + (sizeof(RGBQUAD)<<1));
		memset(pBMI, 0, sizeof(*pBMI));
		pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pBMI->bmiHeader.biPlanes = 1;
		pBMI->bmiHeader.biBitCount = 1;
		pBMI->bmiHeader.biCompression = BI_RGB;
		pBMI->bmiHeader.biWidth = bitmap.bmWidth;
		pBMI->bmiHeader.biHeight = -bitmap.bmHeight;
		pBMI->bmiHeader.biSizeImage = iStride * bitmap.bmHeight;

		dc = CreateCompatibleDC(NULL);
		GetDIBits(dc, hBmp, 0, bitmap.bmHeight, pBitmapBits, pBMI, DIB_RGB_COLORS);
		free(pBMI);
		DeleteDC(dc);
	}

	bDetectedTransparentPixel = FALSE;

	// Step through bitmap row by row - building rects for the rows that arn't the
	// transparent colour
	pBitmapCursor = pBitmapBits;
	iLastRectIDX = 0;
	iRGNDataSize_Rects = 0;
	dwTransMask = 0x80;
	for (iRowIDX =0; iRowIDX < bitmap.bmHeight; iRowIDX++)
	{
		BOOL bInStrip = FALSE;
		for (iColIDX =0; iColIDX < bitmap.bmWidth; iColIDX++)
		{
			// Is the current pixel transparent?
			if( (*pBitmapCursor) & dwTransMask)
			{
				bDetectedTransparentPixel = TRUE;
				// If we are in a strip - close it
				if(bInStrip == TRUE)
				{
					bInStrip = FALSE;
					((RECT*)pRGNData->Buffer)[iLastRectIDX].right = iColIDX;
					iLastRectIDX++;
				}
			}
			else
			{
				// Open a new strip if we need to
				if (bInStrip == FALSE)
				{
					bInStrip = TRUE;

					// Ensure that we have enough memory allocated
					if (iLastRectIDX == iRGNDataSize_Rects)
					{
						iRGNDataSize_Rects += CPC_RECT_QUANTISE;
						pRGNData = (RGNDATA*)realloc(pRGNData, sizeof(RGNDATAHEADER) + (iRGNDataSize_Rects * sizeof(RECT)));
					}
					((RECT*)pRGNData->Buffer)[iLastRectIDX].left = iColIDX;
					((RECT*)pRGNData->Buffer)[iLastRectIDX].top = iRowIDX;
					((RECT*)pRGNData->Buffer)[iLastRectIDX].bottom = iRowIDX+1;
				}
			}

			// Advance to next pixel
			dwTransMask >>= 1;
			if (!dwTransMask)
			{
				dwTransMask = 0x80;
				pBitmapCursor++;
			}
		} // end for column

		// Close any open rects
		if (bInStrip == TRUE)
		{
			((RECT*)pRGNData->Buffer)[iLastRectIDX].right = bitmap.bmWidth;
			iLastRectIDX++;
		}

		// Skip to the start of the next line
		if (dwTransMask != 0x80)
			pBitmapCursor++;

		dwTransMask = 0x80;
		pBitmapCursor += iEndofLineCorrection;
	} // end for row
	free(pBitmapBits);

	// If there are some rects in this region - create the GDI object
	if (bDetectedTransparentPixel == TRUE)
	{
		pRGNData->rdh.dwSize = sizeof(RGNDATAHEADER);
		pRGNData->rdh.iType = RDH_RECTANGLES;
		pRGNData->rdh.nCount = iLastRectIDX;
		pRGNData->rdh.nRgnSize = sizeof(RGNDATAHEADER) + (iLastRectIDX * sizeof(RECT));
		pRGNData->rdh.rcBound.left = 0;
		pRGNData->rdh.rcBound.top = 0;
		pRGNData->rdh.rcBound.right = bitmap.bmWidth;
		pRGNData->rdh.rcBound.bottom = bitmap.bmHeight;
		hRgn = ExtCreateRegion(NULL, pRGNData->rdh.nRgnSize, pRGNData);
	}

	// Cleanup
	if (pRGNData)
		free(pRGNData);
	return hRgn;
}
