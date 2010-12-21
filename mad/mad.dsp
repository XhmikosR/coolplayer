# Microsoft Developer Studio Project File - Name="mad" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mad - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mad.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mad.mak" CFG="mad - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mad - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "mad - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mad - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "data" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "FPM_DEFAULT" /D "ASO_ZEROCHECK" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mad - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "data" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "FPM_DEFAULT" /D "ASO_ZEROCHECK" /D "HAVE_CONFIG_H" /YX /FD /GZ  /c
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

# Name "mad - Win32 Release"
# Name "mad - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bit.c
# End Source File
# Begin Source File

SOURCE=.\decoder.c
# End Source File
# Begin Source File

SOURCE=.\fixed.c
# End Source File
# Begin Source File

SOURCE=.\frame.c
# End Source File
# Begin Source File

SOURCE=.\huffman.c
# End Source File
# Begin Source File

SOURCE=.\layer12.c
# End Source File
# Begin Source File

SOURCE=.\layer3.c
# End Source File
# Begin Source File

SOURCE=.\stream.c
# End Source File
# Begin Source File

SOURCE=.\synth.c
# End Source File
# Begin Source File

SOURCE=.\timer.c
# End Source File
# Begin Source File

SOURCE=.\version.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bit.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\config.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\decoder.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\fixed.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\frame.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\global.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\huffman.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\layer12.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\layer3.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\mad.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\stream.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\synth.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\timer.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\version.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Data Files"

# PROP Default_Filter "dat"
# Begin Source File

SOURCE=.\data\D.dat
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\data\imdct_s.dat
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\data\qc_table.dat
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\data\rq_table.dat
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\data\sf_table.dat
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
