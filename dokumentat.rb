require 'getoptlong'

=begin
--- Organizing documentation:
@file some.hierarchy (controls placement in files)
@class class_name (starts or continues documenting a class, include the whole hierarchy please)
@module module_name (starts or continues documenting a class, include the whole hierarchy please)
@section section_name (starts or continues documenting a class, include the whole hierarchy please)
@method method_name(param,param,param) > retval

Text before the first organizational tag will be put in index.html

--- Describing stuff:
@param paramname description
@see some.hierarchy.class_or_module_or_section_name.method description
@throws exception description
--- Formatting:
All HTML, with "enter enter" for new paragraph. *word* turns into bold, /word/ turns into italics.
--- Example:
/**
Welcome to RUDL.
@file RUDL.Audio --> Turns into file output_dir/rudl/audio
	Optional description for the top of this file.
	@module Audio
		This module contains all of RUDL's audio stuff.
	@class Mixer
		Here be the main mixer.
		@section Initializer
			@method initialize(a, b, c)
			@method initialize(a, b) 
				@param a rather vague parameter it is.
		@section Instance methods
			@method set_volume(loudness)
				Sets volume to /loudness/
				@param loudness value from 0-255
				@see mute
				@see mixerboy.playa.mute
			@method mute
				@throws IWontQuitException
			Turns off all sound
	@class Sound
	This class roxxorz
	
*/
=end

$verbose

class Dokumentat
	def initialize(output_dir)
	end
	
	def process_dir(source_path)
	end
end


def main
	options = GetoptLong.new(
		["--output-dir",	"-o",	GetoptLong::REQUIRED_ARGUMENT],
		["--verbose",		"-v",	GetoptLong::NO_ARGUMENT ]
	)
	
end

main