/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#ifdef HAVE_SMPEG_SMPEG_H

#ifdef IN_DEVELOPMENT

#include "rudl_movie.h"
#include "rudl_video.h"
#include "rudl_audio.h"
#include "smpeg/smpeg.h"

/*
=begin
<<< docs/head
= Movie
The Movie object represents an opened MPEG file.
You control playback similar to a Sound object.

Movie objects have a target display Surface.
The movie is rendered to this Surface in a background thread.
If the Surface is the display surface,
and the system supports it,
the movie will render into a Hardware YUV overlay plane.
If you don't set a display Surface,
it will default to the display Surface.

Movies are played back in background threads,
so there is very little management needed on the user end.
Just load the Movie, set the destination, and play()


== Class Methods
--- Movie.new( filename )
Loads an MPEG stream from file ((<filename>))
=end */

static void init_smpeg()
{
	DEBUG_S("Starting SMPEG");
}

typedef struct {
	SMPEG* movie;
	SMPEG_Info info;
} BIGSMPEG;

#define GETMOVIE BIGSMPEG* movie; Data_Get_Struct(self, BIGSMPEG, movie);

void delete_BIGSMPEG(BIGSMPEG* me)
{
	SMPEG_delete(me->movie);
	free(me);
}

static void movie_check_error(BIGSMPEG* smpeg)
{
	char*error=SMPEG_error(smpeg->movie);
	if(error){
		SDL_RAISE_S(error);
	}
}

static void movie_renew_info(BIGSMPEG* smpeg)
{
	SMPEG_getinfo(smpeg->movie, &(smpeg->info));
}

static VALUE movie_new(VALUE self, VALUE filename)
{
	//init_smpeg();
	BIGSMPEG* smpeg=(BIGSMPEG*)malloc(sizeof(BIGSMPEG));
	smpeg->movie=SMPEG_new(STR2CSTR(filename), &(smpeg->info), true);
	movie_check_error(smpeg);
	return Data_Wrap_Struct(classMovie, 0, delete_BIGSMPEG, smpeg);
}

static VALUE movie_play(VALUE self)
{
	GETMOVIE;
	SMPEG_play(movie->movie);
	return self;
}

	classMovie=rb_define_module_under(moduleRUDL, "Movie");
	rb_define_singleton_method(classMovie, "new", movie_new, 1);
	rb_define_method(classMovie, "play", movie_play, 0);
}
#include "smpeg/smpeg.h"

static SMPEG_Filter* filters[3];
#define NULL_FILTER 0
#define BILINEAR_FILTER 1
#define DEBLOCKING_FILTER 2
#define NUM_FILTERS 3

static void setInfoToSMPEGInfo(VALUE self,SMPEG_Info info)
{
  rb_iv_set(self,"@has_audio",BOOL(info.has_audio));
  rb_iv_set(self,"@has_video",BOOL(info.has_video));
  rb_iv_set(self,"@width",INT2NUM(info.width));
  rb_iv_set(self,"@height",INT2NUM(info.height));
  rb_iv_set(self,"@current_frame",INT2NUM(info.current_frame));
  rb_iv_set(self,"@current_fps",INT2NUM(info.current_fps));
  rb_iv_set(self,"@audio_string",rb_str_new2(info.audio_string));
  rb_iv_set(self,"@audio_current_frame",INT2NUM(info.audio_current_frame));
  rb_iv_set(self,"@current_offset",UINT2NUM(info.current_offset));
  rb_iv_set(self,"@total_size",UINT2NUM(info.total_size));
  rb_iv_set(self,"@current_time",UINT2NUM(info.current_time));
  rb_iv_set(self,"@total_time",UINT2NUM(info.total_time));
}

static VALUE smpeg_load(VALUE self,VALUE filename)
{
  SMPEG *mpeg;
  VALUE infoObj;
  char error_msg[2048];
    
  mpeg = SMPEG_new(STR2CSTR(filename),NULL,SDL_WasInit(SDL_INIT_AUDIO));
  if( SMPEG_error(mpeg) ){
    snprintf(error_msg,sizeof(error_msg),"Couldn't load %s: %s",
	     STR2CSTR(filename),SMPEG_error(mpeg));
    SMPEG_delete(mpeg);
    rb_raise(eSDLError,"%s",error_msg);
  }
  
  return Data_Wrap_Struct(classMovie,0,SMPEG_delete,mpeg);
}

static VALUE smpeg_getInfo(VALUE self,VALUE infoObj)
{
  SMPEG *mpeg;
  SMPEG_Info info;
  
  if( !rb_obj_is_kind_of(infoObj,classMovie) )
    rb_raise(rb_eArgError,"type mismatch(expect SDL::MPEG::Info)");
  Data_Get_Struct(self,SMPEG,mpeg);

  SMPEG_getinfo(mpeg,&info);
  setInfoToSMPEGInfo(infoObj,info);
  
  return self;
}

static VALUE smpeg_enableAudio(VALUE self,VALUE enable)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_enableaudio(mpeg,RTEST(enable));
  return self;
}

static VALUE smpeg_enableVideo(VALUE self,VALUE enable)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_enablevideo(mpeg,RTEST(enable));
  return self;
}

static VALUE smpeg_status(VALUE self)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  return INT2FIX(SMPEG_status(mpeg));
}

static VALUE smpeg_setVolume(VALUE self,VALUE volume)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_setvolume(mpeg,NUM2INT(volume));
}

static VALUE smpeg_setDisplay(VALUE self,VALUE dst)
{
  SMPEG *mpeg;
  SDL_Surface *surface;
  if( !rb_obj_is_kind_of(dst,cSurface) )
    rb_raise(rb_eArgError,"type mismatchi(expect Surface)");
  
  Data_Get_Struct(self,SMPEG,mpeg);
  Data_Get_Struct(dst,SDL_Surface,surface);

  SMPEG_setdisplay(mpeg,surface,NULL,NULL);
  return self;
}

static VALUE smpeg_setLoop(VALUE self,VALUE repeat)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_loop(mpeg,RTEST(repeat));
  return self;
}

static VALUE smpeg_scaleXY(VALUE self,VALUE w,VALUE h)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_scaleXY(mpeg,NUM2INT(w),NUM2INT(h));
  return self;
}

static VALUE smpeg_scale(VALUE self,VALUE scale)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_scale(mpeg,NUM2INT(scale));
  return self;
}

static VALUE smpeg_move(VALUE self,VALUE x,VALUE y)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_move(mpeg,NUM2INT(x),NUM2INT(y));
  return self;
}

static VALUE smpeg_setDisplayRegion(VALUE self,VALUE x,VALUE y,VALUE w, VALUE h)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_setdisplayregion(mpeg,NUM2INT(x),NUM2INT(y),NUM2INT(w),NUM2INT(h));
  return self;
}

static VALUE smpeg_play(VALUE self)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_play(mpeg);
  return self;
}

static VALUE smpeg_pause(VALUE self)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_pause(mpeg);
  return self;
}

static VALUE smpeg_stop(VALUE self)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_stop(mpeg);
  return self;
}

static VALUE smpeg_rewind(VALUE self)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_rewind(mpeg);
  return self;
}

static VALUE smpeg_seek(VALUE self,VALUE bytes)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_seek(mpeg,NUM2INT(bytes));
  return self;
}

static VALUE smpeg_skip(VALUE self,VALUE seconds)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_skip(mpeg,NUM2DBL(seconds));
  return self;
}

static VALUE smpeg_renderFrame(VALUE self,VALUE framenum)
{
  SMPEG *mpeg;
  Data_Get_Struct(self,SMPEG,mpeg);
  SMPEG_renderFrame(mpeg,NUM2INT(framenum));
  return self;
}

static VALUE smpeg_setFilter(VALUE self,VALUE filter)
{
	SMPEG *mpeg;

	Data_Get_Struct(self,SMPEG,mpeg);
	
	RUDL_ASSERT(NUM2INT(filter)>=0, "No such filter");
	RUDL_ASSERT(NUM2INT(filter)<NUM_FILTERS, "No such filter");

	SMPEG_filter(mpeg,filters[NUM2INT(filter)]);
	return self;
}

void initMovieClasses()
{
	classMovie= rb_define_class_under(moduleRUDL,"Movie",rb_cObject);

	filters[NULL_FILTER] = SMPEGfilter_null();
	filters[BILINEAR_FILTER] = SMPEGfilter_bilinear();
	filters[DEBLOCKING_FILTER] = SMPEGfilter_deblocking();

	rb_define_attr(classMovie,"has_audio",1,0);
	rb_define_attr(classMovie,"has_video",1,0);
	rb_define_attr(classMovie,"width",1,0);
	rb_define_attr(classMovie,"height",1,0);
	rb_define_attr(classMovie,"current_frame",1,0);
	rb_define_attr(classMovie,"current_fps",1,0);
	rb_define_attr(classMovie,"audio_string",1,0);
	rb_define_attr(classMovie,"audio_current_frame",1,0);
	rb_define_attr(classMovie,"current_offset",1,0);
	rb_define_attr(classMovie,"total_size",1,0);
	rb_define_attr(classMovie,"current_time",1,0);
	rb_define_attr(classMovie,"total_time",1,0);

	rb_define_singleton_method(classMovie,"load",smpeg_load,1);
	rb_define_singleton_method(classMovie,"new",smpeg_load,1);

	rb_define_method(classMovie,"info",smpeg_getInfo,1);
	rb_define_method(classMovie,"enableAudio",smpeg_enableAudio,1);
	rb_define_method(classMovie,"enableVideo",smpeg_enableVideo,1);
	rb_define_method(classMovie,"status",smpeg_status,0);
	rb_define_method(classMovie,"setVolume",smpeg_setVolume,1);
	rb_define_method(classMovie,"setDisplay",smpeg_setDisplay,1);
	rb_define_method(classMovie,"setLoop",smpeg_setLoop,1);
	rb_define_method(classMovie,"scaleXY",smpeg_scaleXY,2);
	rb_define_method(classMovie,"scale",smpeg_scale,1);
	rb_define_method(classMovie,"move",smpeg_move,1);
	rb_define_method(classMovie,"setDisplayRegion",smpeg_setDisplayRegion,4);
	rb_define_method(classMovie,"play",smpeg_play,0);
	rb_define_method(classMovie,"pause",smpeg_pause,0);
	rb_define_method(classMovie,"stop",smpeg_stop,0);
	rb_define_method(classMovie,"rewind",smpeg_rewind,0);
	rb_define_method(classMovie,"seek",smpeg_seek,1);
	rb_define_method(classMovie,"skip",smpeg_skip,1);
	rb_define_method(classMovie,"renderFrame",smpeg_renderFrame,1);
	rb_define_method(classMovie,"setFilter",smpeg_setFilter,1);

	rb_define_const(classMovie,"ERROR",INT2FIX(SMPEG_ERROR));
	rb_define_const(classMovie,"STOPPED",INT2FIX(SMPEG_STOPPED));
	rb_define_const(classMovie,"PLAYING",INT2FIX(SMPEG_PLAYING));
	rb_define_const(classMovie,"NULL_FILTER",INT2FIX(NULL_FILTER));
	rb_define_const(classMovie,"BILINEAR_FILTER",INT2FIX(BILINEAR_FILTER));
	rb_define_const(classMovie,"DEBLOCKING_FILTER",INT2FIX(DEBLOCKING_FILTER));
  
}
#endif

#endif
