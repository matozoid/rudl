require 'getoptlong'
require 'ftools'
require 'pp'

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
*/
/**
@file Audio
--> Will output into file output_dir/project_name/audio.html. If you don't file, it will go into index.html.
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
	attr_reader :children
	attr_reader :name
	
	def initialize(name)
		@name=name
		@children=[]
		@text=nil
	end
	
	def <=>(other)
		@name<=>other.name
	end
	
	def ==(other)
		@name==other.name
	end
	
	def link
		"<a href='index.html'>back</a>"
	end
	
	def add_text(text)
		if @text
			@text=@text+text
		else
			@text=''
		end
	end
	
	def write(output_directory)
		children.each do |child|
			child.write(output_directory)
		end
	end
end

class RootEntry < Entry
end

class ProjectEntry < Entry
end

class FileEntry < Entry
	def write(output_directory)
		File.open(output_directory+'/'+@name.downcase+'.html', 'w') do |file|
			children.sort.each do |child|
				child.write(file)
			end
		end	end
end

class ClassEntry < Entry
	def write(file)
		file.write("<h2>#{@name}</h2>\n")
		file.write("#{@text}")
	end
end

class ModuleEntry < Entry
	def write(file)
		file.write("<h2>#{@name}</h2>\n")
		file.write("#{@text}")
	end
end

class SectionEntry < Entry
	def write(file)
		file.write("<h3>#{@name}</h3>\n")
		file.write("#{@text}")
	end
end

class MethodEntry < Entry
	def write(file)
		file.write("<h3>#{@name}</h3>\n")
		file.write("#{@text}")
	end
end

class ParameterEntry < Entry
end

class Path
	attr_reader :path
	def initialize(root)
		@path=[root]
	end
	
	def goto(entry)
		last_in_path=@path[-1]
		if last_in_path.children.include?(entry)
			entry=last_in_path.children[last_in_path.children.index(entry)]
		else
			last_in_path.children.push entry
		end
		@path.push(entry)	end
	
	def goto_file_level
		@path=@path[0..1] # bad, should be search for class
	end
	
	def goto_section_level
		@path=@path[0..2] # bad
	end
	
	def goto_class_level
	end
	
	def goto_method_level
	end
	
	def backup_to(path, class_type)
		while(@path.length>0 && @path[@path.length-1]!=class_type)
			p @path
			@path=@path[0..-2]
		end	end
	
	def deepest_node
		@path[-1]
	end
	
end

class Dokumentat
	def initialize(project_name)
		say "Starting project #{project_name}"
		@project_name=project_name
		@project=ProjectEntry.new(@project_name)
		@root=RootEntry.new("root")
		@root.children.push(@project)
	end
	
	def process_dir(source_path)
		files=Dir[source_path]
		files.each do |file_name|
			path=Path.new(@root)
			path.goto(@project)
			process_file(file_name, path)
		end
	end
	
	def process_file(file_name, path)
		say "Processing file #{file_name}"
		File.open(file_name, 'r') do |file|
			file.read.scan(/\/\*\*\s*(.*?)\s*\*\//m)  do |block|
				lines=block.to_s.split("\n")
				section_mode=true
				text=""
				lines.each do |line|
					if line[0]!=?@
						section_mode=false
					end
					if section_mode
						type, text=(line.match /@([^ ]*) (.*)/)[1..2]
						case type.downcase
							when 'file'
								path.goto_file_level
								path.goto(FileEntry.new(text))
							when 'section'
								path.goto_section_level
								path.goto(SectionEntry.new(text))
							when 'method'
								path.goto_method_level
								path.goto(MethodEntry.new(text))
							when 'class'
								path.goto_class_level
								path.goto(ClassEntry.new(text))
							when 'module'
								path.goto_class_level
								path.goto(ModuleEntry.new(text))
							else
								say "Unknown section: #{line}"
						end
					else
						if line.strip.length==0
							text=text+'\n<p>'
						else
							text=text+line
						end
					end
				end
				path.deepest_node.add_text(text)
			end
		end
	end
	
	def write(output_path)
		say "Writing result to #{output_path}/"
		File.makedirs(output_path)
		pp @root
		@root.write(output_path)	end
	
	def say(text)
		puts text if $verbose
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
