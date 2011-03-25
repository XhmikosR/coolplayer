# *
# * makefile.mak, makefile for building CoolPlayer with WDK
# *
# * Copyright (C) 2010-2011 XhmikosR
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


!IFDEF x64
BINDIR       = ..\bin\WDK\Release_x64
!ELSE
BINDIR       = ..\bin\WDK\Release_x86
!ENDIF
OBJDIR       = $(BINDIR)\obj
EXE          = $(BINDIR)\coolplayer.exe


CP_DIR       = ..\coolplayer
MAD_DIR      = ..\mad
OGG_DIR      = ..\ogg
VORBIS_DIR   = ..\vorbis
ZLIB_DIR     = ..\zlib


DEFINES      = /D "_WINDOWS" /D "NDEBUG" /D "_CRT_SECURE_NO_WARNINGS"
CFLAGS       = /nologo /c /Fo"$(OBJDIR)/" /EHsc /MD /O1 /GL /MP
LDFLAGS      = /NOLOGO /WX /INCREMENTAL:NO /RELEASE /OPT:REF /OPT:ICF /LTCG /MERGE:.rdata=.text \
               /DYNAMICBASE /NXCOMPAT /DEBUG
LIBS         = kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib comdlg32.lib \
               comctl32.lib winspool.lib ole32.lib oleaut32.lib dsound.lib wininet.lib \
               winmm.lib
MTFLAGS      = -nologo


!IFDEF x64
CFLAGS       = $(CFLAGS) /D "_WIN64" /D "_WIN32_WINNT=0x0502"
LDFLAGS      = $(LDFLAGS) /SUBSYSTEM:WINDOWS,5.02 /MACHINE:X64
LIBS         = $(LIBS) msvcrt_win2003.obj
RFLAGS       = /d "_WIN64"
!ELSE
CFLAGS       = $(CFLAGS) /D "WIN32" /D "_WIN32_WINNT=0x0500"
LDFLAGS      = $(LDFLAGS) /SUBSYSTEM:WINDOWS,5.0 /MACHINE:X86
LIBS         = $(LIBS) msvcrt_win2000.obj
RFLAGS       = /d "WIN32"
!ENDIF


CPCFLAGS     = cl $(CFLAGS) /W3 /WX $(DEFINES) /I "$(MAD_DIR)" /I "$(OGG_DIR)" \
               /I "$(VORBIS_DIR)" /I ".." /Tc $<
MADCFLAGS    = cl $(CFLAGS) /W0 $(DEFINES) /D "FPM_DEFAULT" /D "ASO_ZEROCHECK" \
               /D "HAVE_CONFIG_H" /I "$(MAD_DIR)\data" /Tc $<
OGGCFLAGS    = cl $(CFLAGS) /W0 $(DEFINES) /I "$(OGG_DIR)" /Tc $<
VORBISCFLAGS = cl $(CFLAGS) /W0 $(DEFINES) /I "$(OGG_DIR)" /Tc $<
ZLIBCFLAGS   = cl $(CFLAGS) /W0 $(DEFINES) /Tc $<



###############
##  Targets  ##
###############
BUILD:	CHECKDIRS $(EXE)

CHECKDIRS:
	-MKDIR "$(OBJDIR)" >NUL 2>&1

CLEAN:
	ECHO Cleaning... & ECHO.
	-DEL "$(EXE)" "$(OBJDIR)\*.obj" "$(OBJDIR)\coolplayer.res" \
	"$(BINDIR)\coolplayer.pdb" >NUL 2>&1
	-RMDIR /Q "$(OBJDIR)" "$(BINDIR)" >NUL 2>&1

REBUILD:	CLEAN BUILD


####################
##  Object files  ##
####################
CP_OBJ = \
    $(OBJDIR)\about.obj \
    $(OBJDIR)\bitmap2region.obj \
    $(OBJDIR)\CLV_ListView.obj \
    $(OBJDIR)\CompositeFile.obj \
    $(OBJDIR)\coolplayer.res \
    $(OBJDIR)\CPI_CircleBuffer.obj \
    $(OBJDIR)\CPI_Equaliser_Basic.obj \
    $(OBJDIR)\CPI_ID3_Genres.obj \
    $(OBJDIR)\CPI_Image.obj \
    $(OBJDIR)\CPI_Indicators.obj \
    $(OBJDIR)\CPI_Interface.obj \
    $(OBJDIR)\CPI_InterfacePart.obj \
    $(OBJDIR)\CPI_InterfacePart_CommandButton.obj \
    $(OBJDIR)\CPI_InterfacePart_Indicator.obj \
    $(OBJDIR)\CPI_Keyboard.obj \
    $(OBJDIR)\CPI_Player.obj \
    $(OBJDIR)\CPI_Player_Callbacks.obj \
    $(OBJDIR)\CPI_Player_CoDec_MPEG.obj \
    $(OBJDIR)\CPI_Player_CoDec_OGG.obj \
    $(OBJDIR)\CPI_Player_CoDec_WAV.obj \
    $(OBJDIR)\CPI_Player_CoDec_WinAmpPlugin.obj \
    $(OBJDIR)\CPI_Player_Engine.obj \
    $(OBJDIR)\CPI_Player_FileAssoc.obj \
    $(OBJDIR)\CPI_Player_Output_DirectSound.obj \
    $(OBJDIR)\CPI_Player_Output_File.obj \
    $(OBJDIR)\CPI_Player_Output_Wave.obj \
    $(OBJDIR)\CPI_Playlist.obj \
    $(OBJDIR)\CPI_PlaylistItem.obj \
    $(OBJDIR)\CPI_PlaylistWindow.obj \
    $(OBJDIR)\CPI_Playlist_Callbacks.obj \
    $(OBJDIR)\CPI_Stream.obj \
    $(OBJDIR)\CPI_Stream_Internet.obj \
    $(OBJDIR)\CPI_Stream_LocalFile.obj \
    $(OBJDIR)\CPI_Verbs.obj \
    $(OBJDIR)\CPSK_Skin.obj \
    $(OBJDIR)\DLG_Find.obj \
    $(OBJDIR)\main.obj \
    $(OBJDIR)\options.obj \
    $(OBJDIR)\profile.obj \
    $(OBJDIR)\RotatingIcon.obj \
    $(OBJDIR)\shwapi.obj \
    $(OBJDIR)\skin.obj \
    $(OBJDIR)\stdafx.obj \
    $(OBJDIR)\String.obj \
    $(OBJDIR)\WindowsOS.obj

MAD_OBJ = \
    $(OBJDIR)\bit.obj \
    $(OBJDIR)\decoder.obj \
    $(OBJDIR)\fixed.obj \
    $(OBJDIR)\frame.obj \
    $(OBJDIR)\huffman.obj \
    $(OBJDIR)\layer12.obj \
    $(OBJDIR)\layer3.obj \
    $(OBJDIR)\stream.obj \
    $(OBJDIR)\synth.obj \
    $(OBJDIR)\timer.obj \
    $(OBJDIR)\version.obj

OGG_OBJ = \
    $(OBJDIR)\bitwise.obj \
    $(OBJDIR)\framing.obj

VORBIS_OBJ = \
    $(OBJDIR)\analysis.obj \
    $(OBJDIR)\bitrate.obj \
    $(OBJDIR)\block.obj \
    $(OBJDIR)\codebook.obj \
    $(OBJDIR)\envelope.obj \
    $(OBJDIR)\floor0.obj \
    $(OBJDIR)\floor1.obj \
    $(OBJDIR)\info.obj \
    $(OBJDIR)\lookup.obj \
    $(OBJDIR)\lpc.obj \
    $(OBJDIR)\lsp.obj \
    $(OBJDIR)\mapping0.obj \
    $(OBJDIR)\mdct.obj \
    $(OBJDIR)\psy.obj \
    $(OBJDIR)\registry.obj \
    $(OBJDIR)\res0.obj \
    $(OBJDIR)\sharedbook.obj \
    $(OBJDIR)\smallft.obj \
    $(OBJDIR)\synthesis.obj \
    $(OBJDIR)\vorbisfile.obj \
    $(OBJDIR)\window.obj

ZLIB_OBJ = \
    $(OBJDIR)\adler32.obj \
    $(OBJDIR)\compress.obj \
    $(OBJDIR)\crc32.obj \
    $(OBJDIR)\deflate.obj \
    $(OBJDIR)\gzclose.obj \
    $(OBJDIR)\gzlib.obj \
    $(OBJDIR)\gzread.obj \
    $(OBJDIR)\gzwrite.obj \
    $(OBJDIR)\infback.obj \
    $(OBJDIR)\inffast.obj \
    $(OBJDIR)\inflate.obj \
    $(OBJDIR)\inftrees.obj \
    $(OBJDIR)\trees.obj \
    $(OBJDIR)\uncompr.obj \
    $(OBJDIR)\zutil.obj

OBJECTS = $(CP_OBJ) $(MAD_OBJ) $(OGG_OBJ) $(VORBIS_OBJ) $(ZLIB_OBJ)


###################
##  Batch rules  ##
###################
{$(CP_DIR)}.c{$(OBJDIR)}.obj::
    $(CPCFLAGS)

{$(MAD_DIR)}.c{$(OBJDIR)}.obj::
    $(MADCFLAGS)

{$(OGG_DIR)}.c{$(OBJDIR)}.obj::
    $(OGGCFLAGS)

{$(VORBIS_DIR)}.c{$(OBJDIR)}.obj::
    $(VORBISCFLAGS)

{$(ZLIB_DIR)}.c{$(OBJDIR)}.obj::
    $(ZLIBCFLAGS)


################
##  Commands  ##
################
$(EXE): $(OBJECTS)
	rc $(RFLAGS) /fo"$(OBJDIR)\coolplayer.res" "$(CP_DIR)\coolplayer.rc" >NUL
	link $(LDFLAGS) $(LIBS) $(OBJECTS) /OUT:"$(EXE)"


####################
##  Dependencies  ##
####################
!INCLUDE "makefile.deps.mak"
