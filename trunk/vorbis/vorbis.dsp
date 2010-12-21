# Microsoft Developer Studio Project File - Name="vorbis" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=vorbis - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vorbis.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vorbis.mak" CFG="vorbis - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vorbis - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "vorbis - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vorbis - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../ogg" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "vorbis - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../ogg" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "vorbis - Win32 Release"
# Name "vorbis - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\analysis.c
# End Source File
# Begin Source File

SOURCE=.\bitrate.c
# End Source File
# Begin Source File

SOURCE=.\block.c
# End Source File
# Begin Source File

SOURCE=.\codebook.c
# End Source File
# Begin Source File

SOURCE=.\envelope.c
# End Source File
# Begin Source File

SOURCE=.\floor0.c
# End Source File
# Begin Source File

SOURCE=.\floor1.c
# End Source File
# Begin Source File

SOURCE=.\info.c
# End Source File
# Begin Source File

SOURCE=.\lookup.c
# End Source File
# Begin Source File

SOURCE=.\lpc.c
# End Source File
# Begin Source File

SOURCE=.\lsp.c
# End Source File
# Begin Source File

SOURCE=.\mapping0.c
# End Source File
# Begin Source File

SOURCE=.\mdct.c
# End Source File
# Begin Source File

SOURCE=.\psy.c
# End Source File
# Begin Source File

SOURCE=.\registry.c
# End Source File
# Begin Source File

SOURCE=.\res0.c
# End Source File
# Begin Source File

SOURCE=.\sharedbook.c
# End Source File
# Begin Source File

SOURCE=.\smallft.c
# End Source File
# Begin Source File

SOURCE=.\synthesis.c
# End Source File
# Begin Source File

SOURCE=.\vorbisfile.c
# End Source File
# Begin Source File

SOURCE=.\window.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "vorbis"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\vorbis\codec.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\vorbis\vorbisfile.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\backends.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\bitrate.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\codebook.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\codec_internal.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\envelope.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\highlevel.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\lookup.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\lookup_data.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\lpc.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\lsp.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\masking.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\mdct.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\misc.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\os.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\psy.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\registry.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\scales.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\smallft.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\window.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
