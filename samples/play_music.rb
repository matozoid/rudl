require 'RUDL'; include RUDL; include Constant
=begin
@file samples
@section play_music
This sample plays a bit of music.
=end
# Set the mixer to some nice values. If we skip this, it starts itself up when# it is needed and chooses some default values then.# Odd: the "mixer" variable is not needed, every method on it is also available on# class Mixer itself.mixer=Mixer.new(44100, AUDIO_S16SYS, 2, 16384)# Load a music file and play it. Simple huh?# Odd: when you say "happysong.stop", it doesn't actually stop happysong. It always# stops the song that is currently playing. Some other methods do this too.music=Music.new('media/crapola_ha_ha_thump.mod')music.playputs 'Just an example that plays some music'gets