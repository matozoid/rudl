require 'getoptlong'
require 'ftools'

=begin
--- Organizing documentation:
A hierarchy is project.class_or_module.method.parameter.
Incompletely specified hierarchies will be completed by searching to the left
through the hierarchy currently being processed:rightmost: parameters of this method
then: methods of this class or module
then: go through nested classes and modules, looking at methods (or class and module names??)then: go through all project names known

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
$open_tag='/**'
$close_tag='*/'

class TagPosition
	attr_reader :project_name, :file_name, :tag_name
	
	def initialize(project_name, file_name, tag_name)
		@project_name=project_name
		@file_name=file_name
		@tag_name=tag_name
	end
end

=begin
current_hierarchy: ["someproject", "somefile", "somefile.someclass(es)", "somefile.someclass.somemethod"]
index: {
	"someproject.somefile.someclass" => someproject, somefile, sometag
	...
}

=end

class Entry
	def link
		"<a href='index.html'>back</a>"
	end
end

class ProjectEntry < Entry
	def initialize(name)
		@name=name	end
	
	def link
		"<a href='index.html'>#{@project_name}</a>"
	end
end

class FileEntry < Entry
	def initialize(name)
		@name=name	end
	
	def link
		"aaa"
	end
end

class ClassEntry < Entry
	def initialize(name)
		@name=name	end
end

class ModuleEntry < Entry
	def initialize(name)
		@name=name	end
end

class MethodEntry < Entry
	def initialize(name)
		@name=name	end
end

class ParameterEntry < Entry
	def initialize(name)
		@name=name	end
	
end

class Dokumentat
	def initialize(project_name)
		say "Starting project #{project_name}"
		@project_name=project_name
		@project=ProjectEntry.new(@project_name)
		@hierarchy={@project=>{}}
	end
	
	def backup_path_to_index_class(path, class_type)
		while(path.length>0 && path[path.length-1]!=class_type)
			p path
			path=path[0..-2]
		end	end
	
	def process_dir(source_path)
		files=Dir[source_path]
		files.each do |file_name|
			path=[@project]
			process_file(file_name, path)
		end
	end
	
	def process_file(file_name, path)
		say "Processing file #{file_name}"
		path.push FileEntry.new(file_name)
		File.open(file_name, 'r') do |file|
			file.read.scan(/\/\*\*\s*(.*?)\s*\*\//m)  do |block|
				p block
				#lines=
			end
		end
	end
	
	def write(output_path)
		say "Writing result to #{output_path}/"
		File.makedirs(output_path)	end
	
	def say(text)
		puts text if($verbose)
	end
end


def main
	options = GetoptLong.new(
		["--project-name",	"-p",		GetoptLong::REQUIRED_ARGUMENT],
		["--output-dir",	"-o",		GetoptLong::REQUIRED_ARGUMENT],
		["--verbose",		"-v",		GetoptLong::NO_ARGUMENT ],
		["--open-tag",		"-O",		GetoptLong::REQUIRED_ARGUMENT],
		["--close-tag",		"-C",		GetoptLong::REQUIRED_ARGUMENT]
	)
	project_name=nil
	output_dir=nil
	
	options.each do |opt, arg|
		case opt
			when '--verbose'
				$verbose=true
			when '--project-name'
				project_name=arg
			when '--output-dir'
				output_dir=arg
			when '--open-tag'
				$open_tag=arg
			when '--open-tag'
				$open_tag=arg
			else
				raise "Unknown option: #{opt}"
		end
	end

	raise "No project name defined" if(!project_name)

	dokumentat=Dokumentat.new(project_name)

	ARGV.each do |path|
		dokumentat.process_dir(path)
	end
	
	dokumentat.write(output_dir)
end

ARGV=['--verbose', '--project-name=rudl', '--output-dir=docs', 'dokumentat.rb', 'dokme/*.c', 'dokme/dokmetoo/*.c']

main
