require 'getoptlong'

=begin
--- Organizing documentation:
A hierarchy is project.class_or_module.method.parameter.
Incompletely specified hierarchies will be completed by searching to the left
through the hierarchy currently being processed:rightmost: parameters of this method
then: methods of this class or module
then: go through nested classes and modules, looking at methods (or class and module names??)then: go through all projects known

@file some.hierarchy (controls placement in files)
@class class_name (starts or continues documenting a class, include the whole hierarchy please)
@module module_name (starts or continues documenting a module, include the whole hierarchy please)
@section section_name (starts a user defined section within the current block)
@method method_name(param,param,param) > retval

Text before the first organizational tag will be put in index.html

--- Describing stuff:
@throws exception description
--- Formatting:
All HTML, except @something.

/** starts a dokumentat block
@something tells us what we're documenting
@something continue adding these until you're done
The first line that doesn't start with @ is the documentation itself. 
From here on, every @something is seen as a reference to something in the hierarchy. 
If you want to start documenting something else, close the comment and start over.
*/ ends a dokumentat block
--- Example:
/**
Welcome to RUDL.
@file Audio --> Will output into file output_dir/project_name/audio.html. If you don't file, it will go into index.html.
Optional description for the top of this file.
*/
/**
@section Hello all
This is a nice section of stuff.
*/
/**
@module RUDL.Audio
This module contains all of RUDL's audio stuff.
*/
/**
@section Beer
Some important information about beer.
*/
/**
@class Mixer
Here be the main mixer.
*/
/**
@section Initializer
@method initialize(a, b, c)
@method initialize(a, b) 
Do all this great initialization stuff. Methods get referenced by name: @initialize
*/
/**
@section Instance methods
@method set_volume(loudness)
Sets volume to @loudness. 
See @mute and @mixerboy.playa.mute
*/
/**
@method mute
Turns off all sound
@throws IWontQuitException
*/
/**
@class Sound
This class roxxorz
	
*/
=end

$verbose=false

class Dokumentat
	def initialize(project_name)
		say "Starting project #{project_name}"
	end
	
	def process_dir(source_path)
	end
	
	def write(output_path)	end
	
	def say(text)
		puts text if($verbose)
	end
end


def main
	options = GetoptLong.new(
		["--project-name",	"-p", GetoptLong::REQUIRED_ARGUMENT],
		["--output-dir",	"-o",	GetoptLong::REQUIRED_ARGUMENT],
		["--verbose",		"-v",	GetoptLong::NO_ARGUMENT ]
	)
	project_name=nil
	output_dir=nil
	
	options.each do |opt, arg|
		puts "Option: #{opt}, arg #{arg.inspect}"
		$verbose=true if(opt=='--verbose')
		project_name=arg if(opt=='--project-name')
		output_dir=arg if(opt=='--output-dir')
	end

	raise "No project name defined" if(!project_name)

	dokumentat=Dokumentat.new(project_name)

	ARGV.each do |path|
		dokumentat.process_dir(path)
	end
	
	dokumentat.write(output_dir)
	
	puts "Remaining args: #{ARGV.join(', ')}"
end

main