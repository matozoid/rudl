/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_audio.h"

VALUE classSound;
VALUE classChannel;
VALUE classMixer;
VALUE classMusic;
VALUE classEndOfMusicEvent;

#ifdef HAVE_SDL_MIXER_H
#include "SDL_mixer.h"

#ifndef MIX_MAJOR_VERSION
Your SDL_mixer lib is too old
#endif

// Thanks to Ruby-SDL
#define MAX_CHANNELS 256 /* should be more than enough */
static VALUE playing_wave[MAX_CHANNELS];
static VALUE playing_music=Qnil;

void clearGCHack()
{
	int i;
	for(i=0; i<MAX_CHANNELS; i++){
		playing_wave[i]=Qnil;
		rb_global_variable(&(playing_wave[i]));
	}
	rb_global_variable( &playing_music );
}

static VALUE mixer_get_format(VALUE self);

//////////////////
Mix_Chunk* retrieveMixChunk(VALUE self);

void initAudio()
{
	if(!SDL_WasInit(SDL_INIT_AUDIO)){
		DEBUG_S("Starting audio subsystem");

		initSDL();

		if(!SDL_WasInit(SDL_INIT_AUDIO)){
			SDL_VERIFY(!SDL_InitSubSystem(SDL_INIT_AUDIO));
		}

		if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1){
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
			SDL_RAISE;
		}
	}
}

void quitAudio()
{
	// It's a HACK! For some reason some or all Musics can't be freed after
	// the mixer was closed. This will give some ugly bogus Music objects
	// though, but hey, it wasn't me who closed and opened the mixer and reused
	// some Music objects!
	rb_eval_string("ObjectSpace.each_object(RUDL::Music) {|x| x.destroy}");
	if(SDL_WasInit(SDL_INIT_AUDIO)){
		Mix_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);

		DEBUG_S("Stopping audio subsystem");
	}
}

/////////////// CHANNEL

__inline__ VALUE createChannelObject(int number)
{
	return rb_funcall(classChannel, id_new, 1, INT2NUM(number));
}

__inline__ int retrieveChannelNumber(VALUE channel)
{
	VALUE tmp;

	tmp=rb_iv_get(channel, "@number");
	return NUM2INT(tmp);
}
/*
=begin
<<< docs/head

This page describes the various classes that access SDL_mixer.

= Channel
A Channel is an interface object to one of the mixer's channels.
It can be used for manipulating sound on a particular channel.
== Class Methods
--- Channel.new( number )
Creates a Channel interface object for mixer channel ((|number|)).
=end */


// Replaced by some evalled stuff

/*
=begin
== Instance Methods
--- Channel#fade_out( milliseconds )
--- Channel#volume
--- Channel#volume=( loudness )
These are volume methods.
fade_out fades the channel to silence in ((|milliseconds|)) milliseconds.
volume returns the current volume.
volume= sets the volume to loudness.
Volumes range from 0.0 to 1.0.
=end */
static VALUE channel_fade_out(VALUE self, VALUE milliseconds)
{
	Mix_FadeOutChannel(retrieveChannelNumber(self), NUM2UINT(milliseconds));
	return self;
}

static VALUE channel_get_volume(VALUE self, VALUE volume)
{
	return DBL2NUM((Mix_Volume(retrieveChannelNumber(self), -1))/128.0);
}

static VALUE channel_set_volume(VALUE self, VALUE volume)
{
	Mix_Volume(retrieveChannelNumber(self), (int)(NUM2DBL(volume)*128));
	return self;
}

/*
=begin
--- Channel#busy
--- Channel#pause
--- Channel#unpause
--- Channel#play( sound, loops, maxtime )

--- Channel#play( sound, loops )

--- Channel#play( sound )

--- Channel#stop
These are pretty self-explanatory.
play plays (({Sound})) sound 1+loops times, for a maximum time of maxtime milliseconds.
=end */
static VALUE channel_busy(VALUE self)
{
	return Mix_Playing(retrieveChannelNumber(self));
}

static VALUE channel_pause(VALUE self)
{
	Mix_Pause(retrieveChannelNumber(self));
	return self;
}

static VALUE channel_play(int argc, VALUE* argv, VALUE self)
{
	int loops = 0, maxtime = -1, channelnum;
	VALUE sound=Qnil, loopsValue, maxtimeValue;
	Mix_Chunk* chunk=retrieveMixChunk(argv[0]);

	switch(rb_scan_args(argc, argv, "12", &sound, &loopsValue, &maxtimeValue)){
		case 3: maxtime=NUM2INT(maxtimeValue);
		case 2: loops=NUM2INT(loopsValue);
	}

	channelnum = Mix_PlayChannelTimed(retrieveChannelNumber(self), chunk, loops, maxtime);
	
	if(channelnum != -1){
		Mix_GroupChannel(channelnum, (int)chunk);
	}
	
	return self;
}

static VALUE channel_stop(VALUE self)
{
	Mix_HaltChannel(retrieveChannelNumber(self));
	return self;
}

static VALUE channel_unpause(VALUE self)
{
	Mix_Resume(retrieveChannelNumber(self));
	return self;
}
/*
=begin
--- Channel#set_panning( left, right )
Set the panning of a channel. The left and right channels are specified
as numbers between 0.0 and 1.0, quietest to loudest, respectively.
Technically, this is just individual volume control for a sample with
two (stereo) channels, so it can be used for more than just panning.
If you want real panning, call it like this:

  channel.set_panning(left, 1.0-left);

...which isn't so hard.

Returns self.
=end */
static VALUE channel_set_panning(VALUE self, VALUE left, VALUE right)
{
	double l=NUM2DBL(left);
	double r=NUM2DBL(right);
	RUDL_ASSERT(l<=1.0, "left volume too high");
	RUDL_ASSERT(l>=0.0, "left volume too low");
	RUDL_ASSERT(r<=1.0, "right volume too high");
	RUDL_ASSERT(r>=0.0, "right volume too low");
	SDL_VERIFY(Mix_SetPanning(retrieveChannelNumber(self), (Uint8)(l*255), (Uint8)(r*255)));
	return self;
}

/*
=begin
--- Channel#set_position( angle, distance )
Set the position of a channel. ((|angle|)) is an number from 0 to 360, that
specifies the location of the sound in relation to the listener. ((|angle|))
will be reduced as neccesary (540 becomes 180 degrees, -100 becomes 260).
Angle 0 is due north, and rotates clockwise as the value increases.
For efficiency, the precision of this effect may be limited (angles 1
through 7 might all produce the same effect, 8 through 15 are equal, etc).

((|distance|)) is a number between 0.0 and 1.0 that specifies the space
between the sound and the listener. The larger the number, the further
away the sound is. Using 1.0 does not guarantee that the channel will be
culled from the mixing process or be completely silent. For efficiency,
the precision of this effect may be limited (distance 0 through 0.1 might
all produce the same effect, 0.1 through 0.2 are equal, etc). Setting ((|angle|))
and ((|distance|)) to 0 unregisters this effect, since the data would be
unchanged.
 
Returns self.
=end */
static VALUE channel_set_position(VALUE self, VALUE angle, VALUE distance)
{
	SDL_VERIFY(Mix_SetPosition(retrieveChannelNumber(self), (Sint16)NUM2INT(angle), (Sint8)(NUM2DBL(distance)*255)));
	return self;
}

/*
=begin
--- Channel#set_distance( distance )
((|distance|)) is a number between 0.0 and 1.0 that specifies the space
between the sound and the listener. The larger the number, the further
away the sound is. Using 1.0 does not guarantee that the channel will be
culled from the mixing process or be completely silent. For efficiency,
the precision of this effect may be limited (distance 0 through 0.1 might
all produce the same effect, 0.1 through 0.2 are equal, etc). Setting 
((|distance|)) to 0 unregisters this effect, since the data would be
unchanged.

Returns self.
=end */
static VALUE channel_set_distance(VALUE self, VALUE distance)
{
	SDL_VERIFY(Mix_SetDistance(retrieveChannelNumber(self), (Uint8)(NUM2DBL(distance)*255)));
	return self;
}

/*
=begin
--- Channel#reverse_stereo( reverse )
Causes a channel to reverse its stereo. This is handy if the user has his
speakers hooked up backwards, or you would like to have a minor bit of
psychedelia in your sound code.  :)  Calling this function with ((|reverse|))
set to true reverses the chunks's usual channels. If ((|reverse|)) is false,
the effect is unregistered.

Returns self.
=end */
static VALUE channel_reverse_stereo(VALUE self, VALUE reverse)
{
	SDL_VERIFY(Mix_SetReverseStereo(retrieveChannelNumber(self), NUM2BOOL(reverse)));
	return self;
}

/////////////// SOUND
Mix_Chunk* retrieveMixChunk(VALUE self)
{
	Mix_Chunk* chunk;
	Data_Get_Struct(self, Mix_Chunk, chunk);

	RUDL_ASSERT(chunk, "Retrieved NULL chunk");
	return chunk;
}

/*
=begin
= Sound
Sound is a single sample.
It is loaded from a WAV file.
== Class Methods
--- Sound.new
--- Sound.new( filename )
--- Sound.load_new( filename )
Creates a new (({Sound})) object with the sound in file ((|filename|)).
=end */
static VALUE sound_new(VALUE self, VALUE filename)
{
	Mix_Chunk* chunk;
	VALUE newObject;
	
	initAudio();

	chunk=Mix_LoadWAV(STR2CSTR(filename));
	SDL_VERIFY(chunk);

	newObject=Data_Wrap_Struct(classSound, 0, SDL_FreeWAV, chunk);

	RUDL_VERIFY(newObject, "Sound.new misbehaved");


	rb_obj_call_init(newObject, 0, NULL);
	return newObject;
}


/*
=begin
--- Sound.import( sampledata )
This method imports raw sampledata.
If it is not in the Mixer's format, use (({Mixer.convert})) first.
=end */

static VALUE sound_import(VALUE self, VALUE sampledata)
{
	Mix_Chunk* chunk=(Mix_Chunk *)malloc(sizeof(Mix_Chunk));
	VALUE newObject;

	initAudio();

	Check_Type(sampledata, T_STRING);

	chunk->allocated = 0;
	chunk->alen = RSTRING(sampledata)->len;
	chunk->abuf = malloc(chunk->alen);
	chunk->volume = MIX_MAX_VOLUME;
	memcpy(chunk->abuf, RSTRING(sampledata)->ptr, chunk->alen);

	newObject=Data_Wrap_Struct(classSound, 0, SDL_FreeWAV, chunk);
	RUDL_VERIFY(newObject, "Sound.load_raw misbehaved");

	rb_obj_call_init(newObject, 0, NULL);
	return newObject;
}

/*
#=begin
#--- Sound.mix( destination, source )
#--- Sound.mix( destination, source, max_volume )
#=end */
/*

__inline__ char* sound_pad(VALUE input, int to_length)

{
	char* new_sound=(char*)calloc(to_length, sizeof(char));
	memcpy(new_sound, RSTRING(input)->ptr, RSTRING(input)->len);
	return new_sound;
}

static VALUE sound_mix(int argc, VALUE* argv, VALUE self)
{
	VALUE v_data1, v_data2, v_volume;
	double volume=1.0;
	char *data1, *data2;

	switch(rb_scan_args(argc, argv, "21", &v_data1, &v_data2, &v_volume)){
		case 3: volume=NUM2DBL(v_volume);
	}
	Check_Type(v_data1, T_STRING);
	Check_Type(v_data2, T_STRING);
	*data1=RSTRING(v_data1)->len;
	*data2=RSTRING(v_data2)->len;

	if(RSTRING(v_data1)->len > RSTRING(v_data2)->len){
		data2=sound_pad(v_data2, RSTRING(v_data1)->len);
	}

	if(RSTRING(v_data1)->len < RSTRING(v_data2)->len){
		data1=sound_pad(v_data1, RSTRING(v_data2)->len);
	}
Kopieer sdl_mixer.c

	SDL_MixAudio(data, Uint32 len, int volume);
}
*/

/*
=begin
--- String.to_sound
Creates a new (({Sound})) object with the sound (in .WAV format) in the string.
=end */
static VALUE string_to_sound(VALUE self)
{
	Mix_Chunk* chunk;
	VALUE newObject;
	SDL_RWops* rwops;
	
	initAudio();

	rwops=SDL_RWFromMem(RSTRING(self)->ptr, RSTRING(self)->len);
	chunk=Mix_LoadWAV_RW(rwops, 0);
	SDL_FreeRW(rwops);
	SDL_VERIFY(chunk);

	newObject=Data_Wrap_Struct(classSound, 0, SDL_FreeWAV, chunk);
	RUDL_VERIFY(newObject, "String.to_sound misbehaved");

	rb_obj_call_init(newObject, 0, NULL);
	return newObject;
}

/*
=begin
== Instance Methods
--- Sound#fade_out( milliseconds )
--- Sound#volume
--- Sound#volume=( loudness )
These are volume methods.
fade_out fades all instances of this sound that are playing to silence in ((|milliseconds|)) milliseconds.
volume returns the current volume.
volume= sets the volume to loudness.
Volumes range from 0.0 to 1.0.
=end */
static VALUE sound_fade_out(VALUE self, VALUE time)
{
	Mix_FadeOutGroup((int)retrieveMixChunk(self), NUM2UINT(time));
	return self;
}

static VALUE sound_get_volume(VALUE self)
{
	return DBL2NUM((Mix_VolumeChunk(retrieveMixChunk(self), -1))/128.0);
}

static VALUE sound_set_volume(VALUE self, VALUE volume)
{
	Mix_VolumeChunk(retrieveMixChunk(self), (int)(NUM2DBL(volume)*128));
	return self;
}

/*
=begin
--- Sound#play( loops, maxtime )
Starts playing a song on an available channel.
If no channels are available, it will not play and return ((|nil|)).
Loops controls how many extra times the sound will play, a negative loop will play
indefinitely, it defaults to 0.
Maxtime is the number of total milliseconds that the sound will play.
It defaults to forever (-1).


Returns a channel object for the channel that is selected to play the sound.

Returns nil if for some reason no channel is found.


--- Sound#stop
Stops all channels playing this (({Sound})).
=end */
static VALUE sound_play(int argc, VALUE* argv, VALUE self)
{
	VALUE loopsValue, timeValue;
	int loops=0;
	int time=-1;
	int channelnum=-1;
	Mix_Chunk* chunk=retrieveMixChunk(self);

	switch(rb_scan_args(argc, argv, "02", &loopsValue, &timeValue)){
		case 2: time=NUM2INT(timeValue);
		case 1: loops=NUM2INT(loopsValue);
	}

	channelnum = Mix_PlayChannelTimed(-1, chunk, loops, time);
	if(channelnum == -1) return Qnil;

	//make sure volume on this arbitrary channel is set to full
	Mix_Volume(channelnum, 128);

	Mix_GroupChannel(channelnum, (int)chunk);

	playing_wave[channelnum]=self; // to avoid gc problem
	
	return createChannelObject(channelnum);
}

static VALUE sound_stop(VALUE self)
{
	Mix_HaltGroup((int)retrieveMixChunk(self));
	return self;
}

static VALUE sound_get_num_channels(VALUE self)
{
	return UINT2NUM(Mix_GroupCount((int)retrieveMixChunk(self)));
}



/*
=begin
--- Sound#to_s
Returns a string with the sampledata.
=end */
static VALUE sound_export(VALUE self)
{
	Mix_Chunk* chunk=retrieveMixChunk(self);

	return rb_str_new(chunk->abuf, chunk->alen);
}

/*
=begin
--- Sound.convert( sample, source_format, destination_format )
Returns a string with the string ((|sample|)) with sampledata in it,
assumed to be in ((|source_format|)),
converted to the ((|destination_format|)).

A format is an array with these contents: [frequency, format (like AUDIO_S8), channels].
=end */
static __inline__ void unpack_audio_format_array(VALUE array, int* rate, Uint16* format, int* channels)
{
	*rate=NUM2INT(rb_ary_entry(array, 0));
	*format=NUM2Uint16(rb_ary_entry(array, 1));
	*channels=NUM2INT(rb_ary_entry(array, 2));
	DEBUG_I(*format);
	DEBUG_I(*channels);
	DEBUG_I(*rate);
}

static __inline__ VALUE pack_audio_format_array(int rate, Uint16 format, int channels)
{
	return rb_ary_new3(3, INT2NUM(rate), INT2NUM(format), INT2NUM(channels));
}

static VALUE sound_convert(int argc, VALUE* argv, VALUE self)
{
	int dest_rate;
	Uint16 dest_format;
	int dest_channels;

	Uint8* dest_memory;
	int dest_length;

	int src_rate;
	Uint16 src_format;
	int src_channels;

	VALUE src;
	Uint8* src_memory;
	int src_length;

	VALUE src_array, dest_array;
	
	switch(rb_scan_args(argc, argv, "21", &src, &src_array, &dest_array)){
		case 2:	{
			initAudio();
			dest_array=mixer_get_format(0);
			break;
		}
	}

	src_memory=RSTRING(src)->ptr;
	src_length=RSTRING(src)->len;

	unpack_audio_format_array(src_array, &src_rate, &src_format, &src_channels);
	unpack_audio_format_array(dest_array, &dest_rate, &dest_format, &dest_channels);

	// call converter
	SDL_VERIFY( rudl_convert_audio(
			src_memory, src_length,
			&dest_memory, &dest_length,
			src_format, src_channels, src_rate,
			dest_format, dest_channels, dest_rate) !=-1);

	// wrap result in a string
	return rb_str_new(dest_memory, dest_length);
}

/*
=begin
--- Sound#format
Returns the format of the (({Sound})), which is always the same as the Mixer's format.
See (({Mixer.format}))
=end */

/////////////// MIXER
/*
=begin
= Mixer
Mixer is the main sound class.

== Class and instance Methods
--- Mixer.new
--- Mixer.new( frequency )
--- Mixer.new( frequency, format )
--- Mixer.new( frequency, format, stereo )
--- Mixer.new( frequency, format, stereo, buffersize )

Initializes the sound system.
This call is not neccesary, the mixer will call (({Mixer.new})) when it is needed.
When you disagree with the defaults (at the time of writing: 16 bit, 22kHz, stereo, 4096)
you can set them yourself, but do this ((*before*)) using any sound related method!
* ((|frequency|)) is a number like 11025, 22050 or 44100.
* ((|format|)) is AUDIO_S8 for 8 bit samples or AUDIO_S16SYS for 16 bit samples.

  Other possibilities are listed at the bottom of this page.
* ((|channels|)) is 1 (mono) or 2 (stereo.)

* ((|buffersize|)) is how many samples are calculated in one go.

  4096 by default.
  Set higher if your sound stutters.

(({Mixer.new})) will return a Mixer object, but if you don't want it, you can
discard it and use class methods instead.

=end */
static VALUE mixer_initialize(int argc, VALUE* argv, VALUE self)
{
	int frequency = MIX_DEFAULT_FREQUENCY;
	Uint16 format = MIX_DEFAULT_FORMAT;
	int channels = MIX_DEFAULT_CHANNELS;

	int buffersize=4096;

	VALUE frequencyValue, sizeValue, stereoValue, bufferValue;

	switch(rb_scan_args(argc, argv, "04", &frequencyValue, &sizeValue, &stereoValue, &bufferValue)){

		case 4: buffersize=NUM2INT(bufferValue);
		case 3:	channels=NUM2INT(stereoValue);
		case 2:	format=NUM2Uint16(sizeValue);
		case 1:	frequency=NUM2INT(frequencyValue);
	}

	initSDL();

	if(!SDL_WasInit(SDL_INIT_AUDIO)){

		DEBUG_S("Starting audio subsystem");
		SDL_VERIFY(!SDL_InitSubSystem(SDL_INIT_AUDIO));
	}

	if(Mix_OpenAudio(frequency, format, channels, buffersize) == -1){

		DEBUG_S("Stopping audio subsystem");
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		SDL_RAISE;
	}

	return self;
}
/*
=begin
--- Mixer.destroy
--- Mixer#destroy
(was Mixer.quit)
Uninitializes the Mixer.
If you really want to do this, then be warned that all Music objects will be destroyed too!
=end */
static VALUE mixer_destroy(VALUE self)
{
	quitAudio();
	return self;
}

/*
=begin
--- Mixer.fade_out( millisecs )
--- Mixer#fade_out( millisecs )
Fades away all sound to silence in millisecs milliseconds.
=end */
static VALUE mixer_fade_out(VALUE self, VALUE time)
{
	initAudio();
	Mix_FadeOutChannel(-1, NUM2INT(time));
	return self;
}

/*
=begin
--- Mixer.find_free_channel
--- Mixer.find_oldest_channel
--- Mixer#find_free_channel
--- Mixer#find_oldest_channel
Both functions search for a channel that can be used to play new sounds on.
find_free_channel finds a channel that is not playing anything and returns nil 
if there is no such channel.
find_oldest_channel finds the channel that has been playing for the longest time.
=end */
static VALUE mixer_find_free_channel(VALUE self)
{
	int channel;
	initAudio();

	channel=Mix_GroupAvailable(-1);
	if(channel==-1){
		return Qnil;
	}

	return createChannelObject(channel);
}

static VALUE mixer_find_oldest_channel(VALUE self)
{
	initAudio();
	return createChannelObject(Mix_GroupOldest(-1));
}
/*
=begin
--- Mixer.busy?
--- Mixer#busy?
Returns the number of current active channels.
This is not the total channels, 
but the number of channels that are currently playing sound.
=end */
static VALUE mixer_get_busy(VALUE self)
{
	if(!SDL_WasInit(SDL_INIT_AUDIO)){
		return INT2NUM(0);
	}
	return INT2NUM(Mix_Playing(-1));
}

/*
=begin
--- Mixer.num_channels
--- Mixer.num_channels=( amount )
--- Mixer#num_channels
--- Mixer#num_channels=( amount )
Gets or sets the current number of channels available for the mixer.
This value defaults to 8 when the mixer is first initialized.
RUDL imposes a channel limit of 256.

These channels are not the same thing as in (({Mixer.init})).
=end */
static VALUE mixer_get_num_channels(VALUE self)
{
	initAudio();
	return INT2NUM(Mix_GroupCount(-1));
}

static VALUE mixer_set_num_channels(VALUE self, VALUE channelsValue)
{
	int channels=NUM2INT(channelsValue);
	initAudio();
	RUDL_VERIFY(channels<=MAX_CHANNELS, "256 channels ought to be enough for anyone");
	Mix_AllocateChannels(channels);
	return self;
}

/*
=begin
--- Mixer.pause
--- Mixer.unpause
--- Mixer.stop
--- Mixer#pause
--- Mixer#unpause
--- Mixer#stop
These work on all channels at once.
=end */
static VALUE mixer_pause(VALUE self)
{
	initAudio();
	Mix_Pause(-1);
	return self;
}

static VALUE mixer_stop(VALUE self)
{
	initAudio();
	Mix_HaltChannel(-1);
	return self;
}

static VALUE mixer_unpause(VALUE self)
{
	initAudio();
	Mix_Resume(-1);
	return self;
}

/*
=begin
--- Mixer.reserved=( amount )
--- Mixer#reserved=( amount )
This sets aside the first ((|amount|)) channels.
They can only be played on by directly creating a Channel object and playing on it.
No other function will take these channels for playing.
=end */
static VALUE mixer_set_reserved(VALUE self, VALUE amount)
{
	initAudio();
	Mix_ReserveChannels(NUM2INT(amount));
	return self;
}

/*
=begin
--- Mixer.driver
--- Mixer#driver
Returns the name of the driver doing the sound output.
=end */

static VALUE mixer_driver(VALUE self)
{
	int bufsize=1024;
	char*name=malloc(bufsize*sizeof(char));
	char*result;
	
	initAudio();
	
	result=SDL_AudioDriverName(name, bufsize);
	RUDL_ASSERT(result, "Audio not initialized yet");
	
	return CSTR2STR(name);
}

/*
=begin
--- Mixer.format
--- Mixer#format
These get the parameters that Mixer is playing at.
It returns an array of [format (like AUDIO_S8), channels, frequency].
See also (({Mixer.init})).
=end */

static VALUE mixer_get_format(VALUE self)
{
	int rate;
	Uint16 format;
	int channels;
	
	SDL_VERIFY( Mix_QuerySpec(&rate, &format, &channels) ==1);
	return pack_audio_format_array(rate, format, channels);
}

/////////////// MUSIC

static bool endmusic_event=false;

static void endmusic_callback(void)
{
	if(endmusic_event && SDL_WasInit(SDL_INIT_VIDEO)){
		SDL_Event e;
		memset(&e, 0, sizeof(e));
		e.type = RUDL_ENDMUSICEVENT;
		SDL_PushEvent(&e);
	}
}

Mix_Music* retrieveMusicPointer(VALUE self)
{
	Mix_Music* music;
	Data_Get_Struct(self, Mix_Music, music);
	return music;
}

void freemusic(Mix_Music* m)
{

	if(m){
		Mix_FreeMusic(m); // This is called multiple times, but it seems to work :)

	}
}

/*
=begin
= Music
This encapsulates a song.
For some reason, SDL will only play one song at a time, and even though most of the 

Music methods are instance methods, some will act on the playing Music only.
== Class Methods
--- Music.new( filename )
Creates a new Music object from a MOD, XM, MIDI, MP3 or OGG file (I think.)
=end */

static VALUE music_new(VALUE self, VALUE filename)
{
	VALUE newObject;
	initAudio();
	newObject=Data_Wrap_Struct(classMusic, 0, freemusic, Mix_LoadMUS(STR2CSTR(filename)));
	rb_obj_call_init(newObject, 0, NULL);
	return newObject;
}

/*
=begin
== Instance Methods
--- Music#volume
--- Music#volume=( loudness )
--- Music#fade_out( milliseconds )
Volume methods:
fade_out fades out ((*the currently playing music*)) to silence in ((|milliseconds|)) milliseconds.
volume and volume= get and set the volume.
Volume ranges from 0.0 to 1.0.
These methods work on the currently playing music.
=end */
static VALUE music_fade_out(VALUE self, VALUE time)
{
	Mix_FadeOutMusic(NUM2INT(time));
	return self;
}

static VALUE music_get_volume(VALUE self)
{
	return DBL2NUM(Mix_VolumeMusic(-1)/127.0);
}

static VALUE music_set_volume(VALUE self, VALUE volume)
{
	Mix_VolumeMusic((int)(NUM2DBL(volume)*127));
	return self;
}
/*
=begin
--- Music.play
--- Music.play( loops )
Plays this piece of music, stopping the previously playing one.
Plays the music one time, or loops+1 times if you pass loops.
=end */
static VALUE music_play(int argc, VALUE* argv, VALUE self)
{
	int loops = 0;

	VALUE loopsValue;

	switch(rb_scan_args(argc, argv, "01", &loopsValue)){
		case 1: loops=NUM2INT(loopsValue);
	}

	Mix_HookMusicFinished(endmusic_callback);

	playing_music=self; /* to avoid gc problem */

	SDL_VERIFY(!(Mix_PlayMusic(retrieveMusicPointer(self), loops)==-1));
	return self;
}
/*
=begin
--- Music.stop
--- Music.pause
--- Music.unpause
--- Music.busy?
--- Music.restart
All pretty straight forward, except that they all act on the playing music, not the current one.
=end */
static VALUE music_stop(VALUE self)
{
	Mix_HaltMusic();
	return self;
}

static VALUE music_pause(VALUE self)
{
	Mix_PauseMusic();
	return self;
}

static VALUE music_unpause(VALUE self)
{
	Mix_ResumeMusic();
	return self;
}

static VALUE music_get_busy(VALUE self)
{
	return INT2BOOL(Mix_PlayingMusic());
}

static VALUE music_restart(VALUE self)
{
	Mix_RewindMusic();
	return self;
}
/*
=begin
--- Music.post_end_event
--- Music.post_end_event=( onOrOff )
Returns, or sets whether an EndOfMusicEvent will be posted when the current music stops playing.
((|onOrOff|)) is true or false.
=end */
static VALUE music_set_post_end_event(VALUE self, VALUE onOff)
{
	endmusic_event=NUM2BOOL(onOff);
	return self;
}

static VALUE music_get_post_end_event(VALUE self)
{
	return INT2BOOL(endmusic_event);
}

/*
=begin
--- Music.destroy
Frees the music from memory,
thereby rendering the music instance useless.
=end */

static VALUE music_destroy(VALUE self)
{
	freemusic(retrieveMusicPointer(self));

	DATA_PTR(self)=NULL;
	return self;
}
#endif



/////////////// INIT


void initAudioClasses()
{
#ifdef HAVE_SDL_MIXER_H

	classChannel=rb_define_class_under(moduleRUDL, "Channel", rb_cObject);
	//rb_define_singleton_method(classChannel, "new", channel_new, 1);

	rb_eval_string(
			"module RUDL class Channel				\n"
			"	def initialize(nr)					\n"
			"		@number=nr						\n"
			"	end	end	end							\n"
	);

	rb_define_method(classChannel, "fade_out", channel_fade_out, 1);
	rb_define_method(classChannel, "busy?", channel_busy, 0);
	rb_define_method(classChannel, "volume", channel_get_volume, 0);
	rb_define_method(classChannel, "volume=", channel_set_volume, 1);
	rb_define_method(classChannel, "play", channel_play, -1);
	rb_define_method(classChannel, "stop", channel_stop, 0);
	rb_define_method(classChannel, "pause", channel_pause, 0);
	rb_define_method(classChannel, "unpause", channel_unpause, 0);
	rb_define_method(classChannel, "set_panning", channel_set_panning, 2);
	rb_define_method(classChannel, "set_position", channel_set_position, 2);
	rb_define_method(classChannel, "distance=", channel_set_distance, 1);
	rb_define_method(classChannel, "reverse_stereo", channel_reverse_stereo, 2);

	classSound=rb_define_class_under(moduleRUDL, "Sound", rb_cObject);
	rb_define_singleton_method(classSound, "new", sound_new, 1);

	rb_define_singleton_method(classSound, "load_new", sound_new, 1);

	rb_define_singleton_method(classSound, "import", sound_import, 1);

	rb_define_singleton_method(classSound, "convert", sound_convert, -1);

//	rb_define_singleton_method(classSound, "mix", sound_mix, 3); - maybe in 0.8

	rb_define_method(rb_cString, "to_sound", string_to_sound, 0);

	rb_define_method(classSound, "fade_out", sound_fade_out, 1);
	rb_define_method(classSound, "num_channels", sound_get_num_channels, 0);
	rb_define_method(classSound, "volume", sound_get_volume, 0);
	rb_define_method(classSound, "play", sound_play, -1);
	rb_define_method(classSound, "volume=", sound_set_volume, 1);
	rb_define_method(classSound, "stop", sound_stop, 0);

	rb_define_method(classSound, "export", sound_export, 0);

	rb_define_method(classSound, "format", mixer_get_format, 0);

	classMixer=rb_define_class_under(moduleRUDL, "Mixer", rb_cObject);

	rb_define_method(classMixer, "initialize", mixer_initialize, -1);
	rb_define_singleton_and_instance_method(classMixer, "destroy", mixer_destroy, 0);

	rb_define_singleton_and_instance_method(classMixer, "fade_out", mixer_fade_out, 1);
	rb_define_singleton_and_instance_method(classMixer, "find_free_channel", mixer_find_free_channel, 0);
	rb_define_singleton_and_instance_method(classMixer, "find_oldest_channel", mixer_find_oldest_channel, 0);
	rb_define_singleton_and_instance_method(classMixer, "busy?", mixer_get_busy, 0);
	rb_define_singleton_and_instance_method(classMixer, "num_channels", mixer_get_num_channels, 0);
	rb_define_singleton_and_instance_method(classMixer, "pause", mixer_pause, 0);
	rb_define_singleton_and_instance_method(classMixer, "num_channels=", mixer_set_num_channels, 1);
	rb_define_singleton_and_instance_method(classMixer, "reserved=", mixer_set_reserved, 1);
	rb_define_singleton_and_instance_method(classMixer, "stop", mixer_stop, 0);
	rb_define_singleton_and_instance_method(classMixer, "unpause", mixer_unpause, 0);
	
	rb_define_singleton_and_instance_method(classMixer, "driver", mixer_driver, 0);
	
	rb_define_singleton_and_instance_method(classMixer, "format", mixer_get_format, 0);

	classMusic=rb_define_class_under(moduleRUDL, "Music", rb_cObject);
	rb_define_singleton_method(classMusic, "new", music_new, 1);
	rb_define_method(classMusic, "busy?", music_get_busy, 0);
	rb_define_method(classMusic, "fade_out", music_fade_out, 1);
	rb_define_method(classMusic, "post_end_event=", music_set_post_end_event, 1);
	rb_define_method(classMusic, "post_end_event", music_get_post_end_event, 1);
	rb_define_method(classMusic, "volume", music_get_volume, 0);
	rb_define_method(classMusic, "volume=", music_set_volume, 1);
	rb_define_method(classMusic, "play", music_play, -1);
	rb_define_method(classMusic, "stop", music_stop, 0);
	rb_define_method(classMusic, "restart", music_restart, 0);
	rb_define_method(classMusic, "pause", music_pause, 0);
	rb_define_method(classMusic, "unpause", music_unpause, 0);
	rb_define_method(classMusic, "destroy", music_destroy, 0);

	// Backward compatibility
	rb_alias(classMusic, rb_intern("dealloc"), rb_intern("destroy"));

	rb_define_singleton_method(classMixer, "init", mixer_initialize, -1);
/*
=begin
= EndOfMusicEvent
This event is posted when the current music has ended.
=end */
	classEndOfMusicEvent=rb_define_class_under(moduleRUDL, "EndOfMusicEvent", rb_cObject);


/*
=begin
= Constants
These are for indicating an audio format:
AUDIO_U8, AUDIO_S8, AUDIO_U16LSB, AUDIO_S16LSB, AUDIO_U16MSB, AUDIO_S16MSB, 
AUDIO_U16, AUDIO_S16, AUDIO_U16SYS, AUDIO_S16SYS
=end */


	DEC_CONSTN(AUDIO_U8);
	DEC_CONSTN(AUDIO_S8);
	DEC_CONSTN(AUDIO_U16LSB);
	DEC_CONSTN(AUDIO_S16LSB);
	DEC_CONSTN(AUDIO_U16MSB);
	DEC_CONSTN(AUDIO_S16MSB);
	DEC_CONSTN(AUDIO_U16);
	DEC_CONSTN(AUDIO_S16);
	DEC_CONSTN(AUDIO_U16SYS);
	DEC_CONSTN(AUDIO_S16SYS);

	clearGCHack();
#endif
}
