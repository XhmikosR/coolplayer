# *
# * makefile.deps.mak, contains the dependencies for makefile.mak
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


CP_HEADERS = $(CP_DIR)\globals.h $(CP_DIR)\stdafx.h

##################
##  coolplayer  ##
##################
$(OBJDIR)\about.obj: \
    $(CP_DIR)\about.c \
    $(CP_HEADERS)

$(OBJDIR)\bitmap2region.obj: \
    $(CP_DIR)\bitmap2region.c \
    $(CP_HEADERS)

$(OBJDIR)\CLV_ListView.obj: \
    $(CP_DIR)\CLV_ListView.c \
    $(CP_HEADERS) \
    $(CP_DIR)\resource.h

$(OBJDIR)\CompositeFile.obj: \
    $(CP_DIR)\CompositeFile.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CompositeFile.h \
    $(ZLIB_DIR)\zlib.h

$(OBJDIR)\coolplayer.res: \
    $(CP_DIR)\coolplayer.rc \
    $(CP_DIR)\globals.h \
    $(CP_DIR)\resource.h \
    $(CP_DIR)\res\about.txt \
    $(CP_DIR)\res\changes.txt \
    $(CP_DIR)\res\coolplayer.exe.manifest \
    $(CP_DIR)\res\coolplayer.ico \
    $(CP_DIR)\res\Default.CPSkin \
    $(CP_DIR)\res\keyboard.txt \
    $(CP_DIR)\res\main_bigfont.bmp \
    $(CP_DIR)\res\main_down.bmp \
    $(CP_DIR)\res\main_smallfont.bmp \
    $(CP_DIR)\res\main_up.bmp \
    $(CP_DIR)\res\mp3.ico \
    $(CP_DIR)\res\pls.ico \
    $(CP_DIR)\res\readme.txt \
    $(CP_DIR)\res\SysIcon.bmp \
    $(CP_DIR)\res\SysIcon_Mask.bmp \
    $(CP_DIR)\res\usage.txt

$(OBJDIR)\CPI_CircleBuffer.obj: \
    $(CP_DIR)\CPI_CircleBuffer.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_CircleBuffer.h

$(OBJDIR)\CPI_Equaliser_Basic.obj: \
    $(CP_DIR)\CPI_Equaliser_Basic.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Equaliser.h

$(OBJDIR)\CPI_ID3_Genres.obj: \
    $(CP_DIR)\CPI_ID3_Genres.c \
    $(CP_DIR)\CPI_ID3.h \
    $(CP_DIR)\stdafx.h

$(OBJDIR)\CPI_Image.obj: \
    $(CP_DIR)\CPI_Image.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CompositeFile.h

$(OBJDIR)\CPI_Indicators.obj: \
    $(CP_DIR)\CPI_Indicators.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_InterfacePart.h

$(OBJDIR)\CPI_Interface.obj: \
    $(CP_DIR)\CPI_Interface.c \
    $(CP_HEADERS) \
    $(CP_DIR)\resource.h \
    $(CP_DIR)\WindowsOS.h \
    $(CP_DIR)\CPI_InterfacePart.h

$(OBJDIR)\CPI_InterfacePart.obj: \
    $(CP_DIR)\CPI_InterfacePart.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_InterfacePart.h

$(OBJDIR)\CPI_InterfacePart_CommandButton.obj: \
    $(CP_DIR)\CPI_InterfacePart_CommandButton.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_InterfacePart.h

$(OBJDIR)\CPI_InterfacePart_Indicator.obj: \
    $(CP_DIR)\CPI_InterfacePart_Indicator.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_InterfacePart.h \
    $(CP_DIR)\CPI_Indicators.h

$(OBJDIR)\CPI_Keyboard.obj: \
    $(CP_DIR)\CPI_Keyboard.c \
    $(CP_HEADERS) \
    $(CP_DIR)\resource.h \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h

$(OBJDIR)\CPI_Player.obj: \
    $(CP_DIR)\CPI_Player.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Player_Messages.h

$(OBJDIR)\CPI_Player_Callbacks.obj: \
    $(CP_DIR)\CPI_Player_Callbacks.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_PlaylistItem.h

$(OBJDIR)\CPI_Player_CoDec_MPEG.obj: \
    $(CP_DIR)\CPI_Player_CoDec_MPEG.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player_CoDec.h \
    $(MAD_DIR)\mad.h \
    $(CP_DIR)\CPI_Stream.h \
    $(CP_DIR)\CPI_ID3.h

$(OBJDIR)\CPI_Player_CoDec_OGG.obj: \
    $(CP_DIR)\CPI_Player_CoDec_OGG.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Stream.h \
    $(CP_DIR)\CPI_Player_CoDec.h \
    $(CP_DIR)\CPI_ID3.h \
    $(OGG_DIR)\ogg\ogg.h \
    $(VORBIS_DIR)\vorbis\codec.h \
    $(VORBIS_DIR)\vorbis\vorbisfile.h 

$(OBJDIR)\CPI_Player_CoDec_WAV.obj: \
    $(CP_DIR)\CPI_Player_CoDec_WAV.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player_CoDec.h \
    $(CP_DIR)\CP_RIFFStructs.h \
    $(CP_DIR)\CPI_ID3.h

$(OBJDIR)\CPI_Player_CoDec_WinAmpPlugin.obj: \
    $(CP_DIR)\CPI_Player_CoDec_WinAmpPlugin.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player_CoDec.h \
    $(CP_DIR)\CP_WinAmpStructs.h \
    $(CP_DIR)\CPI_CircleBuffer.h

$(OBJDIR)\CPI_Player_Engine.obj: \
    $(CP_DIR)\CPI_Player_Engine.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player_Engine.h

$(OBJDIR)\CPI_Player_FileAssoc.obj: \
    $(CP_DIR)\CPI_Player_FileAssoc.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player_CoDec.h

$(OBJDIR)\CPI_Player_Output_DirectSound.obj: \
    $(CP_DIR)\CPI_Player_Output_DirectSound.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Player_CoDec.h \
    $(CP_DIR)\CPI_Player_Output.h \
    $(CP_DIR)\CPI_Equaliser.h

$(OBJDIR)\CPI_Player_Output_File.obj: \
    $(CP_DIR)\CPI_Player_Output_File.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Player_CoDec.h \
    $(CP_DIR)\CPI_Player_Output.h \
    $(CP_DIR)\CPI_Equaliser.h \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h

$(OBJDIR)\CPI_Player_Output_Wave.obj: \
    $(CP_DIR)\CPI_Player_Output_Wave.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Player_CoDec.h \
    $(CP_DIR)\CPI_Player_Output.h \
    $(CP_DIR)\CPI_Equaliser.h

$(OBJDIR)\CPI_Playlist.obj: \
    $(CP_DIR)\CPI_Playlist.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h \
    $(CP_DIR)\CPI_PlaylistItem_Internal.h \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Player_Engine.h

$(OBJDIR)\CPI_PlaylistItem.obj: \
    $(CP_DIR)\CPI_PlaylistItem.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h \
    $(CP_DIR)\CPI_PlaylistItem_Internal.h \
    $(CP_DIR)\CPI_ID3.h \
    $(CP_DIR)\CP_RIFFStructs.h \
    $(OGG_DIR)\ogg\ogg.h \
    $(VORBIS_DIR)\vorbis\codec.h \
    $(VORBIS_DIR)\vorbis\vorbisfile.h

$(OBJDIR)\CPI_PlaylistWindow.obj: \
    $(CP_DIR)\CPI_PlaylistWindow.c \
    $(CP_HEADERS) \
    $(CP_DIR)\WindowsOS.h \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_ID3.h \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h \
    $(CP_DIR)\CPI_PlaylistWindow.h \
    $(CP_DIR)\CPI_Indicators.h

$(OBJDIR)\CPI_Playlist_Callbacks.obj: \
    $(CP_DIR)\CPI_Playlist_Callbacks.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_PlaylistItem.h \
    $(CP_DIR)\CPI_Playlist.h

$(OBJDIR)\CPI_Stream.obj: \
    $(CP_DIR)\CPI_Stream.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Stream.h

$(OBJDIR)\CPI_Stream_Internet.obj: \
    $(CP_DIR)\CPI_Stream_Internet.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Stream.h \
    $(CP_DIR)\CPI_CircleBuffer.h \
    $(CP_DIR)\CPI_Player_Messages.h

$(OBJDIR)\CPI_Stream_LocalFile.obj: \
    $(CP_DIR)\CPI_Stream_LocalFile.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Stream.h

$(OBJDIR)\CPI_Verbs.obj: \
    $(CP_DIR)\CPI_Verbs.c \
    $(CP_HEADERS) \
    $(CP_DIR)\resource.h \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistWindow.h \
    $(CP_DIR)\DLG_Find.h \
    $(CP_DIR)\CPI_Player.h

$(OBJDIR)\CPSK_Skin.obj: \
    $(CP_DIR)\CPSK_Skin.c \
    $(CP_HEADERS) \
    $(CP_DIR)\resource.h \
    $(CP_DIR)\CompositeFile.h

$(OBJDIR)\DLG_Find.obj: \
    $(CP_DIR)\DLG_Find.c \
    $(CP_HEADERS) \
    $(CP_DIR)\DLG_Find.h \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h

$(OBJDIR)\main.obj: \
    $(CP_DIR)\main.c \
    $(CP_HEADERS) \
    $(CP_DIR)\WindowsOS.h \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h \
    $(CP_DIR)\DLG_Find.h \
    $(CP_DIR)\CPI_PlaylistWindow.h \
    $(CP_DIR)\RotatingIcon.h \
    $(CP_DIR)\CPI_Indicators.h

$(OBJDIR)\options.obj: \
    $(CP_DIR)\options.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Player.h \
    $(CP_DIR)\CPI_Playlist.h

$(OBJDIR)\profile.obj: \
    $(CP_DIR)\profile.c \
    $(CP_HEADERS) \
    $(CP_DIR)\CPI_Playlist.h \
    $(CP_DIR)\CPI_PlaylistItem.h

$(OBJDIR)\RotatingIcon.obj: \
    $(CP_DIR)\RotatingIcon.c \
    $(CP_HEADERS)

$(OBJDIR)\shwapi.obj: \
    $(CP_DIR)\shwapi.c \
    $(CP_HEADERS)

$(OBJDIR)\skin.obj: \
    $(CP_DIR)\skin.c \
    $(CP_HEADERS)

$(OBJDIR)\stdafx.obj: \
    $(CP_DIR)\stdafx.c \
    $(CP_DIR)\stdafx.h

$(OBJDIR)\String.obj: \
    $(CP_DIR)\String.c \
    $(CP_HEADERS)

$(OBJDIR)\WindowsOS.obj: \
    $(CP_DIR)\WindowsOS.c \
    $(CP_HEADERS) \
    $(CP_DIR)\WindowsOS.h


###########
##  mad  ##
###########
$(OBJDIR)\bit.obj: $(MAD_DIR)\bit.c
$(OBJDIR)\decoder.obj: $(MAD_DIR)\decoder.c
$(OBJDIR)\fixed.obj: $(MAD_DIR)\fixed.c
$(OBJDIR)\frame.obj: $(MAD_DIR)\frame.c
$(OBJDIR)\huffman.obj: $(MAD_DIR)\huffman.c
$(OBJDIR)\layer12.obj: $(MAD_DIR)\layer12.c
$(OBJDIR)\layer3.obj: $(MAD_DIR)\layer3.c
$(OBJDIR)\stream.obj: $(MAD_DIR)\stream.c
$(OBJDIR)\synth.obj: $(MAD_DIR)\synth.c
$(OBJDIR)\timer.obj: $(MAD_DIR)\timer.c
$(OBJDIR)\version.obj: $(MAD_DIR)\version.c


###########
##  ogg  ##
###########
$(OBJDIR)\bitwise.obj: $(OGG_DIR)\bitwise.c
$(OBJDIR)\framing.obj: $(OGG_DIR)\framing.c


##############
##  vorbis  ##
##############
$(OBJDIR)\analysis.obj: $(VORBIS_DIR)\analysis.c
$(OBJDIR)\bitrate.obj: $(VORBIS_DIR)\bitrate.c
$(OBJDIR)\block.obj: $(VORBIS_DIR)\block.c
$(OBJDIR)\codebook.obj: $(VORBIS_DIR)\codebook.c
$(OBJDIR)\envelope.obj: $(VORBIS_DIR)\envelope.c
$(OBJDIR)\floor0.obj: $(VORBIS_DIR)\floor0.c
$(OBJDIR)\floor1.obj: $(VORBIS_DIR)\floor1.c
$(OBJDIR)\info.obj: $(VORBIS_DIR)\info.c
$(OBJDIR)\lookup.obj: $(VORBIS_DIR)\lookup.c
$(OBJDIR)\lpc.obj: $(VORBIS_DIR)\lpc.c
$(OBJDIR)\lsp.obj: $(VORBIS_DIR)\lsp.c
$(OBJDIR)\mapping0.obj: $(VORBIS_DIR)\mapping0.c
$(OBJDIR)\mdct.obj: $(VORBIS_DIR)\mdct.c
$(OBJDIR)\psy.obj: $(VORBIS_DIR)\psy.c
$(OBJDIR)\registry.obj: $(VORBIS_DIR)\registry.c
$(OBJDIR)\res0.obj: $(VORBIS_DIR)\res0.c
$(OBJDIR)\sharedbook.obj: $(VORBIS_DIR)\sharedbook.c
$(OBJDIR)\smallft.obj: $(VORBIS_DIR)\smallft.c
$(OBJDIR)\synthesis.obj: $(VORBIS_DIR)\synthesis.c
$(OBJDIR)\vorbisfile.obj: $(VORBIS_DIR)\vorbisfile.c
$(OBJDIR)\window.obj: $(VORBIS_DIR)\window.c


############
##  zlib  ##
############
$(OBJDIR)\adler32.obj: $(ZLIB_DIR)\adler32.c
$(OBJDIR)\compress.obj: $(ZLIB_DIR)\compress.c
$(OBJDIR)\crc32.obj: $(ZLIB_DIR)\crc32.c
$(OBJDIR)\deflate.obj: $(ZLIB_DIR)\deflate.c
$(OBJDIR)\gzclose.obj: $(ZLIB_DIR)\gzclose.c
$(OBJDIR)\gzlib.obj: $(ZLIB_DIR)\gzlib.c
$(OBJDIR)\gzread.obj: $(ZLIB_DIR)\gzread.c
$(OBJDIR)\gzwrite.obj: $(ZLIB_DIR)\gzwrite.c
$(OBJDIR)\infback.obj: $(ZLIB_DIR)\infback.c
$(OBJDIR)\inffast.obj: $(ZLIB_DIR)\inffast.c
$(OBJDIR)\inflate.obj: $(ZLIB_DIR)\inflate.c
$(OBJDIR)\inftrees.obj: $(ZLIB_DIR)\inftrees.c
$(OBJDIR)\trees.obj: $(ZLIB_DIR)\trees.c
$(OBJDIR)\uncompr.obj: $(ZLIB_DIR)\uncompr.c
$(OBJDIR)\zutil.c: $(ZLIB_DIR)\zutil.c
