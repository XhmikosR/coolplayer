# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
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

# Name "zlib - Win32 Release"
# Name "zlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\adler32.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\compress.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\crc32.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\deflate.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\gzio.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\infback.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\inffast.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\inflate.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\inftrees.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\minigzip.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\trees.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\uncompr.c
# ADD CPP /YX"zutil.h"
# End Source File
# Begin Source File

SOURCE=.\zutil.c

!IF  "$(CFG)" == "zlib - Win32 Release"

# ADD CPP /Yc"zutil.h"

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# ADD CPP /Yc"zutil.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\crc32.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\deflate.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\inffast.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\inffixed.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\inflate.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\inftrees.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\trees.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\zconf.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\zconf.in.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\zlib.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\zutil.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
