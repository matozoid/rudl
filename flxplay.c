/*      vi:set nowrap ts=2 sw=2:
*/
#define flxplay_version "0.2"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rudl.h"
#include "rudl_video.h"

VALUE classFLCDecoder;

typedef struct {
  FILE *file;
  Uint8 *pMembuf;
  Uint32 membufSize;
  Uint8 *pFrame;
  Uint8 *pChunk;
  Uint16 FrameCount;    // Frame Counter
  //
  Uint32 HeaderSize;    // Fli file size
  Uint16 HeaderCheck;   // Fli header check
  Uint16 HeaderFrames;  // Number of frames in flic
  Uint16 HeaderWidth;   // Fli width
  Uint16 HeaderHeight;  // Fli heigth
  Uint16 HeaderDepth;   // Color depth
  Uint16 HeaderSpeed;   // Number of video ticks between frame
  Uint32 FrameSize;     // Frame size in bytes
  Uint16 FrameCheck;    // Frame check
  Uint16 FrameChunks;   // Number of chunks in frame
  Uint32 ChunkSize;     // Size of chunk
  Uint16 ChunkType;     // Type of chunk
  //
  VALUE surface;
  SDL_Color colors[256];
  int screen_w;
  int screen_h;
  int screen_depth;
  int loop;
  int lastFrameTicks;
} FLC;

#define ReadU16(tmp1, tmp2) (Uint16)*(tmp1) = ((Uint8)*(tmp2+1)<<8)+(Uint8)*(tmp2);
#define ReadU32(tmp1, tmp2) (Uint32)*(tmp1) = (((((((Uint8)*(tmp2+3)<<8)+((Uint8)*(tmp2+2)))<<8)+((Uint8)*(tmp2+1)))<<8)+(Uint8)*(tmp2));


void FlcReadFile(FLC*flc, Uint32 size)
{ 
	if(size>flc->membufSize) {
		RUDL_VERIFY((flc->pMembuf=realloc(flc->pMembuf, size+1)), "Realloc failed");
	}
	RUDL_VERIFY(fread(flc->pMembuf, sizeof(Uint8), size, flc->file)!=0, "Can't read flx file");
}

int FlcCheckHeader(FLC*flc ,char *filename)
{ 
	if((flc->file=fopen(filename, "rb"))==NULL) {
		return(0);
	}

	FlcReadFile(flc, 128);

	ReadU32(&flc->HeaderSize, flc->pMembuf);
	ReadU16(&flc->HeaderCheck, flc->pMembuf+4);
	ReadU16(&flc->HeaderFrames, flc->pMembuf+6);
	ReadU16(&flc->HeaderWidth, flc->pMembuf+8);
	ReadU16(&flc->HeaderHeight, flc->pMembuf+10);
	ReadU16(&flc->HeaderDepth, flc->pMembuf+12);
	ReadU16(&flc->HeaderSpeed, flc->pMembuf+16);

	if((flc->HeaderCheck==0x0AF12) || (flc->HeaderCheck==0x0AF11)) { 
		flc->screen_w=flc->HeaderWidth;
		flc->screen_h=flc->HeaderHeight;
		flc->screen_depth=8;
		if(flc->HeaderCheck==0x0AF11) {
			flc->HeaderSpeed*=1000/70;
		}
		return(1);
	}
	return(0);
}

int FlcCheckFrame(FLC*flc)
{ 
	flc->pFrame=flc->pMembuf+flc->FrameSize-16;
	ReadU32(&flc->FrameSize, flc->pFrame+0);
	ReadU16(&flc->FrameCheck, flc->pFrame+4);
	ReadU16(&flc->FrameChunks, flc->pFrame+6);


	flc->pFrame+=16;
	if(flc->FrameCheck==0x0f1fa) { 
		return(0);
	}

	if(flc->FrameCheck==0x0f100) { 
		return(0);
	}

	return(1);
}

void COLORS256(FLC*flc)
{ 
	Uint8 *pSrc;
	Uint16 NumColorPackets;
	Uint16 NumColors;
	Uint8 NumColorsSkip;
	int i;
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);
	
	pSrc=flc->pChunk+6;
	ReadU16(&NumColorPackets, pSrc);
	pSrc+=2;
	while(NumColorPackets--) {
		NumColorsSkip=*(pSrc++);
		if(!(NumColors=*(pSrc++))) {
			NumColors=256;
		}
		i=0;
		while(NumColors--) {
			flc->colors[i].r=*(pSrc++);
			flc->colors[i].g=*(pSrc++);
			flc->colors[i].b=*(pSrc++);
			i++;
		}
		SDL_SetColors(surface, flc->colors, NumColorsSkip, i);
	}
}

void SS2(FLC*flc)
{ Uint8 *pSrc, *pDst, *pTmpDst;
  Sint8 CountData;
  Uint8 ColumSkip, Fill1, Fill2;
  Uint16 Lines, Count;
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);

  pSrc=flc->pChunk+6;
  pDst=surface->pixels;
  ReadU16(&Lines, pSrc);
  pSrc+=2;
  while(Lines--) {
    ReadU16(&Count, pSrc);
    pSrc+=2;

    while(Count & 0xc000) {
// Upper bits 11 - Lines skip 

      if((Count & 0xc000)==0xc000) {  // 0xc000h = 1100000000000000
        pDst+=(0x10000-Count)*surface->pitch;
      }

      if((Count & 0xc000)==0x4000) {  // 0x4000h = 0100000000000000
// Upper bits 01 - Last pixel

#ifdef DEBUG
            printf("Last pixel not implemented");
#endif
      }
      ReadU16(&Count, pSrc);
      pSrc+=2;
    }

    if((Count & 0xc000)==0x0000) {      // 0xc000h = 1100000000000000
      pTmpDst=pDst;
      while(Count--) {
        ColumSkip=*(pSrc++);
        pTmpDst+=ColumSkip;
        CountData=*(pSrc++);
        if(CountData>0) {
          while(CountData--) {
            *(pTmpDst++)=*(pSrc++);
            *(pTmpDst++)=*(pSrc++);
          }
        } else { 
          if(CountData<0) {
            CountData=(0x100-CountData);
            Fill1=*(pSrc++);
            Fill2=*(pSrc++);
            while(CountData--) {
              *(pTmpDst++)=Fill1;
              *(pTmpDst++)=Fill2;
            }
          }
        }
      }
      pDst+=surface->pitch;
    } 
  }
}

void DECODE_BRUN(FLC*flc)
{ Uint8 *pSrc, *pDst, *pTmpDst, Fill;
  Sint8 CountData;
  int HeightCount, PacketsCount;
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);

  HeightCount=flc->HeaderHeight;
  pSrc=flc->pChunk+6;
  pDst=surface->pixels;
  while(HeightCount--) {
    pTmpDst=pDst;
    PacketsCount=*(pSrc++);
    while(PacketsCount--) {
      CountData=*(pSrc++);
      if(CountData>0) {
        Fill=*(pSrc++);
        while(CountData--) {
          *(pTmpDst++)=Fill;
        }
      } else { 
        if(CountData<0) {
          CountData=(0x100-CountData);
          while(CountData--) {
          *(pTmpDst++)=*(pSrc++);
          }
        }
      }
    }
    pDst+=surface->pitch;
  }
}


void DECODE_LC(FLC*flc) 
{ Uint8 *pSrc, *pDst, *pTmpDst;
  Sint8 CountData;
  Uint8 CountSkip;
  Uint8 Fill;
  Uint16 Lines, tmp;
  int PacketsCount;
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);

  pSrc=flc->pChunk+6;
  pDst=surface->pixels;

  ReadU16(&tmp, pSrc);
  pSrc+=2;
  pDst+=tmp*surface->pitch;
  ReadU16(&Lines, pSrc);
  pSrc+=2;
  while(Lines--) {
    pTmpDst=pDst;
    PacketsCount=*(pSrc++);
    while(PacketsCount--) {
      CountSkip=*(pSrc++);
      pTmpDst+=CountSkip;
      CountData=*(pSrc++);
      if(CountData>0) {
        while(CountData--) {
          *(pTmpDst++)=*(pSrc++);
        }
      } else { 
        if(CountData<0) {
          CountData=(0x100-CountData);
          Fill=*(pSrc++);
          while(CountData--) {
            *(pTmpDst++)=Fill;
          }
        }
      }
    }
    pDst+=surface->pitch;
  }
}

void DECODE_COLOR(FLC*flc)
{ Uint8 *pSrc;
  Uint16 NumColors, NumColorPackets;
  Uint8 NumColorsSkip;
  int i;
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);

  pSrc=flc->pChunk+6;
  ReadU16(&NumColorPackets, pSrc);
  pSrc+=2;
  while(NumColorPackets--) {
    NumColorsSkip=*(pSrc++);
    if(!(NumColors=*(pSrc++))) {
      NumColors=256;
    }
    i=0;
    while(NumColors--) {
      flc->colors[i].r=*(pSrc++)<<2;
      flc->colors[i].g=*(pSrc++)<<2;
      flc->colors[i].b=*(pSrc++)<<2;
      i++;
    }
    SDL_SetColors(surface, flc->colors, NumColorsSkip, i);
  }
}


void DECODE_COPY(FLC*flc)
{
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);

	Uint8 *pSrc, *pDst;
	int Lines = flc->screen_h;
	pSrc=flc->pChunk+6;
	pDst=surface->pixels;
	while(Lines-- > 0) {
		memcpy(pDst, pSrc, flc->screen_w);
		pSrc+=flc->screen_w;
		pDst+=surface->pitch;
	}
}

void BLACK(FLC*flc)
{
	Uint8 *pDst;
	int Lines = flc->screen_h;
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);
	pDst=surface->pixels;
	while(Lines-- > 0) {
		memset(pDst, 0, flc->screen_w);
		pDst+=surface->pitch;
	}
}


void FlcDoOneFrame(FLC*flc)
{
	int ChunkCount; 
	SDL_Surface*surface=retrieveSurfacePointer(flc->surface);
	ChunkCount=flc->FrameChunks;
	flc->pChunk=flc->pMembuf;
	if ( SDL_LockSurface(surface) < 0 )
		return;
	while(ChunkCount--) {
		ReadU32(&flc->ChunkSize, flc->pChunk+0);
		ReadU16(&flc->ChunkType, flc->pChunk+4);

		switch(flc->ChunkType) {
			case 4:
				COLORS256(flc);
			break;
			case 7:
				SS2(flc);
			break;
			case 11:
				DECODE_COLOR(flc);
			break;
			case 12:
				DECODE_LC(flc);
			break;
			case 13:
				BLACK(flc);
			break;
			case 15:
				DECODE_BRUN(flc);
			break;
			case 16:
				DECODE_COPY(flc);
			break;
			case 18:
				#ifdef DEBUG
				printf("Chunk 18 not yet done.\n");
				#endif
			break;
			default:
			SDL_RAISE_S("Ieek an non implemented chunk type!");
		}
		flc->pChunk+=flc->ChunkSize;
	}
	SDL_UnlockSurface(surface);
}
/*
void SDLWaitFrame(FLC*flc)
{ static Uint32 oldTick=0;
  Uint32 currentTick;
  Sint32 waitTicks;

  currentTick=SDL_GetTicks(); 
  waitTicks=(oldTick+=(flc->HeaderSpeed))-currentTick;

  if(waitTicks>0) {
    SDL_Delay(waitTicks);
  }
}
*/
void FlcInitFirstFrame(FLC*flc)
{ 
	flc->FrameSize=16;
	flc->FrameCount=0;
	if(fseek(flc->file, 128, SEEK_SET)) {
		printf("Fseek read failed\n");
		exit (1);
	}
	FlcReadFile(flc, flc->FrameSize);
}

void FLCMark(FLC*flc)
{
	rb_gc_mark(flc->surface);
}

void FlcInit(FLC*flc, char *filename)
{ 
	VALUE args[3];

	flc->pMembuf=NULL;
	flc->membufSize=0;

	RUDL_VERIFY(FlcCheckHeader(flc, filename), "Wrong header");

	args[0]=rb_ary_new3(2, INT2NUM(flc->HeaderWidth), INT2NUM(flc->HeaderHeight));
	args[1]=UINT2NUM(SDL_SWSURFACE);
	args[2]=INT2NUM(flc->HeaderDepth);
	flc->surface=surface_new(3, args, classSurface);
	flc->lastFrameTicks=SDL_GetTicks();
}

void FlcDeInit(FLC*flc)
{ 
	flc->surface=Qnil;
	fclose(flc->file);
	free(flc->pMembuf);
}

void FlcMain(FLC*flc)
{
/*	int quit=0;
	SDL_Event event;
	FlcInitFirstFrame(flc);
	while(!quit) {
		flc->FrameCount++;
		if(FlcCheckFrame(flc)) {
			if (flc->FrameCount<=flc->HeaderFrames) {
				printf("Frame failure -- corrupt file?\n");
				exit(1);
			} else {
				if(flc->loop)
					FlcInitFirstFrame(flc);
				else {
					SDL_Delay(1000);
					quit=1;
				}
				continue;
			}
		}

		FlcReadFile(flc, flc->FrameSize);

		if(flc->FrameCheck!=0x0f100) {
			FlcDoOneFrame(flc);
			SDLWaitFrame(flc);
			// TODO: Track which rectangles have really changed
			SDL_UpdateRect(flc->surface, 0, 0, 0, 0);
		}

		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					quit=1;
					break;
				case SDL_QUIT:
					quit=1;
				default:
					break;
			}
		}
	}*/
}

////////////////// INTERFACE

FLC* retrieveFLCPointer(VALUE self)
{
	FLC* flc;
	Data_Get_Struct(self, FLC, flc);
	SDL_ASSERT(flc);
	return flc;
}

static VALUE flc_decoder_reset(VALUE self)
{
	FLC* flc=retrieveFLCPointer(self);
	FlcInitFirstFrame(flc);
	return self;
}

static VALUE createFLCObject(char* filename)
{
	VALUE newObject;
	FLC* flc=(FLC*)malloc(sizeof(FLC));

	initVideo();

	SDL_ASSERT(flc);

	FlcInit(flc, filename);

	newObject=Data_Wrap_Struct(classFLCDecoder, FLCMark, FlcDeInit, flc);
	flc_decoder_reset(newObject);
	return newObject;
}

static VALUE flc_decoder_new(VALUE clas, VALUE filename)
{
	return createFLCObject(STR2CSTR(filename));
}

static VALUE flc_decoder_destroy(VALUE self)
{
	FlcDeInit(retrieveFLCPointer(self));
	DATA_PTR(self)=NULL;
	return Qnil;
}

static VALUE flc_decoder_surface(VALUE self)
{
	return retrieveFLCPointer(self)->surface;
}

static VALUE flc_decoder_next(VALUE self)
{
	FLC* flc=retrieveFLCPointer(self);
	flc->FrameCount++;
	if(FlcCheckFrame(flc)){
		flc_decoder_reset(self);
	}else{
		FlcReadFile(flc, flc->FrameSize);

		if(flc->FrameCheck!=0x0f100) {
			FlcDoOneFrame(flc);
		}
	}
	return self;
}

static VALUE flc_decoder_delay(VALUE self)
{
	FLC* flc=retrieveFLCPointer(self);
	Uint32 now=SDL_GetTicks();
	if(flc->HeaderSpeed>now-flc->lastFrameTicks){
		SDL_Delay(flc->HeaderSpeed-(now-flc->lastFrameTicks));
	}
	DEBUG_I(flc->HeaderSpeed-(now-flc->lastFrameTicks));
	flc->lastFrameTicks=now;
	return self;
}

static VALUE flc_decoder_frame(VALUE self)
{
	FLC* flc=retrieveFLCPointer(self);
	return INT2NUM(flc->FrameCount);
}

void initFLXClasses()
{
	classFLCDecoder=rb_define_class_under(moduleRUDL, "FLCDecoder", rb_cObject);
	rb_define_singleton_method(classFLCDecoder, "new", flc_decoder_new, 1);
	rb_define_singleton_method(classFLCDecoder, "destroy", flc_decoder_destroy, 0);
	rb_define_method(classFLCDecoder, "surface", flc_decoder_surface, 0);
	rb_define_method(classFLCDecoder, "next", flc_decoder_next, 0);
	rb_define_method(classFLCDecoder, "delay", flc_decoder_delay, 0);
	rb_define_method(classFLCDecoder, "reset", flc_decoder_reset, 0);
	rb_define_method(classFLCDecoder, "frame", flc_decoder_frame, 0);
}
