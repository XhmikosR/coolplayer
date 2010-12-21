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

/*
 * MPEG Audio codec (using MAD)
 * Copyright (C) 2001 Robert Leslie
 *
 * 19-Aug-2001: Created initial version
 */


#include "stdafx.h"
#include "globals.h"
#include "CPI_Player_CoDec.h"
#include "mad.h"
// #include "mmsystem.h"
#include "CPI_Stream.h"
#include "CPI_ID3.h"
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

# define SAMPLE_DEPTH 16
# define scale(x, y) dither((x), (y))

struct xing
{
	long flags;
	unsigned long frames;
	unsigned long bytes;
	unsigned char toc[100];
	long scale;
};

enum
{
	XING_FRAMES = 0x00000001L,
	XING_BYTES  = 0x00000002L,
	XING_TOC    = 0x00000004L,
	XING_SCALE  = 0x00000008L
};

# define XING_MAGIC (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g')

struct dither
{
	mad_fixed_t error[3];
	mad_fixed_t random;
};

typedef struct
{
	CPs_InStream* m_pInStream;
	
	unsigned long size;    /* stream size, in bytes */
	CPs_CoDecOptions options;  /* codec options */
	struct xing xing;    /* Xing VBR tag data */
	struct mad_stream stream;  /* MAD stream structure */
	struct mad_frame frame;   /* MAD frame structure */
	struct mad_synth synth;   /* MAD synth structure */
	
	unsigned int samplecount;  /* samples output from current frame */
	
	mad_timer_t timer;    /* current playing time position */
	mad_timer_t length;    /* total playing time of current stream */
	unsigned long rate;    /* bitrate sum for computing average */
	unsigned long frames;   /* number of frames decoded */
	
	CPs_FileInfo info;    /* stream info */
	
	unsigned char buffer[40000]; /* input stream buffer */
	unsigned int buflen;   /* input stream buffer length */
} CPs_CoDec_MPEG;

/*
 * NAME:  parse_xing()
 * DESCRIPTION: read a Xing VBR tag
 */
static
int parse_xing(struct xing *xing, struct mad_bitptr ptr, unsigned int bitlen)
{
	if (bitlen < 64 || mad_bit_read(&ptr, 32) != XING_MAGIC)
		goto fail;
		
	xing->flags = mad_bit_read(&ptr, 32);
	
	bitlen -= 64;
	
	if (xing->flags & XING_FRAMES)
	{
		if (bitlen < 32)
			goto fail;
			
		xing->frames = mad_bit_read(&ptr, 32);
		
		bitlen -= 32;
	}
	
	if (xing->flags & XING_BYTES)
	{
		if (bitlen < 32)
			goto fail;
			
		xing->bytes = mad_bit_read(&ptr, 32);
		
		bitlen -= 32;
	}
	
	if (xing->flags & XING_TOC)
	{
		int i;
		
		if (bitlen < 800)
			goto fail;
			
		for (i = 0; i < 100; ++i)
			xing->toc[i] = (unsigned char) mad_bit_read(&ptr, 8);
			
		bitlen -= 800;
	}
	
	if (xing->flags & XING_SCALE)
	{
		if (bitlen < 32)
			goto fail;
			
		xing->scale = mad_bit_read(&ptr, 32);
		
		bitlen -= 32;
	}
	
	return 0;
	
fail:
	xing->flags = 0;
	return -1;
}

/*
 * NAME:  scan_header()
 * DESCRIPTION: read the initial frame(s) to get stream statistics
 */
static
int scan_header(CPs_InStream* pInStream, struct mad_header *header, struct xing *xing)
{

	struct mad_stream stream;
	
	struct mad_frame frame;
	unsigned char buffer[8192];
	unsigned int buflen = 0;
	int count = 0, result = 0;
	
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	
	if (xing)
		xing->flags = 0;
		
	while (1)
	{
		if (buflen < sizeof(buffer))
		{
			// DWORD bytes;
			unsigned int bytes;
			
			if (pInStream->Read(pInStream, buffer + buflen, sizeof(buffer) - buflen, &bytes) == FALSE
					|| bytes == 0)
			{
				result = -1;
				break;
			}
			
			buflen += bytes;
		}
		
		mad_stream_buffer(&stream, buffer, buflen);
		
		while (1)
		{
			if (mad_frame_decode(&frame, &stream) == -1)
			{
				if (!MAD_RECOVERABLE(stream.error))
					break;
					
				continue;
			}
			
			if (count++ ||
					(xing && parse_xing(xing, stream.anc_ptr,
										stream.anc_bitlen) == 0))
				break;
		}
		
		if (count || stream.error != MAD_ERROR_BUFLEN)
			break;
			
		memmove(buffer, stream.next_frame,
				buflen = &buffer[buflen] - stream.next_frame);
	}
	
	if (count)
	{
		if (header)
			*header = frame.header;
	}
	
	else
		result = -1;
		
	mad_frame_finish(&frame);
	
	mad_stream_finish(&stream);
	
	return result;
}


/*
 * NAME:  prng()
 * DESCRIPTION: 32-bit pseudo-random number generator
 */
static __inline
unsigned long prng(unsigned long state)
{
	return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

/*
 * NAME:  dither()
 * DESCRIPTION: dither and scale sample
 */
static __inline
signed int dither(mad_fixed_t sample, struct dither *dither)
{
	unsigned int scalebits;
	mad_fixed_t output, mask, random;
	
	enum
	{
		MIN = -MAD_F_ONE,
		MAX =  MAD_F_ONE - 1
	};
	
	/* noise shape */
	sample += dither->error[0] - dither->error[1] + dither->error[2];
	
	dither->error[2] = dither->error[1];
	dither->error[1] = dither->error[0] / 2;
	
	/* bias */
	output = sample + (1L << (MAD_F_FRACBITS + 1 - SAMPLE_DEPTH - 1));
	
	scalebits = MAD_F_FRACBITS + 1 - SAMPLE_DEPTH;
	mask = (1L << scalebits) - 1;
	
	/* dither */
	random  = prng(dither->random);
	output += (random & mask) - (dither->random & mask);
	
	dither->random = random;
	
	/* clip */
	
	if (output > MAX)
	{
		output = MAX;
		
		if (sample > MAX)
			sample = MAX;
	}
	
	else if (output < MIN)
	{
		output = MIN;
		
		if (sample < MIN)
			sample = MIN;
	}
	
	/* quantize */
	output &= ~mask;
	
	/* error feedback */
	dither->error[0] = sample - output;
	
	/* scale */
	return output >> scalebits;
}

/*
 * NAME:  pack_pcm()
 * DESCRIPTION: scale and dither MAD output
 */
static
void pack_pcm(unsigned char **pcm, unsigned int nsamples,
			  mad_fixed_t const *ch1, mad_fixed_t const *ch2)
{
	register signed int s0, s1;
	
	static struct dither d0, d1;
	
	if (ch2)    /* stereo */
	{
		while (nsamples--)
		{
			s0 = scale(*ch1++, &d0);
			s1 = scale(*ch2++, &d1);
# if SAMPLE_DEPTH == 16
			(*pcm)[0 + 0] = s0 >> 0;
			(*pcm)[0 + 1] = s0 >> 8;
			(*pcm)[2 + 0] = s1 >> 0;
			(*pcm)[2 + 1] = s1 >> 8;
			
			*pcm += 2 * 2;
# elif SAMPLE_DEPTH == 8
			(*pcm)[0] = s0 ^ 0x80;
			(*pcm)[1] = s1 ^ 0x80;
			
			*pcm += 2;
# else
#  error "bad SAMPLE_DEPTH"
# endif
		}
	}
	
	else    /* mono */
	{
		while (nsamples--)
		{
			s0 = scale(*ch1++, &d0);
			
# if SAMPLE_DEPTH == 16
			(*pcm)[0] = s0 >> 0;
			(*pcm)[1] = s0 >> 8;
			
			*pcm += 2;
# elif SAMPLE_DEPTH == 8
			*(*pcm)++ = s0 ^ 0x80;
# endif
		}
	}
}

/*
 * NAME:  cleanup()
 * DESCRIPTION: close file and free memory
 */
static void cleanup(CPs_CoDec_MPEG *context)
{
	if (context->m_pInStream)
	{
		context->m_pInStream->Uninitialise(context->m_pInStream);
		context->m_pInStream = NULL;
		
		mad_synth_finish(&context->synth);
		mad_frame_finish(&context->frame);
		mad_stream_finish(&context->stream);
	}
}



////////////////////////////////////////////////////////////////////////////////
//
//
// Module functions
void CPP_OMMP3_Uninitialise(CPs_CoDecModule* pModule);
BOOL CPP_OMMP3_OpenFile(CPs_CoDecModule* pModule, const char* pcFilename, DWORD dwCookie, HWND hWndOwner);
void CPP_OMMP3_CloseFile(CPs_CoDecModule* pModule);
void CPP_OMMP3_Seek(CPs_CoDecModule* pModule, const int iNumerator, const int iDenominator);
void CPP_OMMP3_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo* pInfo);
//
BOOL CPP_OMMP3_GetPCMBlock(CPs_CoDecModule* pModule, void* pBlock, DWORD* pdwBlockSize);
int CPP_OMMP3_GetCurrentPos_secs(CPs_CoDecModule* pModule);
//
void CP_InitialiseCodec_MPEG(CPs_CoDecModule* pCoDec)
{
	CPs_CoDec_MPEG *pContext;
	
	// Setup functions
	pCoDec->Uninitialise = CPP_OMMP3_Uninitialise;
	pCoDec->OpenFile = CPP_OMMP3_OpenFile;
	pCoDec->CloseFile = CPP_OMMP3_CloseFile;
	pCoDec->Seek = CPP_OMMP3_Seek;
	pCoDec->GetFileInfo = CPP_OMMP3_GetFileInfo;
	
	pCoDec->GetPCMBlock = CPP_OMMP3_GetPCMBlock;
	pCoDec->GetCurrentPos_secs = CPP_OMMP3_GetCurrentPos_secs;
	
	// Setup private data
	pCoDec->m_pModuleCookie = malloc(sizeof(CPs_CoDec_MPEG));
	pContext = (CPs_CoDec_MPEG*)pCoDec->m_pModuleCookie;
	pContext->m_pInStream = NULL;
	pContext->size = 0;
	
	pContext->options.m_iPretendOption = 42;
	
	pContext->xing.flags = 0;
	
	pContext->samplecount = 0;
	
	pContext->timer  = mad_timer_zero;
	pContext->length = mad_timer_zero;
	pContext->rate   = 0;
	pContext->frames = 0;
	
	pContext->info.m_iFileLength_Secs = 0;
	pContext->info.m_iBitRate_Kbs     = 0;
	pContext->info.m_iFreq_Hz         = 0;
	pContext->info.m_bStereo          = TRUE;
	pContext->info.m_b16bit           = (SAMPLE_DEPTH == 16);
	
	pContext->buflen = 0;
	
	CPFA_InitialiseFileAssociations(pCoDec);
	CPFA_AddFileAssociation(pCoDec, "MP3", 0L);
	CPFA_AddFileAssociation(pCoDec, "MP2", 0L);
	CPFA_AddFileAssociation(pCoDec, "MP1", 0L);
}


/*
 * NAME:  codec->Destroy()
 * DESCRIPTION: clean-up and delete a codec object
 */
void CPP_OMMP3_Uninitialise(CPs_CoDecModule* pModule)
{
	CPs_CoDec_MPEG *context = (CPs_CoDec_MPEG *)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(context);
	
	cleanup(context);
	
	free(context);
	
	CPFA_EmptyFileAssociations(pModule);
}

/*
 * NAME:  codec->OpenFile()
 * DESCRIPTION: open a new MPEG audio stream
 */
BOOL CPP_OMMP3_OpenFile(CPs_CoDecModule* pModule, char const *path, DWORD dwCookie, HWND hWndOwner)
{
	CPs_CoDec_MPEG *context = (CPs_CoDec_MPEG *)pModule->m_pModuleCookie;
	int iStreamStart = 0;
	
	CP_CHECKOBJECT(context);
	
	cleanup(context);
	
	CP_TRACE1("CPI_CoDec__OpenFile: \"%s\"", path);
	
	context->m_pInStream = CP_CreateInStream(path, hWndOwner);
	
	if (!context->m_pInStream)
	{
		CP_TRACE0("CPI_CoDec__OpenFile: failed");
		return FALSE;
	}
	
	// Skip over ID3v2 tag (if there is one)
	
	if (context->m_pInStream->IsSeekable(context->m_pInStream) == TRUE) // only works on seekable streams
	{
		CIs_ID3v2Tag tag;
		unsigned int iBytesRead;
		
		memset(&tag, 0, sizeof(tag));
		context->m_pInStream->Read(context->m_pInStream, &tag, sizeof(tag), &iBytesRead);
		
		if (memcmp(tag.m_cTAG, "ID3", 3) == 0)
		{
			iStreamStart = sizeof(CIs_ID3v2Tag);
			iStreamStart += (tag.m_cSize_Encoded[0] << 21)
							| (tag.m_cSize_Encoded[1] << 14)
							| (tag.m_cSize_Encoded[2] << 7)
							| tag.m_cSize_Encoded[3];
		}
		
		context->m_pInStream->Seek(context->m_pInStream, iStreamStart);
	}
	
	mad_stream_init(&context->stream);
	
	mad_frame_init(&context->frame);
	mad_synth_init(&context->synth);
	
	if (scan_header(context->m_pInStream,
					&context->frame.header, &context->xing) == -1)
	{
		CP_TRACE0("CPI_CoDec__OpenFile: failed to read file header");
		cleanup(context);
		return FALSE;
	}
	
	context->m_pInStream->Seek(context->m_pInStream, iStreamStart);
	
	context->size             = context->m_pInStream->GetLength(context->m_pInStream);
	context->synth.pcm.length = 0;
	context->samplecount      = 0;
	context->timer            = mad_timer_zero;
	
	if (context->xing.flags & XING_FRAMES)
	{
		context->length = context->frame.header.duration;
		mad_timer_multiply(&context->length, context->xing.frames);
	}
	
	else
	{
		/* estimate playing time from file size */
		
		mad_timer_set(&context->length, 0,
					  1, context->frame.header.bitrate / 8);
		mad_timer_multiply(&context->length, context->size);
	}
	
	context->rate   = 0;
	
	context->frames = 0;
	
	context->info.m_iFileLength_Secs =
		mad_timer_count(context->length, MAD_UNITS_SECONDS);
	context->info.m_iBitRate_Kbs     = context->frame.header.bitrate / 1000;
	context->info.m_iFreq_Hz         = context->frame.header.samplerate;
	context->info.m_bStereo          =
		context->frame.header.mode == MAD_MODE_SINGLE_CHANNEL ? FALSE : TRUE;
	context->info.m_b16bit           = (SAMPLE_DEPTH == 16);
	
	context->buflen = 0;
	
	return TRUE;
}

/*
 * NAME:  codec->CloseFile()
 * DESCRIPTION: Closes the file
 */
void CPP_OMMP3_CloseFile(CPs_CoDecModule* pModule)
{
	CPs_CoDec_MPEG *context = (CPs_CoDec_MPEG *)pModule->m_pModuleCookie;
	CP_CHECKOBJECT(context);
	CP_TRACE0("Close MPEG file");
	
	cleanup(context);
}

/*
 * NAME:  codec->Seek()
 * DESCRIPTION: seek to a specific location
 */
void CPP_OMMP3_Seek(CPs_CoDecModule* pModule, int const numer, int const denom)
{
	CPs_CoDec_MPEG *context = (CPs_CoDec_MPEG *)pModule->m_pModuleCookie;
	double fraction;
	unsigned long position;
	
	CP_CHECKOBJECT(context);
	CP_ASSERT(context->m_pInStream);
	CP_ASSERT(numer >= 0 && denom > 0);
	
	// If the IStream doesn't support seeking - ignore the seek
	
	if (context->m_pInStream->IsSeekable(context->m_pInStream) == FALSE)
		return;
		
	fraction = (double) numer / denom;
	
	position = (unsigned long)
			   (mad_timer_count(context->length, MAD_UNITS_MILLISECONDS) * fraction);
	           
	mad_timer_set(&context->timer, position / 1000, position % 1000, 1000);
	
	if (context->xing.flags & XING_TOC)
	{
		int percent, p1, p2;
		
		percent = (int)(fraction * 100);
		p1 = (percent < 100) ? context->xing.toc[percent    ] : 0x100;
		p2 = (percent <  99) ? context->xing.toc[percent + 1] : 0x100;
		
		fraction = (p1 + (p2 - p1) * (fraction * 100 - percent)) / 0x100;
	}
	
	context->m_pInStream->Seek(context->m_pInStream, (LONG)(context->size * fraction));
	
	if (context->m_pInStream->Read(context->m_pInStream, context->buffer, sizeof(context->buffer), &context->buflen) == FALSE)
		context->buflen = 0;
		
	mad_stream_buffer(&context->stream, context->buffer, context->buflen);
	
	mad_frame_mute(&context->frame);
	
	mad_synth_mute(&context->synth);
	
	if (numer)
	{
		int skip;
		
		skip = 2;
		
		do
		{
			if (mad_frame_decode(&context->frame, &context->stream) == 0)
			{
				mad_timer_add(&context->timer, context->frame.header.duration);
				
				if (--skip == 0)
					mad_synth_frame(&context->synth, &context->frame);
			}
			
			else if (!MAD_RECOVERABLE(context->stream.error))
				break;
		} while (skip);
	}
	
	context->synth.pcm.length = 0;
	
	context->samplecount      = 0;
}


/*
 * NAME:  codec->GetPCMBlock()
 * DESCRIPTION: decode and return the next PCM block
 */
BOOL CPP_OMMP3_GetPCMBlock(CPs_CoDecModule* pModule, void *block, DWORD *size)
{
	CPs_CoDec_MPEG *context = (CPs_CoDec_MPEG *)pModule->m_pModuleCookie;
	unsigned char *samples = block;
	unsigned int nsamples;
	
	CP_CHECKOBJECT(context);
	CP_ASSERT(context->m_pInStream);
	
	nsamples = (*size / (SAMPLE_DEPTH / 8)) >>
			   (context->info.m_bStereo ? 1 : 0);
	*size = 0;
	
	while (nsamples)
	{
		unsigned int count, bitrate;
		
		count = context->synth.pcm.length - context->samplecount;
		
		if (count > nsamples)
			count = nsamples;
			
		if (count)
		{
			mad_fixed_t const *ch1, *ch2;
			
			ch1 = context->synth.pcm.samples[0] + context->samplecount;
			ch2 = context->synth.pcm.samples[1] + context->samplecount;
			
			if (context->info.m_bStereo == FALSE)
				ch2 = 0;
			else if (context->synth.pcm.channels == 1)
				ch2 = ch1;
				
			pack_pcm(&samples, count, ch1, ch2);
			
			context->samplecount += count;
			
			nsamples             -= count;
			
			if (nsamples == 0)
				break;
		}
		
		while (mad_frame_decode(&context->frame, &context->stream) == -1)
		{
			// DWORD bytes;
			unsigned int bytes;
			
			if (MAD_RECOVERABLE(context->stream.error))
				continue;
				
			if (context->stream.next_frame)
			{
				memmove(context->buffer, context->stream.next_frame,
						context->buflen = context->buffer +
										  context->buflen - context->stream.next_frame);
			}
			
			if (context->m_pInStream->Read(context->m_pInStream,
										   context->buffer + context->buflen,
										   sizeof(context->buffer) - context->buflen, &bytes) == FALSE
					|| bytes == 0)
			{
				return FALSE;
			}
			
			mad_stream_buffer(&context->stream,
			
							  context->buffer, context->buflen += bytes);
		}
		
		bitrate = context->frame.header.bitrate / 1000;
		
		context->rate += bitrate;
		context->frames++;
		
		context->info.m_iBitRate_Kbs = bitrate;
		
		mad_synth_frame(&context->synth, &context->frame);
		
		context->samplecount = 0;
		
		mad_timer_add(&context->timer, context->frame.header.duration);
	}
	
	*size = samples - (unsigned char *) block;
	
	return TRUE;
}

/*
 * NAME:  codec->GetFileInfo()
 * DESCRIPTION: return statistics about the current stream
 */
void CPP_OMMP3_GetFileInfo(CPs_CoDecModule* pModule, CPs_FileInfo *info)
{
	CPs_CoDec_MPEG *context = (CPs_CoDec_MPEG *)pModule->m_pModuleCookie;
	
	CP_CHECKOBJECT(context);
	
	if (!(context->xing.flags & XING_FRAMES) && context->frames)
	{
		/* update length estimate */
		
		mad_timer_set(&context->length, 0,
					  1, (context->rate / context->frames) * (1000 / 8));
		mad_timer_multiply(&context->length, context->size);
		
		if (mad_timer_compare(context->timer, context->length) > 0)
		{
			context->length = context->timer;
			context->size   = context->m_pInStream->GetLength(context->m_pInStream);
		}
		
		context->info.m_iFileLength_Secs =
		
			mad_timer_count(context->length, MAD_UNITS_SECONDS);
	}
	
	*info = context->info;
}

/*
 * NAME:  codec->GetCurrentPos_secs()
 * DESCRIPTION: return the current playing time position
 */
int CPP_OMMP3_GetCurrentPos_secs(CPs_CoDecModule* pModule)
{
	CPs_CoDec_MPEG *context = (CPs_CoDec_MPEG *)pModule->m_pModuleCookie;
	
	CP_CHECKOBJECT(context);
	
	return mad_timer_count(context->timer, MAD_UNITS_SECONDS);
}

/*
 * The following makes editing nicer under Emacs!
 *
 * Local Variables:
 * c-indentation-style: "msvc++"
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
