/*
	File copied to RUDL to enable bugfixing
*/

/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* Functions for audio drivers to perform runtime conversion of audio cvt->format */

#include <stdio.h>


#include "rudl.h"
#include "SDL_error.h"
#include "SDL_audio.h"
#include "rudl_audio.h"

typedef struct rudl_audio_conversion_info {
	Uint16 format;		// Source audio cvt->format
	Uint16 dst_format;		// Target audio cvt->format
	double rate_incr;		// Rate conversion increment
	Uint8 *buf;				// Buffer to hold entire audio data
	int    len;				// Length of current audio buffer
} rudl_audio_conversion_info;

// Effectively mix right and left channels into a single channel
void rudl_convert_stereo_to_mono(rudl_audio_conversion_info *cvt)
{
	int i;
	Sint32 sample;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting to mono\n");
#endif
	switch (cvt->format&0x8018) {

		case AUDIO_U8: {
			Uint8 *src, *dst;

			src = cvt->buf;
			dst = cvt->buf;
			for ( i=cvt->len/2; i; --i ) {
				sample = src[0] + src[1];
				if ( sample > 255 ) {
					*dst = 255;
				} else {
					*dst = sample;
				}
				src += 2;
				dst += 1;
			}
		}
		break;

		case AUDIO_S8: {
			Sint8 *src, *dst;

			src = (Sint8 *)cvt->buf;
			dst = (Sint8 *)cvt->buf;
			for ( i=cvt->len/2; i; --i ) {
				sample = src[0] + src[1];
				if ( sample > 127 ) {
					*dst = 127;
				} else
				if ( sample < -128 ) {
					*dst = -128;
				} else {
					*dst = sample;
				}
				src += 2;
				dst += 1;
			}
		}
		break;

		case AUDIO_U16: {
			Uint8 *src, *dst;

			src = cvt->buf;
			dst = cvt->buf;
			if ( (cvt->format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len/4; i; --i ) {
					sample = (Uint16)((src[0]<<8)|src[1])+
					         (Uint16)((src[2]<<8)|src[3]);
					if ( sample > 65535 ) {
						dst[0] = 0xFF;
						dst[1] = 0xFF;
					} else {
						dst[1] = (sample&0xFF);
						sample >>= 8;
						dst[0] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			} else {
				for ( i=cvt->len/4; i; --i ) {
					sample = (Uint16)((src[1]<<8)|src[0])+
					         (Uint16)((src[3]<<8)|src[2]);
					if ( sample > 65535 ) {
						dst[0] = 0xFF;
						dst[1] = 0xFF;
					} else {
						dst[0] = (sample&0xFF);
						sample >>= 8;
						dst[1] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			}
		}
		break;

		case AUDIO_S16: {
			Uint8 *src, *dst;

			src = cvt->buf;
			dst = cvt->buf;
			if ( (cvt->format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len/4; i; --i ) {
					sample = (Sint16)((src[0]<<8)|src[1])+
					         (Sint16)((src[2]<<8)|src[3]);
					if ( sample > 32767 ) {
						dst[0] = 0x7F;
						dst[1] = 0xFF;
					} else
					if ( sample < -32768 ) {
						dst[0] = 0x80;
						dst[1] = 0x00;
					} else {
						dst[1] = (sample&0xFF);
						sample >>= 8;
						dst[0] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			} else {
				for ( i=cvt->len/4; i; --i ) {
					sample = (Sint16)((src[1]<<8)|src[0])+
					         (Sint16)((src[3]<<8)|src[2]);
					if ( sample > 32767 ) {
						dst[1] = 0x7F;
						dst[0] = 0xFF;
					} else
					if ( sample < -32768 ) {
						dst[1] = 0x80;
						dst[0] = 0x00;
					} else {
						dst[0] = (sample&0xFF);
						sample >>= 8;
						dst[1] = (sample&0xFF);
					}
					src += 4;
					dst += 2;
				}
			}
		}
		break;
	}
	cvt->len /= 2;
}


// Duplicate a mono channel to both stereo channels
void rudl_convert_mono_to_stereo(rudl_audio_conversion_info *cvt)
{
	int i;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting to stereo\n");
#endif
	if ( (cvt->format & 0xFF) == 16 ) {
		Uint16 *src, *dst;

		src = (Uint16 *)(cvt->buf+cvt->len);
		dst = (Uint16 *)(cvt->buf+cvt->len*2);
		for ( i=cvt->len/2; i; --i ) {
			dst -= 2;
			src -= 1;
			dst[0] = src[0];
			dst[1] = src[0];
		}
	} else {
		Uint8 *src, *dst;

		src = cvt->buf+cvt->len;
		dst = cvt->buf+cvt->len*2;
		for ( i=cvt->len; i; --i ) {
			dst -= 2;
			src -= 1;
			dst[0] = src[0];
			dst[1] = src[0];
		}
	}
	cvt->len *= 2;
}

// Convert 8-bit to 16-bit - LSB
void rudl_convert_8_to_16_lsb(rudl_audio_conversion_info *cvt)
{
	int i;
	Uint8 *src, *dst;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting to 16-bit LSB\n");
#endif
	src = cvt->buf+cvt->len;
	dst = cvt->buf+cvt->len*2;
	for ( i=cvt->len; i; --i ) {
		src -= 1;
		dst -= 2;
		dst[1] = *src;
		dst[0] = 0;
	}
	cvt->format = ((cvt->format & ~0x0008) | AUDIO_U16LSB);
	cvt->len *= 2;
}
// Convert 8-bit to 16-bit - MSB
void rudl_convert_8_to_16_msb(rudl_audio_conversion_info *cvt)
{
	int i;
	Uint8 *src, *dst;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting to 16-bit MSB\n");
#endif
	src = cvt->buf+cvt->len;
	dst = cvt->buf+cvt->len*2;
	for ( i=cvt->len; i; --i ) {
		src -= 1;
		dst -= 2;
		dst[0] = *src;
		dst[1] = 0;
	}
	cvt->format = ((cvt->format & ~0x0008) | AUDIO_U16MSB);
	cvt->len *= 2;
}

// Convert 16-bit to 8-bit
void rudl_convert_16_to_8(rudl_audio_conversion_info *cvt)
{
	int i;
	Uint8 *src, *dst;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting to 8-bit\n");
#endif
	src = cvt->buf;
	dst = cvt->buf;
	if ( (cvt->format & 0x1000) != 0x1000 ) { // Little endian
		++src;
	}
	for ( i=cvt->len/2; i; --i ) {
		*dst = *src;
		src += 2;
		dst += 1;
	}
	cvt->format = ((cvt->format & ~0x9010) | AUDIO_U8);
	cvt->len /= 2;
}

// Toggle signed/unsigned
void rudl_toggle_sign(rudl_audio_conversion_info *cvt)
{
	int i;
	Uint8 *data;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting audio signedness\n");
#endif
	data = cvt->buf;
	if ( (cvt->format & 0xFF) == 16 ) {
		if ( (cvt->format & 0x1000) != 0x1000 ) { // Little endian
			++data;
		}
		for ( i=cvt->len/2; i; --i ) {
			*data ^= 0x80;
			data += 2;
		}
	} else {
		for ( i=cvt->len; i; --i ) {
			*data++ ^= 0x80;
		}
	}
	cvt->format = (cvt->format ^ 0x8000);
}

// Toggle endianness
void rudl_convert_endian(rudl_audio_conversion_info *cvt)
{
	int i;
	Uint8 *data, tmp;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting audio endianness\n");
#endif
	data = cvt->buf;
	for ( i=cvt->len/2; i; --i ) {
		tmp = data[0];
		data[0] = data[1];
		data[1] = tmp;
		data += 2;
	}
	cvt->format = (cvt->format ^ 0x1000);
}

// Convert rate up by multiple of 2
void rudl_double_samplerate(rudl_audio_conversion_info *cvt)
{
	int i;
	Uint8 *src, *dst;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting audio rate * 2\n");
#endif
	src = cvt->buf+cvt->len;
	dst = cvt->buf+cvt->len*2;
	switch (cvt->format & 0xFF) {
		case 8:
			for ( i=cvt->len; i; --i ) {
				src -= 1;
				dst -= 2;
				dst[0] = src[0];
				dst[1] = src[0];
			}
			break;
		case 16:
			for ( i=cvt->len/2; i; --i ) {
				src -= 2;
				dst -= 4;
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[0];
				dst[3] = src[1];
			}
			break;
	}
	cvt->len *= 2;
}

// Convert rate down by multiple of 2
void rudl_halve_samplerate(rudl_audio_conversion_info *cvt)
{
	int i;
	Uint8 *src, *dst;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting audio rate / 2\n");
#endif
	src = cvt->buf;
	dst = cvt->buf;
	switch (cvt->format & 0xFF) {
		case 8:
			for ( i=cvt->len/2; i; --i ) {
				dst[0] = src[0];
				src += 2;
				dst += 1;
			}
			break;
		case 16:
			for ( i=cvt->len/4; i; --i ) {
				dst[0] = src[0];
				dst[1] = src[1];
				src += 4;
				dst += 2;
			}
			break;
	}
	cvt->len /= 2;
}

// Very slow rate conversion routine
void rudl_resample(rudl_audio_conversion_info *cvt)
{
	double ipos;
	int i, clen;

#ifdef DEBUG_RUDL
	fprintf(stderr, "Converting audio rate * %4.4f\n", 1.0/cvt->rate_incr);
#endif
	clen = (int)((double)cvt->len / cvt->rate_incr);
	if ( cvt->rate_incr > 1.0 ) {
		switch (cvt->format & 0xFF) {
			case 8: {
				Uint8 *output;

				output = cvt->buf;
				ipos = 0.0;
				for ( i=clen; i; --i ) {
					*output = cvt->buf[(int)ipos];
					ipos += cvt->rate_incr;
					output += 1;
				}
			}
			break;

			case 16: {
				Uint16 *output;

				clen &= ~1;
				output = (Uint16 *)cvt->buf;
				ipos = 0.0;
				for ( i=clen/2; i; --i ) {
					*output=((Uint16 *)cvt->buf)[(int)ipos];
					ipos += cvt->rate_incr;
					output += 1;
				}
			}
			break;
		}
	} else {
		switch (cvt->format & 0xFF) {
			case 8: {
				Uint8 *output;

				output = cvt->buf+clen;
				ipos = (double)cvt->len;
				for ( i=clen; i; --i ) {
					ipos -= cvt->rate_incr;
					output -= 1;
					*output = cvt->buf[(int)ipos];
				}
			}
			break;

			case 16: {
				Uint16 *output;

				clen &= ~1;
				output = (Uint16 *)(cvt->buf+clen);
				ipos = (double)cvt->len/2;
				for ( i=clen/2; i; --i ) {
					ipos -= cvt->rate_incr;
					output -= 1;
					*output=((Uint16 *)cvt->buf)[(int)ipos];
				}
			}
			break;
		}
	}
	cvt->len = clen;
}


int rudl_convert_audio(

		Uint8* source, int source_length,

		Uint8** destination, int* destination_length,
		Uint16 src_format, Uint8 src_channels, int src_rate,
		Uint16 dst_format, Uint8 dst_channels, int dst_rate)
{
	rudl_audio_conversion_info cvt;

	RUDL_ASSERT( src_channels>0, "source channels set to 0 or less" );
	RUDL_ASSERT( dst_channels>0, "destination channels set to 0 or less" );
	RUDL_ASSERT( src_rate>0, "source samplerate set to 0 or less" );
	RUDL_ASSERT( dst_rate>0, "destination samplerate set to 0 or less" );
	RUDL_ASSERT( src_channels<=4, "more than four source channels specified" );
	RUDL_ASSERT( dst_channels<=4, "more than four destination channels specified" );

	cvt.format = src_format;
	cvt.dst_format = dst_format;
	cvt.len = source_length;
	cvt.buf=(Uint8*)malloc(8* ((double)dst_rate/src_rate) *source_length);
	memcpy(cvt.buf, source, source_length);

	// First filter:  Endian conversion from src to dst
	if ( (cvt.format & 0x1000) != (dst_format & 0x1000)
		&& ((cvt.format & 0xff) != 8) ) {
		rudl_convert_endian(&cvt);
	}
	
	// Second filter: Sign conversion -- signed/unsigned
	if ( (cvt.format & 0x8000) != (dst_format & 0x8000) ) {
		rudl_toggle_sign(&cvt);
	}

	// Next filter:  Convert 16 bit <--> 8 bit PCM
	if ( (cvt.format & 0xFF) != (dst_format & 0xFF) ) {
		switch (dst_format&0x10FF) {
			case AUDIO_U8:
				rudl_convert_16_to_8(&cvt);
				//cvt.len_ratio /= 2;
				break;
			case AUDIO_U16LSB:
				rudl_convert_8_to_16_lsb(&cvt);
				//cvt.len_mult *= 2;
				//cvt.len_ratio *= 2;
				break;
			case AUDIO_U16MSB:
				rudl_convert_8_to_16_msb(&cvt);
				//cvt.len_mult *= 2;
				//cvt.len_ratio *= 2;
				break;
		}
	}

	// Last filter:  Mono/Stereo conversion
	if ( src_channels != dst_channels ) {
		while ( (src_channels*2) <= dst_channels ) {
			rudl_convert_mono_to_stereo(&cvt);
			//cvt.len_mult *= 2;
			src_channels *= 2;
			//cvt.len_ratio *= 2;
		}
		// This assumes that 4 channel audio is in the cvt.format:
		// Left {front/back} + Right {front/back}
		// so converting to L/R stereo works properly.
		while ( ((src_channels%2) == 0) &&
				((src_channels/2) >= dst_channels) ) {
			rudl_convert_stereo_to_mono(&cvt);
			src_channels /= 2;
			//cvt.len_ratio /= 2;
		}
		if ( src_channels != dst_channels ) {
			; // Uh oh..
		}
	}

	// Do rate conversion
	cvt.rate_incr = 0.0;
	if ( (src_rate/100) != (dst_rate/100) ) {
		Uint32 hi_rate, lo_rate;
		int len_mult;
		double len_ratio;
		void (*rate_cvt)(rudl_audio_conversion_info *cvt);

		if ( src_rate > dst_rate ) {
			hi_rate = src_rate;
			lo_rate = dst_rate;
			rate_cvt = rudl_halve_samplerate;
			len_mult = 1;
			//len_ratio = 0.5;
		} else {
			hi_rate = dst_rate;
			lo_rate = src_rate;
			rate_cvt = rudl_double_samplerate;
			len_mult = 2;
			len_ratio = 2.0;
		}
		// If hi_rate = lo_rate*2^x then conversion is easy
		while ( (lo_rate*2) <= (hi_rate) ) {
			rate_cvt(&cvt);
			//cvt.len_mult *= len_mult;
			lo_rate *= 2;
			//cvt.len_ratio *= len_ratio;
		}
		// We may need a slow conversion here to finish up
		if ( (lo_rate) != (hi_rate) ) {
			if ( src_rate < dst_rate ) {
				cvt.rate_incr = (double)lo_rate/hi_rate;
				//cvt.len_mult *= 2;
				//cvt.len_ratio /= cvt.rate_incr;
			} else {
				cvt.rate_incr = (double)hi_rate/lo_rate;
				//cvt.len_ratio *= cvt.rate_incr;
			}
			rudl_resample(&cvt);
		}
	}
	*destination=cvt.buf;
	*destination_length=cvt.len;
	return 1;
}
