# *
# * makefile.mak, makefile for building CoolPlayer with WDK
# *
# * Copyright (C) 2010-2012 XhmikosR
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# *


# Remove the .SILENT directive in order to display all the commands
.SILENT:


CC = cl.exe
LD = link.exe
RC = rc.exe


!IFDEF x64
BINDIR       = ..\bin\WDK\Release_x64
!ELSE
BINDIR       = ..\bin\WDK\Release_x86
!ENDIF
OBJDIR       = $(BINDIR)\obj
EXE          = $(BINDIR)\coolplayer.exe


CP_OBJDIR     = $(OBJDIR)\coolplayer
MAD_OBJDIR    = $(OBJDIR)\mad
OGG_OBJDIR    = $(OBJDIR)\ogg
VORBIS_OBJDIR = $(OBJDIR)\vorbis
ZLIB_OBJDIR   = $(OBJDIR)\zlib


CP_DIR       = ..\coolplayer
MAD_DIR      = ..\mad
OGG_DIR      = ..\ogg
VORBIS_DIR   = ..\vorbis
ZLIB_DIR     = ..\zlib


DEFINES      = /D "_WINDOWS" /D "NDEBUG" /D "_CRT_SECURE_NO_WARNINGS"
CFLAGS       = /nologo /c /EHsc /MD /O2 /GL /MP $(DEFINES)
LDFLAGS      = /NOLOGO /WX /INCREMENTAL:NO /RELEASE /OPT:REF /OPT:ICF /MERGE:.rdata=.text \
               /LTCG /DYNAMICBASE /NXCOMPAT /DEBUG
LIBS         = kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib comdlg32.lib \
               comctl32.lib winspool.lib ole32.lib oleaut32.lib dsound.lib wininet.lib \
               winmm.lib


!IFDEF x64
DEFINES      = $(DEFINES) /D "_WIN64" /D "_WIN32_WINNT=0x0502"
LDFLAGS      = $(LDFLAGS) /SUBSYSTEM:WINDOWS,5.02 /MACHINE:X64
LIBS         = $(LIBS) msvcrt_win2003.obj
RFLAGS       = /d "_WIN64"
!ELSE
CFLAGS       = $(CFLAGS) /Oy
DEFINES      = $(DEFINES) /D "WIN32" /D "_WIN32_WINNT=0x0500"
LDFLAGS      = $(LDFLAGS) /SUBSYSTEM:WINDOWS,5.0 /MACHINE:X86
LIBS         = $(LIBS) msvcrt_win2000.obj
RFLAGS       = /d "WIN32"
!ENDIF


CPCFLAGS     = $(CFLAGS) /W3 /WX $(DEFINES) /I "$(MAD_DIR)" /I "$(OGG_DIR)" \
               /I "$(VORBIS_DIR)" /I ".."
MADCFLAGS    = $(CFLAGS) /W0 $(DEFINES) /D "FPM_DEFAULT" /D "ASO_ZEROCHECK" \
               /D "HAVE_CONFIG_H" /I "$(MAD_DIR)\data"
OGGCFLAGS    = $(CFLAGS) /W0 $(DEFINES) /I "$(OGG_DIR)"
VORBISCFLAGS = $(CFLAGS) /W0 $(DEFINES) /I "$(OGG_DIR)"
ZLIBCFLAGS   = $(CFLAGS) /W0 $(DEFINES) /D "NO_vsnprintf"



###############
##  Targets  ##
###############
BUILD:	CHECKDIRS $(EXE)

CHECKDIRS:
	IF NOT EXIST "$(OBJDIR)"        MD "$(OBJDIR)"
	IF NOT EXIST "$(CP_OBJDIR)"     MD "$(CP_OBJDIR)"
	IF NOT EXIST "$(MAD_OBJDIR)"    MD "$(MAD_OBJDIR)"
	IF NOT EXIST "$(OGG_OBJDIR)"    MD "$(OGG_OBJDIR)"
	IF NOT EXIST "$(VORBIS_OBJDIR)" MD "$(VORBIS_OBJDIR)"
	IF NOT EXIST "$(ZLIB_OBJDIR)"   MD "$(ZLIB_OBJDIR)"

CLEAN:
	ECHO Cleaning... & ECHO.
	IF EXIST "$(EXE)"                       DEL "$(EXE)"
	IF EXIST "$(CP_OBJDIR)\*.obj"           DEL "$(CP_OBJDIR)\*.obj"
	IF EXIST "$(MAD_OBJDIR)\*.obj"          DEL "$(MAD_OBJDIR)\*.obj"
	IF EXIST "$(OGG_OBJDIR)\*.obj"          DEL "$(OGG_OBJDIR)\*.obj"
	IF EXIST "$(VORBIS_OBJDIR)\*.obj"       DEL "$(VORBIS_OBJDIR)\*.obj"
	IF EXIST "$(ZLIB_OBJDIR)\*.obj"         DEL "$(ZLIB_OBJDIR)\*.obj"
	IF EXIST "$(CP_OBJDIR)\coolplayer.res"  DEL "$(CP_OBJDIR)\coolplayer.res"
	IF EXIST "$(BINDIR)\coolplayer.pdb"     DEL "$(BINDIR)\coolplayer.pdb"
	-IF EXIST "$(CP_OBJDIR)"                RD /Q "$(CP_OBJDIR)"
	-IF EXIST "$(MAD_OBJDIR)"               RD /Q "$(MAD_OBJDIR)"
	-IF EXIST "$(OGG_OBJDIR)"               RD /Q "$(OGG_OBJDIR)"
	-IF EXIST "$(VORBIS_OBJDIR)"            RD /Q "$(VORBIS_OBJDIR)"
	-IF EXIST "$(ZLIB_OBJDIR)"              RD /Q "$(ZLIB_OBJDIR)"
	-IF EXIST "$(OBJDIR)"                   RD /Q "$(OBJDIR)"
	-IF EXIST "$(BINDIR)"                   RD /Q "$(BINDIR)"

REBUILD:	CLEAN BUILD


####################
##  Object files  ##
####################
CP_OBJ = \
    $(CP_OBJDIR)\about.obj \
    $(CP_OBJDIR)\bitmap2region.obj \
    $(CP_OBJDIR)\CLV_ListView.obj \
    $(CP_OBJDIR)\CompositeFile.obj \
    $(CP_OBJDIR)\coolplayer.res \
    $(CP_OBJDIR)\CPI_CircleBuffer.obj \
    $(CP_OBJDIR)\CPI_Equaliser_Basic.obj \
    $(CP_OBJDIR)\CPI_ID3_Genres.obj \
    $(CP_OBJDIR)\CPI_Image.obj \
    $(CP_OBJDIR)\CPI_Indicators.obj \
    $(CP_OBJDIR)\CPI_Interface.obj \
    $(CP_OBJDIR)\CPI_InterfacePart.obj \
    $(CP_OBJDIR)\CPI_InterfacePart_CommandButton.obj \
    $(CP_OBJDIR)\CPI_InterfacePart_Indicator.obj \
    $(CP_OBJDIR)\CPI_Keyboard.obj \
    $(CP_OBJDIR)\CPI_Player.obj \
    $(CP_OBJDIR)\CPI_Player_Callbacks.obj \
    $(CP_OBJDIR)\CPI_Player_CoDec_MPEG.obj \
    $(CP_OBJDIR)\CPI_Player_CoDec_OGG.obj \
    $(CP_OBJDIR)\CPI_Player_CoDec_WAV.obj \
    $(CP_OBJDIR)\CPI_Player_CoDec_WinAmpPlugin.obj \
    $(CP_OBJDIR)\CPI_Player_Engine.obj \
    $(CP_OBJDIR)\CPI_Player_FileAssoc.obj \
    $(CP_OBJDIR)\CPI_Player_Output_DirectSound.obj \
    $(CP_OBJDIR)\CPI_Player_Output_File.obj \
    $(CP_OBJDIR)\CPI_Player_Output_Wave.obj \
    $(CP_OBJDIR)\CPI_Playlist.obj \
    $(CP_OBJDIR)\CPI_PlaylistItem.obj \
    $(CP_OBJDIR)\CPI_PlaylistWindow.obj \
    $(CP_OBJDIR)\CPI_Playlist_Callbacks.obj \
    $(CP_OBJDIR)\CPI_Stream.obj \
    $(CP_OBJDIR)\CPI_Stream_Internet.obj \
    $(CP_OBJDIR)\CPI_Stream_LocalFile.obj \
    $(CP_OBJDIR)\CPI_Verbs.obj \
    $(CP_OBJDIR)\CPSK_Skin.obj \
    $(CP_OBJDIR)\DLG_Find.obj \
    $(CP_OBJDIR)\main.obj \
    $(CP_OBJDIR)\options.obj \
    $(CP_OBJDIR)\profile.obj \
    $(CP_OBJDIR)\RotatingIcon.obj \
    $(CP_OBJDIR)\shwapi.obj \
    $(CP_OBJDIR)\skin.obj \
    $(CP_OBJDIR)\stdafx.obj \
    $(CP_OBJDIR)\String.obj \
    $(CP_OBJDIR)\WindowsOS.obj

MAD_OBJ = \
    $(MAD_OBJDIR)\bit.obj \
    $(MAD_OBJDIR)\decoder.obj \
    $(MAD_OBJDIR)\fixed.obj \
    $(MAD_OBJDIR)\frame.obj \
    $(MAD_OBJDIR)\huffman.obj \
    $(MAD_OBJDIR)\layer12.obj \
    $(MAD_OBJDIR)\layer3.obj \
    $(MAD_OBJDIR)\stream.obj \
    $(MAD_OBJDIR)\synth.obj \
    $(MAD_OBJDIR)\timer.obj \
    $(MAD_OBJDIR)\version.obj

OGG_OBJ = \
    $(OGG_OBJDIR)\bitwise.obj \
    $(OGG_OBJDIR)\framing.obj

VORBIS_OBJ = \
    $(VORBIS_OBJDIR)\analysis.obj \
    $(VORBIS_OBJDIR)\bitrate.obj \
    $(VORBIS_OBJDIR)\block.obj \
    $(VORBIS_OBJDIR)\codebook.obj \
    $(VORBIS_OBJDIR)\envelope.obj \
    $(VORBIS_OBJDIR)\floor0.obj \
    $(VORBIS_OBJDIR)\floor1.obj \
    $(VORBIS_OBJDIR)\info.obj \
    $(VORBIS_OBJDIR)\lookup.obj \
    $(VORBIS_OBJDIR)\lpc.obj \
    $(VORBIS_OBJDIR)\lsp.obj \
    $(VORBIS_OBJDIR)\mapping0.obj \
    $(VORBIS_OBJDIR)\mdct.obj \
    $(VORBIS_OBJDIR)\psy.obj \
    $(VORBIS_OBJDIR)\registry.obj \
    $(VORBIS_OBJDIR)\res0.obj \
    $(VORBIS_OBJDIR)\sharedbook.obj \
    $(VORBIS_OBJDIR)\smallft.obj \
    $(VORBIS_OBJDIR)\synthesis.obj \
    $(VORBIS_OBJDIR)\vorbisfile.obj \
    $(VORBIS_OBJDIR)\window.obj

ZLIB_OBJ = \
    $(ZLIB_OBJDIR)\adler32.obj \
    $(ZLIB_OBJDIR)\compress.obj \
    $(ZLIB_OBJDIR)\crc32.obj \
    $(ZLIB_OBJDIR)\deflate.obj \
    $(ZLIB_OBJDIR)\gzclose.obj \
    $(ZLIB_OBJDIR)\gzlib.obj \
    $(ZLIB_OBJDIR)\gzread.obj \
    $(ZLIB_OBJDIR)\gzwrite.obj \
    $(ZLIB_OBJDIR)\infback.obj \
    $(ZLIB_OBJDIR)\inffast.obj \
    $(ZLIB_OBJDIR)\inflate.obj \
    $(ZLIB_OBJDIR)\inftrees.obj \
    $(ZLIB_OBJDIR)\trees.obj \
    $(ZLIB_OBJDIR)\uncompr.obj \
    $(ZLIB_OBJDIR)\zutil.obj

OBJECTS = $(CP_OBJ) $(MAD_OBJ) $(OGG_OBJ) $(VORBIS_OBJ) $(ZLIB_OBJ)


###################
##  Batch rules  ##
###################
{$(CP_DIR)}.c{$(CP_OBJDIR)}.obj::
    $(CC) $(CPCFLAGS) /Fo"$(CP_OBJDIR)/" /Tc $<

{$(MAD_DIR)}.c{$(MAD_OBJDIR)}.obj::
    $(CC) $(MADCFLAGS) /Fo"$(MAD_OBJDIR)/" /Tc $<

{$(OGG_DIR)}.c{$(OGG_OBJDIR)}.obj::
    $(CC) $(OGGCFLAGS) /Fo"$(OGG_OBJDIR)/" /Tc $<

{$(VORBIS_DIR)}.c{$(VORBIS_OBJDIR)}.obj::
    $(CC) $(VORBISCFLAGS) /Fo"$(VORBIS_OBJDIR)/" /Tc $<

{$(ZLIB_DIR)}.c{$(ZLIB_OBJDIR)}.obj::
    $(CC) $(ZLIBCFLAGS) /Fo"$(ZLIB_OBJDIR)/" /Tc $<


################
##  Commands  ##
################
$(EXE): $(OBJECTS)
	$(RC) $(RFLAGS) /fo"$(CP_OBJDIR)\coolplayer.res" "$(CP_DIR)\coolplayer.rc" >NUL
	$(LD) $(LDFLAGS) $(LIBS) $(OBJECTS) /OUT:"$(EXE)"


####################
##  Dependencies  ##
####################
!INCLUDE "makefile.deps.mak"
