/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
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

void initMovieClasses()
{
	classMovie=rb_define_module_under(moduleRUDL, "Movie");
	rb_define_singleton_method(classMovie, "new", movie_new, 1);
	rb_define_method(classMovie, "play", movie_play, 0);
}
