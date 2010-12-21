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
#include "CPI_Player_CoDec.h"
#include "CP_WinAmpStructs.h"
#include "CPI_CircleBuffer.h"

////////////////////////////////////////////////////////////////////////////////
//
// This is the CoDec module - the basic idea is that the file will be opened and
// data will be sucked through the CoDec via calls to CPI_CoDec__GetPCMBlock
//
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
//
// Module functions
void CPP_OMAPLG_Uninitialise(CPs_CoDecModule* pModule);
BOOL CPP_OMAPLG_OpenFile(CPs_CoDecModule* pModule, const char* pcFilename, DWORD dwCookie, HWND hWndOwner);
void CPP_OMAPLG_CloseFile(CPs_CoDecModule* pModule);
void CPP_OMAPLG_Seek(CPs_CoDecModule* pModule, const int iNumerator, const int iDenominator);
void CPP_OMAPLG_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo* pInfo);
//
BOOL CPP_OMAPLG_GetPCMBlock(CPs_CoDecModule* pModule, void* pBlock, DWORD* pdwBlockSize);
int CPP_OMAPLG_GetCurrentPos_secs(CPs_CoDecModule* pModule);
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Dummy functions used by the in module (but we don't need 'em)
void CP_Dummy_SAVSAInit(int maxlatency_in_ms, int srate) {}

void CP_Dummy_SAVSADeInit() {}

void CP_Dummy_SAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {}

int CP_Dummy_SAGetMode()
{
	return 0;
}

void CP_Dummy_SAAdd(void *data, int timestamp, int csa) {}

void CP_Dummy_VSAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {}

int CP_Dummy_VSAGetMode(int *specNch, int *waveNch)
{
	return 0;
}

void CP_Dummy_VSAAdd(void *data, int timestamp) {}

int CP_Dummy_dsp_isactive()
{
	return 0;
}

int CP_Dummy_dsp_dosamples(short int *samples, int numsamples, int bps, int nch, int srate)
{
	return 0;
}

//
void CP_Dummy_VSASetInfo(int nch, int srate) {}

int CP_Dummy_SetInfo(int bitrate, int srate, int stereo, int synched)
{
	return 0;
}

//
// Dummy output functions
int CP_Dummy_Pause(int pause)
{
	return 0;
}

void CP_Dummy_SetVolume(int volume) {}

void CP_Dummy_SetPan(int pan) {}

int CP_Dummy_GetOutputTime()
{
	return 0;
}

int CP_Dummy_GetWrittenTime()
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//

typedef struct __CPs_CoDec_WinAmpPlugin
{
	CP_PlugInModule* m_pFirstPlugIn;
	
	CP_PlugInModule* m_pActivePluginModule;
	HMODULE m_hModPlugin;
	In_Module* m_pInModule;
	BOOL m_bModuleIsPlaying;
	
	Out_Module m_FakeOutModule;
	
} CPs_CoDec_WinAmpPlugin;

//
////////////////////////////////////////////////////////////////////////////////


#define CIC_CIRCLE_DATABUFFER_SIZE  0x10000  //64Kb
#define CIC_WAITTIMEOUT     2000  // 2Secs
////////////////////////////////////////////////////////////////////////////////
// Callback interface (WinAmp doesn't "DO" cookies so we have to have this global
// variable) - this means that we can only have one thread (the plugins prob aren't
// thread safe anyhow) - but there we go!
//

struct CPs_OutputGlobalData
{
	CRITICAL_SECTION m_csGlobal;
	BOOL m_bStreamIsComplete;
	int m_iCurrentTime_ms;
	
	// Stuff for the file information
	HANDLE m_evtFileInfoValid;
	CPs_FileInfo m_FileInfo;
	
	// Stuff for the data buffer
	CPs_CircleBuffer* m_pCBuffer;
	
	HANDLE m_evtSeekComplete;
	
} glb_OutputData;

//
//
//
int CP_OutPI_Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	// returns >=0 on success, <0 on failure
	// NOTENOTENOTE: bufferlenms and prebufferms are ignored in most if not all output plug-ins.
	//    ... so don't expect the max latency returned to be what you asked for.
	// returns max latency in ms (0 for diskwriters, etc)
	// bufferlenms and prebufferms must be in ms. 0 to use defaults.
	// prebufferms must be <= bufferlenms
	
	if (bitspersamp != 8 && bitspersamp != 16)
	{
		CP_TRACE0("Inappropriate stream type");
		return -1;
	}
	
	if (numchannels != 1 && numchannels != 2)
	{
		CP_TRACE0("Inappropriate stream type");
		return -1;
	}
	
	// Setup the global info struct
	{
		EnterCriticalSection(&glb_OutputData.m_csGlobal);
		
		glb_OutputData.m_FileInfo.m_iFileLength_Secs = 0;
		glb_OutputData.m_FileInfo.m_iBitRate_Kbs = 0;
		glb_OutputData.m_FileInfo.m_iFreq_Hz = samplerate;
		glb_OutputData.m_FileInfo.m_bStereo = (numchannels > 1) ? TRUE : FALSE;
		glb_OutputData.m_FileInfo.m_b16bit = (bitspersamp == 16) ? TRUE : FALSE;
		
		LeaveCriticalSection(&glb_OutputData.m_csGlobal);
		
		SetEvent(glb_OutputData.m_evtFileInfoValid);
	}
	
	return 1;
}

//
//
//
void CP_OutPI_Close()
{
	CP_TRACE0("CP_OutPI_Close");
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	
	if (glb_OutputData.m_pCBuffer)
		glb_OutputData.m_pCBuffer->SetComplete(glb_OutputData.m_pCBuffer);
		
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
}

//
//
//
int CP_OutPI_CanWrite()
{
	int iNumBytesFree;
	// returns number of bytes possible to write at a given time.
	// Never will decrease unless you call Write (or Close, heh)
	
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	
	if (glb_OutputData.m_pCBuffer == NULL)
		iNumBytesFree = 0xFFFFFFFF;
	else
		iNumBytesFree = glb_OutputData.m_pCBuffer->GetFreeSize(glb_OutputData.m_pCBuffer);
		
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
	
	return iNumBytesFree;
}

//
//
//
int CP_OutPI_Write(char *buf, int len)
{
	// 0 on success. Len == bytes to write (<= 8192 always). buf is straight audio data.
	// 1 returns not able to write (yet). Non-blocking, always.
	if (len > CP_OutPI_CanWrite())
		return 1;
		
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	
	if (glb_OutputData.m_pCBuffer)
		glb_OutputData.m_pCBuffer->Write(glb_OutputData.m_pCBuffer, buf, len);
		
	// Update the current time
	{
		int iBytesPerSample = (glb_OutputData.m_FileInfo.m_bStereo ? 2 : 1)
							  << (glb_OutputData.m_FileInfo.m_b16bit ? 1 : 0);
		                      
		glb_OutputData.m_iCurrentTime_ms += ((len / iBytesPerSample) * 1000) / glb_OutputData.m_FileInfo.m_iFreq_Hz;
	}
	
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
	
	return 0;
}

//
//
//
void CP_OutPI_Flush(int iNewTime)
{
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	
	if (glb_OutputData.m_pCBuffer)
		glb_OutputData.m_pCBuffer->Flush(glb_OutputData.m_pCBuffer);
		
	glb_OutputData.m_iCurrentTime_ms = iNewTime;
	
	SetEvent(glb_OutputData.m_evtSeekComplete);
	
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
}

//
//
//
int CP_OutPI_IsPlaying()
{
	CP_TRACE0("CP_OutPI_IsPlaying");
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	
	if (glb_OutputData.m_pCBuffer)
		glb_OutputData.m_pCBuffer->SetComplete(glb_OutputData.m_pCBuffer);
		
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
	
	return 0;
}

//
//
////////////////////////////////////////////////////////////////////////////////


#define CP_INVALIDCHARPOS -1
typedef In_Module*(*wp_winampGetInModule2)();
////////////////////////////////////////////////////////////////////////////////
//
//
//
void ProbeWinAmpModule(CPs_CoDecModule* pCoDec, const char* pcModulePath)
{
	CPs_CoDec_WinAmpPlugin *pContext;
	HMODULE hModPlugin;
	wp_winampGetInModule2 pfnGetInModule;
	In_Module* pInModule;
	pContext = (CPs_CoDec_WinAmpPlugin*)pCoDec->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	// Load plugin and get EP
	hModPlugin = LoadLibrary(pcModulePath);
	
	if (!hModPlugin)
		return;
		
	pfnGetInModule = (wp_winampGetInModule2)GetProcAddress(hModPlugin, "winampGetInModule2");
	
	if (!pfnGetInModule)
	{
		FreeLibrary(hModPlugin);
		return;
	}
	
	// Get the plug in descriptor
	pInModule = pfnGetInModule();
	
	// Skip modules that we cannot support
	if (!pInModule->UsesOutputPlug)
	{
		CP_TRACE1("\"%s\" Cannot be supported (not an output user)", pcModulePath);
		FreeLibrary(hModPlugin);
		return;
	}
	
	// Get the extensions supported by this plugin
	
	if (pInModule && pInModule->FileExtensions && pInModule->FileExtensions[0])
	{
		char* pExtensions = pInModule->FileExtensions;
		CP_PlugInModule* pNewPlugInModule = (CP_PlugInModule*)malloc(sizeof(CP_PlugInModule));
		
		// Create new plug in module and add it to our list
		pNewPlugInModule->m_pNext = pContext->m_pFirstPlugIn;
		STR_AllocSetString(&pNewPlugInModule->m_pcModuleName, pcModulePath, FALSE);
		pContext->m_pFirstPlugIn = pNewPlugInModule;
		
		// Attach etensions to this module
		CP_TRACE1("Probing module: \"%s\"", pcModulePath);
		
		while (*pExtensions)
		{
			char* pDescription = pExtensions + strlen(pExtensions) + 1;
			CP_TRACE2("Extensions: \"%s\" type:\"%s\"", pExtensions, pDescription);
			
			// The extensions are in the format EXT1;EXT2;EXT3  - split these and
			// add them to the player's extension list
			{
				char* pcExtensionCursor;
				char* pcLastExtensionStart = pExtensions;
				
				for (pcExtensionCursor = pExtensions; *pcExtensionCursor; pcExtensionCursor++)
				{
					// Look for an extension break
					if (*pcExtensionCursor == ';' && pcExtensionCursor != pcLastExtensionStart)
					{
						const int iExtensionLen = pcExtensionCursor - pcLastExtensionStart;
						char* pcExtensionCopy = (char*)malloc(iExtensionLen + 1);
						
						memcpy(pcExtensionCopy, pcLastExtensionStart, iExtensionLen);
						pcExtensionCopy[iExtensionLen] = '\0';
						
						CPFA_AddFileAssociation(pCoDec, pcExtensionCopy, (DWORD)pNewPlugInModule);
						
						free(pcExtensionCopy);
						pcLastExtensionStart = pcExtensionCursor + 1;
					}
				}
				
				// Add last extension
				
				if (*pcLastExtensionStart)
					CPFA_AddFileAssociation(pCoDec, pcLastExtensionStart, (DWORD)pNewPlugInModule);
			}
			
			// Move cursor to next extension set
			pExtensions = pDescription + strlen(pDescription) + 1;
		}
	}
	
	// Free plugin
	FreeLibrary(hModPlugin);
}

//
//
//
void AddWinAmpModulesInPath(CPs_CoDecModule* pCoDec, const char* pcFileInPath)
{
	WIN32_FIND_DATA finddata;
	HANDLE hFileFind;
	char pcModuleDirectory[MAX_PATH];
	char pcSearchWildcard[MAX_PATH];
	
	strcpy(pcModuleDirectory, pcFileInPath);
	
	// Work out the module directory
	{
		int iLastSlashPos, iCharIDX;
		
		iLastSlashPos = CP_INVALIDCHARPOS;
		
		for (iCharIDX = 0; pcModuleDirectory[iCharIDX]; iCharIDX++)
		{
			if (pcModuleDirectory[iCharIDX] == '\\')
				iLastSlashPos = iCharIDX;
		}
		
		// We must exist in some kind of directory!!!
		CP_ASSERT(iLastSlashPos != CP_INVALIDCHARPOS);
		
		pcModuleDirectory[iLastSlashPos+1] = '\0';
	}
	
	// Setup wildcard
	strcpy(pcSearchWildcard, pcModuleDirectory);
	strcat(pcSearchWildcard, "in_*.dll");
	
	// Load each module to find out the file extensions that it supports
	// ProbeWinAmpModule will also produce a linked list of module names
	// and assign them to the FileAssociation cookie
	hFileFind = FindFirstFile(pcSearchWildcard, &finddata);
	
	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			char pcFullPath[MAX_PATH];
			strcpy(pcFullPath, pcModuleDirectory);
			strcat(pcFullPath, finddata.cFileName);
			
			ProbeWinAmpModule(pCoDec, pcFullPath);
			
		}
		
		while (FindNextFile(hFileFind, &finddata) != 0);
		
		FindClose(hFileFind);
	}
}

//
//
//
void CP_InitialiseCodec_WinAmpPlugin(CPs_CoDecModule* pCoDec)
{
	CPs_CoDec_WinAmpPlugin *pContext;
	
	// Setup functions
	pCoDec->Uninitialise = CPP_OMAPLG_Uninitialise;
	pCoDec->OpenFile = CPP_OMAPLG_OpenFile;
	pCoDec->CloseFile = CPP_OMAPLG_CloseFile;
	pCoDec->Seek = CPP_OMAPLG_Seek;
	pCoDec->GetFileInfo = CPP_OMAPLG_GetFileInfo;
	
	pCoDec->GetPCMBlock = CPP_OMAPLG_GetPCMBlock;
	pCoDec->GetCurrentPos_secs = CPP_OMAPLG_GetCurrentPos_secs;
	
	// Setup private data
	pCoDec->m_pModuleCookie = malloc(sizeof(CPs_CoDec_WinAmpPlugin));
	pContext = (CPs_CoDec_WinAmpPlugin*)pCoDec->m_pModuleCookie;
	
	pContext->m_pFirstPlugIn = 0L;
	pContext->m_pActivePluginModule = NULL;
	pContext->m_hModPlugin = NULL;
	pContext->m_pInModule = NULL;
	pContext->m_bModuleIsPlaying = FALSE;
	memset(&pContext->m_FakeOutModule, 0, sizeof(pContext->m_FakeOutModule));
	pContext->m_FakeOutModule.version = OUT_VER;
	pContext->m_FakeOutModule.Open = CP_OutPI_Open;
	pContext->m_FakeOutModule.Close = CP_OutPI_Close;
	pContext->m_FakeOutModule.Write = CP_OutPI_Write;
	pContext->m_FakeOutModule.CanWrite = CP_OutPI_CanWrite;
	pContext->m_FakeOutModule.IsPlaying = CP_OutPI_IsPlaying;
	pContext->m_FakeOutModule.Flush = CP_OutPI_Flush;
	
	pContext->m_FakeOutModule.Pause = CP_Dummy_Pause;
	pContext->m_FakeOutModule.SetVolume = CP_Dummy_SetVolume;
	pContext->m_FakeOutModule.SetPan = CP_Dummy_SetPan;
	pContext->m_FakeOutModule.GetOutputTime = CP_Dummy_GetOutputTime;
	pContext->m_FakeOutModule.GetWrittenTime = CP_Dummy_GetWrittenTime;
	
	// Global data
	InitializeCriticalSection(&glb_OutputData.m_csGlobal);
	glb_OutputData.m_evtFileInfoValid = NULL;
	memset(&glb_OutputData.m_FileInfo, 0, sizeof(glb_OutputData.m_FileInfo));
	glb_OutputData.m_pCBuffer = NULL;
	glb_OutputData.m_iCurrentTime_ms = 0;
	glb_OutputData.m_evtSeekComplete = NULL;
	CPFA_InitialiseFileAssociations(pCoDec);
	
	// Look in the current directory for WinAmp plugins - these have the format
	// in_*.dll
	{
		char pcCurrentModuleDirectory[MAX_PATH];
		main_get_program_path(GetModuleHandle(NULL), pcCurrentModuleDirectory, MAX_PATH);
		AddWinAmpModulesInPath(pCoDec, pcCurrentModuleDirectory);
	}
	
	
	/*
	
	  Some plugins have problems running outside of the WinAmp directory - so we will only
	  use plugins from the current dir (until someone has inspiration!!)
	
	*/
	/*
	 // Try to track down the WinAmp plugins directory (and add them too)
	 {
	  LONG lResult;
	  HKEY hKeyWinAmp;
	
	  lResult = RegOpenKey(HKEY_CLASSES_ROOT,
	        "WinAmp.File\\shell\\open\\command",
	        &hKeyWinAmp);
	  if(lResult == ERROR_SUCCESS)
	  {
	   char cWinAmpPath[255];
	   char* pcValue;
	   LONG iValueLen;
	
	   RegQueryValue(hKeyWinAmp, NULL, NULL, &iValueLen);
	   pcValue = (char*)malloc(iValueLen);
	   RegQueryValue(hKeyWinAmp, NULL, pcValue, &iValueLen);
	
	   if(sscanf(pcValue, "\"%255[^\"]", cWinAmpPath) == 1)
	   {
	    int iLastSlashPos, iCharIDX;
	
	    // Remove the WinAmp exe name
	    iLastSlashPos = CP_INVALIDCHARPOS;
	    for(iCharIDX=0; cWinAmpPath[iCharIDX]; iCharIDX++)
	    {
	     if(cWinAmpPath[iCharIDX] == '\\')
	      iLastSlashPos = iCharIDX;
	    }
	    // We must exist in some kind of directory!!!
	    CP_ASSERT(iLastSlashPos != CP_INVALIDCHARPOS);
	    cWinAmpPath[iLastSlashPos+1] = '\0';
	
	    // Append the Plugins folder
	    strcat(cWinAmpPath, "Plugins\\");
	    AddWinAmpModulesInPath(pCoDec, cWinAmpPath);
	   }
	
	   // Cleanup
	   free(pcValue);
	   RegCloseKey(hKeyWinAmp);
	  }
	 }
	*/
}

//
//
//
void CPP_OMAPLG_Uninitialise(CPs_CoDecModule* pModule)
{
	CPs_CoDec_WinAmpPlugin *pContext = (CPs_CoDec_WinAmpPlugin*)pModule->m_pModuleCookie;
	CP_PlugInModule* pPlugInCursor;
	CP_PlugInModule* pPlugInCursor_Next;
	CP_CHECKOBJECT(pContext);
	
	// Cleanup plug in module list
	pPlugInCursor_Next = NULL;
	
	for (pPlugInCursor = pContext->m_pFirstPlugIn; pPlugInCursor; pPlugInCursor = pPlugInCursor_Next)
	{
		pPlugInCursor_Next = (CP_PlugInModule*)pPlugInCursor->m_pNext;
		
		free(pPlugInCursor->m_pcModuleName);
		free(pPlugInCursor);
	}
	
	// Global output data
	DeleteCriticalSection(&glb_OutputData.m_csGlobal);
	
	free(pContext);
	
	CPFA_EmptyFileAssociations(pModule);
}

//
//
//
void InitialiseGlobalData()
{
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	
	// Stuff for the file information
	CP_ASSERT(!glb_OutputData.m_evtFileInfoValid);
	glb_OutputData.m_evtFileInfoValid = CreateEvent(NULL, TRUE, FALSE, NULL);
	memset(&glb_OutputData.m_FileInfo, 0, sizeof(glb_OutputData.m_FileInfo));
	
	// Stuff for the data buffer
	CP_ASSERT(!glb_OutputData.m_pCBuffer);
	glb_OutputData.m_pCBuffer = CP_CreateCircleBuffer(CIC_CIRCLE_DATABUFFER_SIZE);
	glb_OutputData.m_iCurrentTime_ms = 0;
	
	CP_ASSERT(!glb_OutputData.m_evtSeekComplete);
	glb_OutputData.m_evtSeekComplete = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
}

//
//
//
void UnitialiseGlobalData()
{
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	
	// Stuff for the file information
	CP_ASSERT(glb_OutputData.m_evtFileInfoValid);
	CloseHandle(glb_OutputData.m_evtFileInfoValid);
	glb_OutputData.m_evtFileInfoValid = NULL;
	
	// Stuff for the data buffer
	CP_ASSERT(glb_OutputData.m_pCBuffer);
	glb_OutputData.m_pCBuffer->Uninitialise(glb_OutputData.m_pCBuffer);
	glb_OutputData.m_pCBuffer = NULL;
	
	CP_ASSERT(glb_OutputData.m_evtSeekComplete);
	CloseHandle(glb_OutputData.m_evtSeekComplete);
	glb_OutputData.m_evtSeekComplete = NULL;
	
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
}

//
//
//
void CloseCurrentModule(CPs_CoDec_WinAmpPlugin *pContext)
{
	CP_CHECKOBJECT(pContext);
	CP_ASSERT(pContext->m_pActivePluginModule);
	CP_ASSERT(pContext->m_hModPlugin);
	CP_ASSERT(pContext->m_pInModule);
	CP_ASSERT(pContext->m_pInModule->Stop);
	CP_ASSERT(pContext->m_pInModule->Quit);
	
	CP_TRACE1("Free module: \"%s\"", pContext->m_pActivePluginModule->m_pcModuleName);
	
	if (pContext->m_bModuleIsPlaying)
		pContext->m_pInModule->Stop();
		
	pContext->m_pInModule->Quit();
	
	FreeLibrary(pContext->m_hModPlugin);
	
	pContext->m_pActivePluginModule = NULL;
	pContext->m_hModPlugin = NULL;
	pContext->m_pInModule = NULL;
	pContext->m_bModuleIsPlaying = FALSE;
}

//
//
//
void SetupCurrentInputPluginModule(CPs_CoDec_WinAmpPlugin *pContext)
{
	CP_CHECKOBJECT(pContext);
	CP_ASSERT(pContext->m_pActivePluginModule);
	CP_ASSERT(pContext->m_hModPlugin);
	CP_ASSERT(pContext->m_pInModule);
	
	pContext->m_pInModule->hMainWindow = (HWND)0x2398;
	pContext->m_pInModule->hDllInstance = pContext->m_hModPlugin;
	
	// Setup functions
	pContext->m_pInModule->SAVSAInit = CP_Dummy_SAVSAInit;
	pContext->m_pInModule->SAVSADeInit = CP_Dummy_SAVSADeInit;
	pContext->m_pInModule->SAAddPCMData = CP_Dummy_SAAddPCMData;
	pContext->m_pInModule->SAGetMode = CP_Dummy_SAGetMode;
	pContext->m_pInModule->SAAdd = CP_Dummy_SAAdd;
	
	pContext->m_pInModule->VSAAddPCMData = CP_Dummy_VSAAddPCMData;
	pContext->m_pInModule->VSAGetMode = CP_Dummy_VSAGetMode;
	pContext->m_pInModule->VSAAdd = CP_Dummy_VSAAdd;
	
	pContext->m_pInModule->dsp_isactive = CP_Dummy_dsp_isactive;
	pContext->m_pInModule->dsp_dosamples = CP_Dummy_dsp_dosamples;
	
	pContext->m_pInModule->VSASetInfo = CP_Dummy_VSASetInfo;
	pContext->m_pInModule->SetInfo = CP_Dummy_SetInfo;
	
	pContext->m_pInModule->outMod = &pContext->m_FakeOutModule;
}

//
//
//
BOOL CPP_OMAPLG_OpenFile(CPs_CoDecModule* pModule, const char* pcFilename, DWORD dwCookie, HWND hWndOwner)
{
	CPs_CoDec_WinAmpPlugin *pContext = (CPs_CoDec_WinAmpPlugin*)pModule->m_pModuleCookie;
	CP_PlugInModule* pSelectedPlugInModule = (CP_PlugInModule*)dwCookie;
	int iError;
	CP_CHECKOBJECT(pContext);
	
	if (!pSelectedPlugInModule) return FALSE;
	
	// Open output module
	{
		// Module has changed
		wp_winampGetInModule2 pfnGetInModule;
		
		if (pContext->m_pActivePluginModule)
			CloseCurrentModule(pContext);
			
		// Load plugin and get EP
		CP_TRACE1("Load module: \"%s\"", pSelectedPlugInModule->m_pcModuleName);
		
		pContext->m_hModPlugin = LoadLibrary(pSelectedPlugInModule->m_pcModuleName);
		
		if (!pContext->m_hModPlugin)
			return FALSE;
			
		pfnGetInModule = (wp_winampGetInModule2)GetProcAddress(pContext->m_hModPlugin, "winampGetInModule2");
		
		if (!pfnGetInModule)
		{
			FreeLibrary(pContext->m_hModPlugin);
			pContext->m_hModPlugin = NULL;
			return FALSE;
		}
		
		pContext->m_pActivePluginModule = pSelectedPlugInModule;
		
		pContext->m_pInModule = pfnGetInModule();
		SetupCurrentInputPluginModule(pContext);
		pContext->m_pInModule->Init();
		InitialiseGlobalData();
	}
	
	iError = pContext->m_pInModule->Play(pcFilename);
	
	if (iError != 0)
	{
		CP_TRACE1("**** Failed to open file: \"%s\"", pcFilename);
		CloseCurrentModule(pContext);
		UnitialiseGlobalData();
		return FALSE;
	}
	
	pContext->m_bModuleIsPlaying = TRUE;
	
	return TRUE; // Success
}

//
//
//
void CPP_OMAPLG_CloseFile(CPs_CoDecModule* pModule)
{
	CPs_CoDec_WinAmpPlugin *pContext = (CPs_CoDec_WinAmpPlugin*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	
	UnitialiseGlobalData();
	
	if (pContext->m_pActivePluginModule)
		CloseCurrentModule(pContext);
}

//
//
//
void CPP_OMAPLG_Seek(CPs_CoDecModule* pModule, const int iNumerator, const int iDenominator)
{
	CPs_CoDec_WinAmpPlugin *pContext = (CPs_CoDec_WinAmpPlugin*)pModule->m_pModuleCookie;
	int iStreamLength_ms;
	CP_CHECKOBJECT(pContext);
	CP_ASSERT(pContext->m_pActivePluginModule);
	
	iStreamLength_ms = pContext->m_pInModule->GetLength();
	pContext->m_pInModule->SetOutputTime((iNumerator*iStreamLength_ms) / iDenominator);
	WaitForSingleObject(glb_OutputData.m_evtSeekComplete, CIC_WAITTIMEOUT);
}

//
//
//
BOOL CPP_OMAPLG_GetPCMBlock(CPs_CoDecModule* pModule, void* _pBlock, DWORD* pdwBlockSize)
{
	unsigned int bytes;
	BOOL reply;
	
#ifdef _DEBUG
	CPs_CoDec_WinAmpPlugin *pContext = (CPs_CoDec_WinAmpPlugin*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
#endif
	
	reply = glb_OutputData.m_pCBuffer->Read(glb_OutputData.m_pCBuffer, _pBlock, *pdwBlockSize, &bytes);
	*pdwBlockSize = (DWORD) bytes;
	return reply;
}

//
//
//
void CPP_OMAPLG_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo* pInfo)
{
	DWORD dwWaitResult;
	CPs_CoDec_WinAmpPlugin *pContext = (CPs_CoDec_WinAmpPlugin*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_ASSERT(pContext->m_pActivePluginModule);
	
	// Wait up to 1 second for the plugin to return the stream format - if it doesn't
	// set the file info into something that we know will be unsupported
	dwWaitResult = WaitForSingleObject(glb_OutputData.m_evtFileInfoValid, CIC_WAITTIMEOUT);
	
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		EnterCriticalSection(&glb_OutputData.m_csGlobal);
		memcpy(pInfo, &glb_OutputData.m_FileInfo, sizeof(*pInfo));
		LeaveCriticalSection(&glb_OutputData.m_csGlobal);
	}
	
	else
	{
		CP_TRACE0("Module did not respond in time - setting bogus stream params");
		memset(pInfo, 0, sizeof(*pInfo));
	}
	
	pInfo->m_iFileLength_Secs = pContext->m_pInModule->GetLength() / 1000;
}

//
//
//
int CPP_OMAPLG_GetCurrentPos_secs(CPs_CoDecModule* pModule)
{
	int iCurrentPos_secs;
#ifdef _DEBUG
	CPs_CoDec_WinAmpPlugin *pContext = (CPs_CoDec_WinAmpPlugin*)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(pContext);
	CP_ASSERT(pContext->m_pActivePluginModule);
#endif
	
	EnterCriticalSection(&glb_OutputData.m_csGlobal);
	iCurrentPos_secs = glb_OutputData.m_iCurrentTime_ms / 1000;
	LeaveCriticalSection(&glb_OutputData.m_csGlobal);
	
	return iCurrentPos_secs;
}

//
//
//
