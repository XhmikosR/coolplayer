# *
# * makefile.mak, makefile for building CoolPlayer with WDK
# * Copyright (C) 2010 XhmikosR
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

CC=cl
RC=rc
LD=link
MT=mt


!IFDEF x64
BINDIR=..\Release_x64
!ELSE
BINDIR=..\Release_x86
!ENDIF
OBJDIR=$(BINDIR)\obj
APP=$(BINDIR)\coolplayer.exe


CPSRC=..\coolplayer
MADSRC=..\mad
OGGSRC=..\ogg
VORBISSRC=..\vorbis
ZLIBSRC=..\zlib


DEFINES=/D "_WINDOWS" /D "NDEBUG" /D "_CRT_SECURE_NO_WARNINGS"
CFLAGS=/nologo /c /Fo"$(OBJDIR)/" /EHsc /MD /O1 /GS /GL /MP
LIBS=kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib comdlg32.lib comctl32.lib \
	winspool.lib ole32.lib oleaut32.lib dsound.lib wininet.lib winmm.lib
LDFLAGS=/NOLOGO /INCREMENTAL:NO /RELEASE /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT /LTCG \
		/MERGE:.rdata=.text

!IFDEF x64
CFLAGS=$(CFLAGS) /D "_WIN64" /D "_WIN32_WINNT=0x0502" 
RFLAGS=/d "_WIN64"
LIBS=$(LIBS) msvcrt_win2003.obj
LDFLAGS=$(LDFLAGS) /SUBSYSTEM:WINDOWS,5.02 /MACHINE:X64 $(LIBS)
!ELSE
CFLAGS=$(CFLAGS) /D "WIN32" /D "_WIN32_WINNT=0x0500"
RFLAGS=/d "WIN32"
LIBS=$(LIBS) msvcrt_win2000.obj
LDFLAGS=$(LDFLAGS) /SUBSYSTEM:WINDOWS,5.0 /MACHINE:X86 $(LIBS)
!ENDIF


CPCFLAGS=@$(CC) $(CFLAGS) /W3 $(DEFINES) /I "$(MADSRC)" /I "$(OGGSRC)" \
	/I "$(VORBISSRC)" /I ".." /Tc $<
MADCFLAGS=@$(CC) $(CFLAGS) /W0 $(DEFINES) /D "FPM_DEFAULT" /D "ASO_ZEROCHECK" \
	/D "HAVE_CONFIG_H" /I "$(MADSRC)\data" /Tc $<
OGGCFLAGS=@$(CC) $(CFLAGS) /W0 $(DEFINES) /I "$(OGGSRC)" /Tc $<
VORBISCFLAGS=@$(CC) $(CFLAGS) /W0 $(DEFINES) /I "$(OGGSRC)" /Tc $<
ZLIBCFLAGS=@$(CC) $(CFLAGS) /W0 $(DEFINES) /Tc $<



.PHONY:	ALL CHECKDIRS

CHECKDIRS:
		-@ MKDIR "$(OBJDIR)" >NUL 2>&1

ALL:	CHECKDIRS $(APP)

CLEAN:
	-@ DEL "$(APP)" "$(OBJDIR)\*.idb" "$(OBJDIR)\*.obj" "$(BINDIR)\*.pdb" \
	"$(OBJDIR)\*.res" >NUL 2>&1
	-@ RMDIR /Q "$(OBJDIR)" "$(BINDIR)" >NUL 2>&1


OBJECTS= \
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
	$(OBJDIR)\WindowsOS.obj \
\
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
	$(OBJDIR)\version.obj \
\
	$(OBJDIR)\bitwise.obj \
	$(OBJDIR)\framing.obj \
\
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
	$(OBJDIR)\window.obj \
\
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


{$(CPSRC)}.c{$(OBJDIR)}.obj::
	$(CPCFLAGS)

{$(MADSRC)}.c{$(OBJDIR)}.obj::
	$(MADCFLAGS)

{$(OGGSRC)}.c{$(OBJDIR)}.obj::
	$(OGGCFLAGS)

{$(VORBISSRC)}.c{$(OBJDIR)}.obj::
	$(VORBISCFLAGS)

{$(ZLIBSRC)}.c{$(OBJDIR)}.obj::
	$(ZLIBCFLAGS)


$(APP): $(OBJECTS)
	@$(RC) $(RFLAGS) /fo"$(OBJDIR)\coolplayer.res" "$(CPSRC)\coolplayer.rc"
	@$(LD) $(LDFLAGS) /OUT:"$(APP)" $(OBJECTS)
	@$(MT) -nologo -manifest "$(CPSRC)\res\coolplayer.exe.manifest" -outputresource:"$(APP);#1"


# Dependencies

#CPSRC
$(OBJDIR)\about.obj: $(CPSRC)\about.c $(CPSRC)\stdafx.h $(CPSRC)\globals.h
$(OBJDIR)\bitmap2region.obj: $(CPSRC)\bitmap2region.c $(CPSRC)\stdafx.h $(CPSRC)\globals.h
$(OBJDIR)\CLV_ListView.obj: $(CPSRC)\CLV_ListView.c $(CPSRC)\stdafx.h $(CPSRC)\globals.h \
	$(CPSRC)\resource.h
$(OBJDIR)\CompositeFile.obj: $(CPSRC)\CompositeFile.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CompositeFile.h $(ZLIBSRC)\zlib.h
$(OBJDIR)\coolplayer.res: $(CPSRC)\coolplayer.rc $(CPSRC)\globals.h $(CPSRC)\resource.h
$(OBJDIR)\CPI_CircleBuffer.obj: $(CPSRC)\CPI_CircleBuffer.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_CircleBuffer.h
$(OBJDIR)\CPI_Equaliser_Basic.obj: $(CPSRC)\CPI_Equaliser_Basic.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Equaliser.h
$(OBJDIR)\CPI_ID3_Genres.obj: $(CPSRC)\CPI_ID3_Genres.c $(CPSRC)\CPI_ID3.h $(CPSRC)\stdafx.h
$(OBJDIR)\CPI_Image.obj: $(CPSRC)\CPI_Image.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CompositeFile.h
$(OBJDIR)\CPI_Indicators.obj: $(CPSRC)\CPI_Indicators.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CPI_InterfacePart.h
$(OBJDIR)\CPI_Interface.obj: $(CPSRC)\CPI_Interface.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\resource.h $(CPSRC)\WindowsOS.h $(CPSRC)\CPI_InterfacePart.h
$(OBJDIR)\CPI_InterfacePart.obj: $(CPSRC)\CPI_InterfacePart.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_InterfacePart.h
$(OBJDIR)\CPI_InterfacePart_CommandButton.obj: $(CPSRC)\CPI_InterfacePart_CommandButton.c \
	$(CPSRC)\globals.h $(CPSRC)\stdafx.h $(CPSRC)\CPI_InterfacePart.h
$(OBJDIR)\CPI_InterfacePart_Indicator.obj: $(CPSRC)\CPI_InterfacePart_Indicator.c \
	$(CPSRC)\globals.h $(CPSRC)\stdafx.h $(CPSRC)\CPI_InterfacePart.h $(CPSRC)\CPI_Indicators.h
$(OBJDIR)\CPI_Keyboard.obj: $(CPSRC)\CPI_Keyboard.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\resource.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Playlist.h \
	$(CPSRC)\CPI_PlaylistItem.h
$(OBJDIR)\CPI_Player.obj: $(CPSRC)\CPI_Player.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Player_Messages.h
$(OBJDIR)\CPI_Player_Callbacks.obj: $(CPSRC)\CPI_Player_Callbacks.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_PlaylistItem.h
$(OBJDIR)\CPI_Player_CoDec_MPEG.obj: $(CPSRC)\CPI_Player_CoDec_MPEG.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player_CoDec.h $(MADSRC)\mad.h $(CPSRC)\CPI_Stream.h \
	$(CPSRC)\CPI_ID3.h
$(OBJDIR)\CPI_Player_CoDec_OGG.obj: $(CPSRC)\CPI_Player_CoDec_OGG.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Stream.h $(CPSRC)\CPI_Player_CoDec.h $(CPSRC)\CPI_ID3.h \
	$(OGGSRC)\ogg\ogg.h $(VORBISSRC)\vorbis\codec.h $(VORBISSRC)\vorbis\vorbisfile.h 
$(OBJDIR)\CPI_Player_CoDec_WAV.obj: $(CPSRC)\CPI_Player_CoDec_WAV.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player_CoDec.h $(CPSRC)\CP_RIFFStructs.h \
	$(CPSRC)\CPI_ID3.h
$(OBJDIR)\CPI_Player_CoDec_WinAmpPlugin.obj: $(CPSRC)\CPI_Player_CoDec_WinAmpPlugin.c \
	$(CPSRC)\globals.h $(CPSRC)\stdafx.h $(CPSRC)\CPI_Player_CoDec.h \
	$(CPSRC)\CP_WinAmpStructs.h $(CPSRC)\CPI_CircleBuffer.h
$(OBJDIR)\CPI_Player_Engine.obj: $(CPSRC)\CPI_Player_Engine.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player_Engine.h
$(OBJDIR)\CPI_Player_FileAssoc.obj: $(CPSRC)\CPI_Player_FileAssoc.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player_CoDec.h
$(OBJDIR)\CPI_Player_Output_DirectSound.obj: $(CPSRC)\CPI_Player_Output_DirectSound.c \
	$(CPSRC)\globals.h $(CPSRC)\stdafx.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Player_CoDec.h \
	$(CPSRC)\CPI_Player_Output.h $(CPSRC)\CPI_Equaliser.h
$(OBJDIR)\CPI_Player_Output_File.obj: $(CPSRC)\CPI_Player_Output_File.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Player_CoDec.h \
	$(CPSRC)\CPI_Player_Output.h $(CPSRC)\CPI_Equaliser.h $(CPSRC)\CPI_Playlist.h \
	$(CPSRC)\CPI_PlaylistItem.h
$(OBJDIR)\CPI_Player_Output_Wave.obj: $(CPSRC)\CPI_Player_Output_Wave.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Player_CoDec.h \
	$(CPSRC)\CPI_Player_Output.h $(CPSRC)\CPI_Equaliser.h
$(OBJDIR)\CPI_Playlist.obj: $(CPSRC)\CPI_Playlist.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CPI_Playlist.h $(CPSRC)\CPI_PlaylistItem.h $(CPSRC)\CPI_PlaylistItem_Internal.h \
	$(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Player_Engine.h
$(OBJDIR)\CPI_PlaylistItem.obj: $(CPSRC)\CPI_PlaylistItem.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Playlist.h $(CPSRC)\CPI_PlaylistItem.h \
	$(CPSRC)\CPI_PlaylistItem_Internal.h $(CPSRC)\CPI_ID3.h $(CPSRC)\CP_RIFFStructs.h \
	$(OGGSRC)\ogg\ogg.h $(VORBISSRC)\vorbis\codec.h $(VORBISSRC)\vorbis\vorbisfile.h
$(OBJDIR)\CPI_PlaylistWindow.obj: $(CPSRC)\CPI_PlaylistWindow.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\WindowsOS.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_ID3.h \
	$(CPSRC)\CPI_Playlist.h $(CPSRC)\CPI_PlaylistItem.h $(CPSRC)\CPI_PlaylistWindow.h \
	$(CPSRC)\CPI_Indicators.h
$(OBJDIR)\CPI_Playlist_Callbacks.obj: $(CPSRC)\CPI_Playlist_Callbacks.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_PlaylistItem.h $(CPSRC)\CPI_Playlist.h
$(OBJDIR)\CPI_Stream.obj: $(CPSRC)\CPI_Stream.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CPI_Stream.h
$(OBJDIR)\CPI_Stream_Internet.obj: $(CPSRC)\CPI_Stream_Internet.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Stream.h $(CPSRC)\CPI_CircleBuffer.h \
	$(CPSRC)\CPI_Player_Messages.h
$(OBJDIR)\CPI_Stream_LocalFile.obj: $(CPSRC)\CPI_Stream_LocalFile.c $(CPSRC)\globals.h \
	$(CPSRC)\stdafx.h $(CPSRC)\CPI_Stream.h
$(OBJDIR)\CPI_Verbs.obj: $(CPSRC)\CPI_Verbs.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\resource.h $(CPSRC)\CPI_Playlist.h $(CPSRC)\CPI_PlaylistWindow.h \
	$(CPSRC)\DLG_Find.h $(CPSRC)\CPI_Player.h
$(OBJDIR)\CPSK_Skin.obj: $(CPSRC)\CPSK_Skin.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\resource.h $(CPSRC)\CompositeFile.h
$(OBJDIR)\DLG_Find.obj: $(CPSRC)\DLG_Find.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\DLG_Find.h $(CPSRC)\CPI_Playlist.h $(CPSRC)\CPI_PlaylistItem.h
$(OBJDIR)\main.obj: $(CPSRC)\main.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\WindowsOS.h $(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Playlist.h \
	$(CPSRC)\CPI_PlaylistItem.h $(CPSRC)\DLG_Find.h $(CPSRC)\CPI_PlaylistWindow.h \
	$(CPSRC)\RotatingIcon.h $(CPSRC)\CPI_Indicators.h
$(OBJDIR)\options.obj: $(CPSRC)\options.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CPI_Player.h $(CPSRC)\CPI_Playlist.h
$(OBJDIR)\profile.obj: $(CPSRC)\profile.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\CPI_Playlist.h $(CPSRC)\CPI_PlaylistItem.h
$(OBJDIR)\RotatingIcon.obj: $(CPSRC)\RotatingIcon.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h
$(OBJDIR)\shwapi.obj: $(CPSRC)\shwapi.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h
$(OBJDIR)\skin.obj: $(CPSRC)\skin.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h
$(OBJDIR)\stdafx.obj: $(CPSRC)\stdafx.c $(CPSRC)\stdafx.h
$(OBJDIR)\String.obj: $(CPSRC)\String.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h
$(OBJDIR)\WindowsOS.obj: $(CPSRC)\WindowsOS.c $(CPSRC)\globals.h $(CPSRC)\stdafx.h \
	$(CPSRC)\WindowsOS.h

#mad
$(OBJDIR)\bit.obj: $(MADSRC)\bit.c
$(OBJDIR)\decoder.obj: $(MADSRC)\decoder.c
$(OBJDIR)\fixed.obj: $(MADSRC)\fixed.c
$(OBJDIR)\frame.obj: $(MADSRC)\frame.c
$(OBJDIR)\huffman.obj: $(MADSRC)\huffman.c
$(OBJDIR)\layer12.obj: $(MADSRC)\layer12.c
$(OBJDIR)\layer3.obj: $(MADSRC)\layer3.c
$(OBJDIR)\stream.obj: $(MADSRC)\stream.c
$(OBJDIR)\synth.obj: $(MADSRC)\synth.c
$(OBJDIR)\timer.obj: $(MADSRC)\timer.c
$(OBJDIR)\version.obj: $(MADSRC)\version.c

#ogg
$(OBJDIR)\bitwise.obj: $(OGGSRC)\bitwise.c
$(OBJDIR)\framing.obj: $(OGGSRC)\framing.c

#vorbis
$(OBJDIR)\analysis.obj: $(VORBISSRC)\analysis.c
$(OBJDIR)\bitrate.obj: $(VORBISSRC)\bitrate.c
$(OBJDIR)\block.obj: $(VORBISSRC)\block.c
$(OBJDIR)\codebook.obj: $(VORBISSRC)\codebook.c
$(OBJDIR)\envelope.obj: $(VORBISSRC)\envelope.c
$(OBJDIR)\floor0.obj: $(VORBISSRC)\floor0.c
$(OBJDIR)\floor1.obj: $(VORBISSRC)\floor1.c
$(OBJDIR)\info.obj: $(VORBISSRC)\info.c
$(OBJDIR)\lookup.obj: $(VORBISSRC)\lookup.c
$(OBJDIR)\lpc.obj: $(VORBISSRC)\lpc.c
$(OBJDIR)\lsp.obj: $(VORBISSRC)\lsp.c
$(OBJDIR)\mapping0.obj: $(VORBISSRC)\mapping0.c
$(OBJDIR)\mdct.obj: $(VORBISSRC)\mdct.c
$(OBJDIR)\psy.obj: $(VORBISSRC)\psy.c
$(OBJDIR)\registry.obj: $(VORBISSRC)\registry.c
$(OBJDIR)\res0.obj: $(VORBISSRC)\res0.c
$(OBJDIR)\sharedbook.obj: $(VORBISSRC)\sharedbook.c
$(OBJDIR)\smallft.obj: $(VORBISSRC)\smallft.c
$(OBJDIR)\synthesis.obj: $(VORBISSRC)\synthesis.c
$(OBJDIR)\vorbisfile.obj: $(VORBISSRC)\vorbisfile.c
$(OBJDIR)\window.obj: $(VORBISSRC)\window.c

#zlib
$(OBJDIR)\adler32.obj: $(ZLIBSRC)\adler32.c
$(OBJDIR)\compress.obj: $(ZLIBSRC)\compress.c
$(OBJDIR)\crc32.obj: $(ZLIBSRC)\crc32.c
$(OBJDIR)\deflate.obj: $(ZLIBSRC)\deflate.c
$(OBJDIR)\gzclose.obj: $(ZLIBSRC)\gzclose.c
$(OBJDIR)\gzlib.obj: $(ZLIBSRC)\gzlib.c
$(OBJDIR)\gzread.obj: $(ZLIBSRC)\gzread.c
$(OBJDIR)\gzwrite.obj: $(ZLIBSRC)\gzwrite.c
$(OBJDIR)\infback.obj: $(ZLIBSRC)\infback.c
$(OBJDIR)\inffast.obj: $(ZLIBSRC)\inffast.c
$(OBJDIR)\inflate.obj: $(ZLIBSRC)\inflate.c
$(OBJDIR)\inftrees.obj: $(ZLIBSRC)\inftrees.c
$(OBJDIR)\trees.obj: $(ZLIBSRC)\trees.c
$(OBJDIR)\uncompr.obj: $(ZLIBSRC)\uncompr.c
$(OBJDIR)\zutil.c: $(ZLIBSRC)\zutil.c