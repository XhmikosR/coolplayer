# Microsoft Developer Studio Project File - Name="coolplayer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=coolplayer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coolplayer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coolplayer.mak" CFG="coolplayer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coolplayer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "coolplayer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coolplayer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../mad" /I "../ogg" /I "../vorbis" /I "../zlib" /I "../" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib dsound.lib wininet.lib comctl32.lib /nologo /subsystem:windows /map /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "coolplayer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../mad" /I "../ogg" /I "../vorbis" /I "../zlib" /I "../" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib dsound.lib wininet.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "coolplayer - Win32 Release"
# Name "coolplayer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\about.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\bitmap2region.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CLV_ListView.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CompositeFile.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_CircleBuffer.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Equaliser_Basic.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_ID3_Genres.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Image.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Indicators.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Interface.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_InterfacePart.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_InterfacePart_CommandButton.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_InterfacePart_Indicator.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Keyboard.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Callbacks.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_CoDec_MPEG.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_CoDec_OGG.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_CoDec_WAV.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_CoDec_WinAmpPlugin.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Engine.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_FileAssoc.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Output_DirectSound.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Output_File.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Output_Wave.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Playlist.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Playlist_Callbacks.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_PlaylistItem.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_PlaylistWindow.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Stream.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Stream_Internet.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Stream_LocalFile.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPI_Verbs.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\CPSK_Skin.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\DLG_Find.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\main.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\options.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\profile.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\RotatingIcon.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\shwapi.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\skin.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\stdafx.c
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\String.c
# ADD CPP /YX"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\WindowsOS.c
# ADD CPP /YX"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\_BldNum.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CLV_ListView.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CompositeFile.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CP_RIFFStructs.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CP_WinAmpStructs.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_CircleBuffer.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Equaliser.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_ID3.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Image.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Indicators.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Interface.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_InterfacePart.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Keyboard.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Player.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_CoDec.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Engine.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Messages.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Player_Output.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Playlist.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_PlaylistItem.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_PlaylistItem_Internal.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_PlaylistWindow.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Stream.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPI_Verbs.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\CPSK_Skin.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\debug.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\DLG_Find.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\globals.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\resource.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\RotatingIcon.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\skin.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\String.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\WindowsOS.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\about.txt
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\changes.txt
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\coolplayer.ico
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\coolplayer.rc
# End Source File
# Begin Source File

SOURCE=.\res\Default.CPSkin
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\keyboard.txt
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\main_bigfont.bmp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\main_down.bmp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\main_smallfont.bmp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\main_up.bmp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\mp3.ico
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\pls.ico
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\readme.txt
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\SysIcon.bmp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\SysIcon_Mask.bmp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\usage.txt
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\coolplayer.exe.manifest
# End Source File
# End Target
# End Project
