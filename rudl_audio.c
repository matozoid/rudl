/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl_audio.h"

#ifdef HAVE_SDL_MIXER_H
#include "SDL_mixer.h"

// Thanks to Ruby-SDL (should be fixed a little though, I think? Looks like things
// in here that are getting gc-protection are actually never freed. Then again,
// who cares?)
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

//////////////////
Mix_Chunk* retrieveMixChunk(VALUE self);

void initAudio()
{
	if(!SDL_WasInit(SDL_INIT_AUDIO)){
		initSDL();

		if(!SDL_WasInit(SDL_INIT_AUDIO)){
			if(SDL_InitSubSystem(SDL_INIT_AUDIO)){
				SDL_RAISE;
			}
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
	rb_eval_string("ObjectSpace.each_object(RUDL::Music) {|x| x.dealloc}");
	if(SDL_WasInit(SDL_INIT_AUDIO)){
		Mix_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}

/////////////// CHANNEL

VALUE createChannelObject(int number)
{
	VALUE newChannel=rb_obj_alloc(classChannel);
	rb_iv_set(newChannel, "@number", INT2NUM(number));
	return newChannel;
}

int retrieveChannelNumber(VALUE channel)
{
	return NUM2INT(rb_iv_get(channel, "@number"));
}
/*
=begin
= Channel
A Channel is an interface object to one of the mixer's channels.
It can be used for manipulating sound on a particular channel.
== Class Methods
--- Channel.new( number )
Creates a Channel interface object for mixer channel ((|number|)).
=end */
static VALUE channel_new(VALUE self, VALUE number)
{
	VALUE newObject;
	initAudio();
	newObject=createChannelObject(NUM2INT(number));
	rb_obj_call_init(newObject, 0, NULL);
	return newObject;
}

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
	return (DBL2NUM(Mix_Volume(retrieveChannelNumber(self), -1))/128.0);
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
	VALUE sound, loopsValue, maxtimeValue;
	Mix_Chunk* chunk=retrieveMixChunk(sound);

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
/////////////// SOUND
Mix_Chunk* retrieveMixChunk(VALUE self)
{
	Mix_Chunk* chunk;
	Data_Get_Struct(self, Mix_Chunk, chunk);
	return chunk;
}

/*
=begin
= Sound
Sound is a single sample.
It is loaded from a WAV file.
== Class Methods
--- Sound.new( filename )
Creates a new (({Sound})) object with the sound in file ((|filename|)).
=end */

static VALUE sound_new(VALUE self, VALUE filename)
{
	Mix_Chunk* chunk;
	VALUE newObject;
	
	initAudio();

	chunk=Mix_LoadWAV(STR2CSTR(filename));
	
	if(!chunk) SDL_RAISE;
	
	newObject=Data_Wrap_Struct(classSound, 0, SDL_FreeWAV, chunk);
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
	return DBL2NUM(Mix_VolumeChunk(retrieveMixChunk(self), -1))/128.0;
}

static VALUE sound_set_volume(VALUE self, VALUE volume)
{
	Mix_VolumeChunk(retrieveMixChunk(self), (int)(NUM2DBL(volume)*128));
	return self;
}

/*
=begin
--- Channel#play( loops, maxtime )
Starts playing a song on an available channel.
If no channels are available, it will not play and return ((|nil|)).
Loops controls how many extra times the sound will play, a negative loop will play
indefinitely, it defaults to 0.
Maxtime is the number of total milliseconds that the sound will play.
It defaults to forever (-1).
Returns a channel object for the channel that is selected to play the sound.
--- Channel#stop
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

	playing_wave[channelnum]=self; /* to avoid gc problem */
	
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
/////////////// MIXER
/*
=begin
= Mixer
Mixer is the main sound class.
Since there is only one of these, all methods are class methods.
== Class Methods
--- Mixer.init
--- Mixer.init( frequency )
--- Mixer.init( frequency, size )
--- Mixer.init( frequency, size, stereo )
Initializes the sound system.
This call is not neccesary, the mixer will call (({Mixer.init})) when it is needed.
When you disagree with the defaults (at the time of writing: 16 bit, 22kHz, stereo)
you can set them yourself, but do this ((*before*)) using any sound related method!
* ((|frequency|)) is a number like 11025, 22050 or 44100.
* ((|size|)) is AUDIO_S8 for 8 bit samples or AUDIO_S16SYS for 16 bit samples.
* ((|stereo|)) is 1 (mono) or 2 (stereo.)
=end */
static VALUE mixer_init(int argc, VALUE* argv, VALUE self)
{
	int frequency = MIX_DEFAULT_FREQUENCY;
	int size = MIX_DEFAULT_FORMAT;
	int stereo = MIX_DEFAULT_CHANNELS;

	VALUE frequencyValue, sizeValue, stereoValue;

	switch(rb_scan_args(argc, argv, "03", &frequencyValue, &sizeValue, &stereoValue)){
		case 3:	stereo=NUM2INT(stereoValue);
		case 2:	size=NUM2INT(sizeValue);
		case 1:	frequency=NUM2INT(frequencyValue);
	}

	initSDL();

	if(!SDL_WasInit(SDL_INIT_AUDIO)){
		if(SDL_InitSubSystem(SDL_INIT_AUDIO)){
			SDL_RAISE;
		}
	}

	if(Mix_OpenAudio(frequency, size, stereo, 1024) == -1){
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		SDL_RAISE;
	}

	return self;
}
/*
=begin
--- Mixer.quit
Uninitializes the Mixer.
If you really want to do this, then be warned that all Music objects will be destroyed!
=end */
static VALUE mixer_quit(VALUE self)
{
	quitAudio();
	return self;
}

/*
=begin
--- Mixer.fade_out( millisecs )
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
Returns the number of current active channels. This is not the
total channels, but the number of channels that are currently
playing sound.
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
Gets or sets the current number of channels available for the mixer.
This value defaults to 8 when the mixer is first initialized.
RUDL imposes a channel limit of 256.
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
	
	if(channels>MAX_CHANNELS){
		SDL_RAISE_S("256 channels ought to be enough for anyone");
	}
	
	Mix_AllocateChannels(channels);

	return self;
}

/*
=begin
--- Mixer.pause
--- Mixer.unpause
--- Mixer.stop
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
	Mix_FreeMusic(m); // This is called multiple times, but it seems to work :)
}

/*
=begin
= Music
This encapsulates a song.
For some reason, SDL will only play one song at a time, and even though most of the Music methods
are instance methods, some will act on the playing Music only.
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

	if(Mix_PlayMusic(retrieveMusicPointer(self), loops)==-1) SDL_RAISE;
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

static VALUE music_dealloc(VALUE self)
{
	freemusic(retrieveMusicPointer(self));
	return self;
}
#endif
/////////////// INIT
void initAudioClasses()
{
	classChannel=rb_define_class_under(moduleRUDL, "Channel", rb_cObject);
#ifdef HAVE_SDL_MIXER_H
	rb_define_singleton_method(classChannel, "new", channel_new, 1);
	rb_define_method(classChannel, "fade_out", channel_fade_out, 1);
	rb_define_method(classChannel, "busy?", channel_busy, 0);
	rb_define_method(classChannel, "volume", channel_get_volume, 0);
	rb_define_method(classChannel, "volume=", channel_set_volume, 1);
	rb_define_method(classChannel, "play", channel_play, -1);
	rb_define_method(classChannel, "stop", channel_stop, 0);
	rb_define_method(classChannel, "pause", channel_pause, 0);
	rb_define_method(classChannel, "unpause", channel_unpause, 0);
#endif
	classSound=rb_define_class_under(moduleRUDL, "Sound", rb_cObject);
#ifdef HAVE_SDL_MIXER_H
	rb_define_singleton_method(classSound, "new", sound_new, 1);
	rb_define_method(classSound, "fade_out", sound_fade_out, 1);
	rb_define_method(classSound, "num_channels", sound_get_num_channels, 0);
	rb_define_method(classSound, "volume", sound_get_volume, 0);
	rb_define_method(classSound, "play", sound_play, -1);
	rb_define_method(classSound, "volume=", sound_set_volume, 1);
	rb_define_method(classSound, "stop", sound_stop, 0);
#endif
	classMixer=rb_define_module_under(moduleRUDL, "Mixer");
#ifdef HAVE_SDL_MIXER_H
	rb_define_singleton_method(classMixer, "init", mixer_init, -1);
	rb_define_singleton_method(classMixer, "quit", mixer_quit, 0);
	rb_define_singleton_method(classMixer, "fade_out", mixer_fade_out, 1);
	rb_define_singleton_method(classMixer, "find_free_channel", mixer_find_free_channel, 0);
	rb_define_singleton_method(classMixer, "find_oldest_channel", mixer_find_oldest_channel, 0);
	rb_define_singleton_method(classMixer, "busy?", mixer_get_busy, 0);
	rb_define_singleton_method(classMixer, "num_channels", mixer_get_num_channels, 0);
	rb_define_singleton_method(classMixer, "pause", mixer_pause, 0);
	rb_define_singleton_method(classMixer, "num_channels=", mixer_set_num_channels, 1);
	rb_define_singleton_method(classMixer, "reserved=", mixer_set_reserved, 1);
	rb_define_singleton_method(classMixer, "stop", mixer_stop, 0);
	rb_define_singleton_method(classMixer, "unpause", mixer_unpause, 0);
#endif
	classMusic=rb_define_class_under(moduleRUDL, "Music", rb_cObject);
#ifdef HAVE_SDL_MIXER_H
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
	rb_define_method(classMusic, "dealloc", music_dealloc, 0);
#endif
/*
=begin
= EndOfMusicEvent
This event is posted when the current music has ended.
=end */
	classEndOfMusicEvent=rb_define_class_under(moduleRUDL, "EndOfMusicEvent", rb_cObject);

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

#ifdef HAVE_SDL_MIXER_H
	clearGCHack();
#endif
}
