/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_cdrom.h"

VALUE classCD;
VALUE classCDROM;

//////////////////
void initCD()
{
	if(!SDL_WasInit(SDL_INIT_CDROM)){
		DEBUG_S("Starting CDROM subsystem");
		SDL_InitSubSystem(SDL_INIT_CDROM);
	}
}

void quitCD()
{
	if(SDL_WasInit(SDL_INIT_CDROM)){
		DEBUG_S("Stopping CDROM subsystem");
		SDL_QuitSubSystem(SDL_INIT_CDROM);
	}
}

//////////////////
static VALUE createCDROMObject(int number)
{
	VALUE newObject;
	SDL_CD* cd=SDL_CDOpen(number);

	SDL_ASSERT(cd);

	newObject=Data_Wrap_Struct(classCDROM, 0, SDL_CDClose, cd);
	rb_iv_set(newObject, "@id", INT2NUM(number));
	return newObject;
}

SDL_CD* retrieveCDROMPointer(VALUE self)
{
	SDL_CD* cd;
	Data_Get_Struct(self, SDL_CD, cd);
	SDL_ASSERT(cd);
	return cd;
}

/*
=begin
<<< docs/head
= CDROM
A class for playing audio CD's.
== Class Methods
--- CDROM.new( number )
Creates a new CDROM access object for unit ((|number|)).
There can only be one CDROM object per unit.
Unit 0 is the default unit.
=end */
static VALUE cdrom_new(VALUE self, VALUE number)
{
	initCD();
	return createCDROMObject(NUM2INT(number));
}

/*
=begin
--- CDROM.destroy
Uninitializes the CDROM subsystem,
which is normally not necessary.
=end */
static VALUE cdrom_destroy(VALUE self, VALUE number)
{
	quitCD();
	return self;
}

/*
=begin
--- CDROM.count
Returns the number of CDROMs installed.
=end */
static VALUE cdrom_count(VALUE self)
{
	initCD();
	return INT2NUM(SDL_CDNumDrives());
}

/*
=begin
--- CDROM#eject
--- CDROM#pause
--- CDROM#play( track_nr )
--- CDROM#resume
--- CDROM#stop
--- CDROM#paused?
--- CDROM#empty?
--- CDROM#busy?
These are the you-know-what-they-do methods.
=end */
static VALUE cdrom_eject(VALUE self)
{
	SDL_VERIFY(SDL_CDEject(retrieveCDROMPointer(self))!=-1);
	return self;
}

static VALUE cdrom_pause(VALUE self)
{
	SDL_VERIFY(SDL_CDPause(retrieveCDROMPointer(self))!=-1);
	return self;
}

static VALUE cdrom_play(VALUE self, VALUE trackValue)
{
	SDL_CD* cdrom=retrieveCDROMPointer(self);
	int track=NUM2INT(trackValue);
	int offset, length;
	
	SDL_CDStatus(cdrom); // ?

	RUDL_VERIFY(track >= 0 && track < cdrom->numtracks, "Invalid track number");
	RUDL_VERIFY(cdrom->track[track].type == SDL_AUDIO_TRACK, "CD track type is not audio");

	offset = cdrom->track[track].offset;
	length = cdrom->track[track].length;

	SDL_VERIFY(SDL_CDPlay(cdrom, offset, length)!=-1);
	return self;
}

static VALUE cdrom_resume(VALUE self)
{
	SDL_VERIFY(SDL_CDResume(retrieveCDROMPointer(self))!=-1);
	return self;
}

static VALUE cdrom_stop(VALUE self)
{
	SDL_VERIFY(SDL_CDStop(retrieveCDROMPointer(self))!=-1);
	return self;
}

static VALUE cdrom_paused_(VALUE self)
{
	return INT2BOOL(SDL_CDStatus(retrieveCDROMPointer(self))==CD_PAUSED);
}

static VALUE cdrom_empty_(VALUE self)
{
	return INT2BOOL(SDL_CDStatus(retrieveCDROMPointer(self))==CD_TRAYEMPTY);
}

static VALUE cdrom_busy_(VALUE self)
{
	return INT2BOOL(SDL_CDStatus(retrieveCDROMPointer(self))==CD_PLAYING);
}

/*
=begin
--- CDROM#current
=end */
static VALUE cdrom_current(VALUE self)
{
	SDL_CD* cdrom=retrieveCDROMPointer(self);
	int track;
	double seconds;

	SDL_CDStatus(cdrom);
	track = cdrom->cur_track;
	seconds = cdrom->cur_frame / (float)CD_FPS;

	return rb_ary_new3(2, INT2NUM(track), DBL2NUM(seconds));
}

/*
=begin
--- CDROM#number
Returns the unit number that was specified in ((<CDROM.new>)).
=end */
static VALUE cdrom_number(VALUE self)
{
	return rb_iv_get(self, "@id");
}

/*
=begin
--- CDROM#name
Returns a string describing the CDROM.
=end */
static VALUE cdrom_name(VALUE self)
{
	VALUE tmp=rb_iv_get(self, "@id");
	return rb_str_new2(SDL_CDName(NUM2INT(tmp)));
}

/*
=begin
--- CDROM#num_tracks
Returns the number of tracks on the CD.
=end */
static VALUE cdrom_num_tracks(VALUE self)
{
	SDL_CD* cdrom=retrieveCDROMPointer(self);
	SDL_CDStatus(cdrom);
	return INT2NUM(cdrom->numtracks);
}

/*
=begin
--- CDROM#audiotrack?( track_nr )
Returns whether the specified track is an audiotrack.
=end */
static VALUE cdrom_audiotrack_(VALUE self, VALUE trackValue)
{
	SDL_CD* cdrom=retrieveCDROMPointer(self);
	int track=NUM2INT(trackValue);

	SDL_CDStatus(cdrom);
	
	RUDL_VERIFY(track >= 0 && track < cdrom->numtracks, "Invalid track number");

	return INT2BOOL(cdrom->track[track].type == SDL_AUDIO_TRACK);
}

/*
=begin
--- CDROM#track_length( track_nr )
Returns the length of the track.
Returns 0.0 when the track is not an audio track.
=end */
static VALUE cdrom_track_length(VALUE self, VALUE trackValue)
{
	SDL_CD* cdrom=retrieveCDROMPointer(self);
	int track=NUM2INT(trackValue);

	SDL_CDStatus(cdrom);
	
	RUDL_VERIFY(track >= 0 && track < cdrom->numtracks, "Invalid track number");

	if(cdrom->track[track].type != SDL_AUDIO_TRACK){
		return DBL2NUM(0.0);
	}

	return DBL2NUM(cdrom->track[track].length / (double)CD_FPS);
}

/*
=begin
--- CDROM#track_start( track_nr )
Returns the starting time of the track.
=end */
static VALUE cdrom_track_start(VALUE self, VALUE trackValue)
{
	SDL_CD* cdrom=retrieveCDROMPointer(self);
	int track=NUM2INT(trackValue);

	SDL_CDStatus(cdrom);

	RUDL_VERIFY(track >= 0 && track < cdrom->numtracks, "Invalid track number");

	return DBL2NUM(cdrom->track[track].offset / (double)CD_FPS);
}

//////////////////
void initCDClasses()
{
	classCDROM=rb_define_class_under(moduleRUDL, "CDROM", rb_cObject);
	rb_define_singleton_method(classCDROM, "new", cdrom_new, 1);
	rb_define_singleton_method(classCDROM, "destroy", cdrom_destroy, 0);
	rb_define_singleton_method(classCDROM, "count", cdrom_count, 0);
	rb_define_method(classCDROM, "eject", cdrom_eject, 0);
	rb_define_method(classCDROM, "busy?", cdrom_busy_, 0);
	rb_define_method(classCDROM, "current", cdrom_current, 0);
	rb_define_method(classCDROM, "empty?", cdrom_empty_, 0);
	rb_define_method(classCDROM, "number", cdrom_number, 0);
	rb_define_method(classCDROM, "name", cdrom_name, 0);
	rb_define_method(classCDROM, "num_tracks", cdrom_num_tracks, 0);
	rb_define_method(classCDROM, "audiotrack?", cdrom_audiotrack_, 1);
	rb_define_method(classCDROM, "track_length", cdrom_track_length, 1);
	rb_define_method(classCDROM, "track_start", cdrom_track_start, 1);
	rb_define_method(classCDROM, "pause", cdrom_pause, 0);
	rb_define_method(classCDROM, "paused?", cdrom_paused_, 0);
	rb_define_method(classCDROM, "play", cdrom_play, 1);
	rb_define_method(classCDROM, "resume", cdrom_resume, 0);
	rb_define_method(classCDROM, "stop", cdrom_stop, 0);
}
