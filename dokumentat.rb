require 'getoptlong'
require 'ftools'
require 'pp'

=begin
@file Dokumentat
@section 1. Dokumentat
Welcome to Dokumentat, the simple documenter for Ruby C extensions.
Created by Danny van Bruggen and Renne Nissinen.
=end

=begin
@section 2. Why another document system?
After using rd2 for a while, we got annoyed by the oddness of the syntax,
the slowness of the software and the low quality of the output.
Low quality, because the generated html we used contained no markers to
link to, making it impossible to do cross references.
Slow is not really an argument, but it got annoying at times. Dokumentat
doesn't really do a lot, so it's fast.
The syntax, with lots of ((|markers|)) and &lt;&lt;Weirdness|http://nu.nl&gt;&gt; was
hard to remember, and looking up the right codes wasn't something we liked
to do in the middle of programming.

Rdoc, another system, is an impressive piece of software.
However, it was specifically written for Ruby software, not the C extension
libraries.
It promises to document C libraries too, but we got Rdoc pretty confused with
our macros.
It's a matter of taste, but we think the HTML output is painfully hard to read
and plain ugly with all the frames and listings.
=end

=begin
@section 3. What do we promise?
We promise to give you a system that makes documenting your C library,
and optionally your Ruby source, easy.
We mixed rd2 and javadoc, compromised a lot and came up with a simple system
for throwing your documentation together.
=end

=begin
@section 4. The general idea
You start Dokumentat by offering it all the files with documentation in them.
They will be read, and everything starting with a start tag and ending with an end tag
will be seen as a separate documentation block.
Most blocks will start with one or more lines of classification: where the following text
should appear, which method, which class, which file...
The first line that is not a classification is the start of the documentation,
which continues up to the end tag.

The begin and end tags for C files are <code>/**</code> and <code>*/</code> like Javadoc.
Lines are not supposed to start with <code>*</code> though.
The begin and end tags for Ruby files are <code>=begin</code> and <code>=end</code>.
Begin tags are expected to be at the start of a line. End tags may appear anywhere.
=end

=begin
@section 5. Classification
Every Dokumentat tag starts with @@.
If you need the @@ itself, double it: @@@@.
These are the classification tags: <code>@@file</code>, <code>@@section</code>, <code>@@module</code>, <code>@@class</code> and <code>@@method</code>.

<code>@@file</code> specifies in which output file the rest of the documentation in the input file should be stored.
Since it appears in the title of the output page, it should look good, like "@@file Documentation for dokumentat"
You can use more than one <code>@@file</code> statement in one source file, and you can use the same <code>@@file</code>
statement in multiple source files (to get the documentation from multiple files to show up in one
output file.)

Example:

@@file Video stuff

<code>@@module</code> and <code>@@class</code> tell Dokumentat for which module we will encounter documentation.
Specify modules and classes completely, they do not nest.

Example: @@module RUDL, @@class RUDL::Audio

<code>@@method</code> says that documentation for a method is coming up.
Methods are pooled by name.
This means that if multiple methods with the same name are encountered
in a class, section or wherever, they are treated as the same thing, but
with a different parameter list.
Documentation is pooled together for all methods of the same name.

Example:

@@method do_stuff(somevar, bla) -> ReturnedThing

@@method help(someone)

@@method help(someone, reason) (this one will be pooled with help(someone) )

<code>@@section</code> is a way to subdivide one of the previous classifications.
You can have sections with groups of methods, or chapters like the documentation you are
reading now, or whatever you like.

A hierarchy is built with these. A file can contain modules, sections, methods and classes;
modules and classes can contain sections and methods; methods can contain sections, etc.
Everything that is contained will be sorted when Dokumentat starts writing the output files.
=end

=begin
@section 6. The documentation itself.
The documentation starts after the classification lines,
identified by the first line that doesn't start with @@.
You can type anything you like here and it will be transfered literally to the
resulting HTML documents,
with the exception of words starting with @@.
Any word starting with @@ is seen as a reference.
=end

=begin
@section 7. Linking
Not implemented yet.
=end

=begin
@section 8. Parameters
<code>--project-name</code> specifies the name of your project.
This documentation was generated with project name "dokumentat"
It is used for the directory name.

<code>--output-dir</code> specifies the root for all Dokumentat documentation,
for all projects that use it.
This keeps all documentation nicely in one place.

<code>--verbose</code> makes Dokumentat noisy.

<code>--extras-dir</code> specifies the directory that contains additional files that can not
be generated, like images.
The contents will be copied over to the project output dir.

The rest of the parameters will be interpreted as path to files.
They can contain wildcards, and anything else that means something to Ruby's Dir[].
=end


=begin
@section 9. Output
Output is HTML with structural tags only. You should do visual formatting with CSS.
<ul>
<li>Class and module titles are put in &lt;h2&gt; blocks</li>
<li>Section names are in &lt;h3&gt; blocks</li>
<li>Method synopses are in &lt;h4&gt; blocks</li>
<li>The actual documentation text is in &lt;p&gt; blocks</li>
</ul>

=begin
@file source
@module Output
This module contains
=end
module Output
	def Output.stylesheet
<<SHEET
body {
	margin-left: 6% !important;
	margin-right: 4% !important;
}

body, h1, h2, h3, h4, h5, h6, p {
	font-family: "Arial", "Helvetica", "sans-serif";
}

h1{
	text-align: center;
	border-top: solid thick;
	margin-top: 2em;
}

code {
	font-weight: bold;
}

h2 {
	margin-top: 3em;
	border-top: solid thin;
}

dl {
	margin-top: 2em;
	border-top: solid thin;
	border-top-style: dotted;
}

ul, ol>li {
	margin-bottom: 0.5em;
}
SHEET
	end

	def Output.html_header(title)
<<HEADER
<html>
<head>
	<title>#{title}</title>
	<link rel='stylesheet' title='Dokumentat' href='../dokumentat.css' media='screen,projection' />
</head>
<body>

HEADER
	end

	def Output.html_footer
		"\n</body>\n</html>\n"
	end
	
	def Output.format_link(target, text)
		"<a href='#{target}'>#{text}</a>"
	end
	
	def Output.format_class(class_name)
		"<h2>#{class_name}</h2>\n"
	end
	
	def Output.format_method(name, parameter_list)
		"<h4>#{name}#{parameter_list}</h4>\n"
	end

	def Output.format_text(text)
		"#{@text}"
	end
end

ARGV=[
	'--verbose',
	'--project-name=dokumentat',
	'--output-dir=docs',
	'dokumentat.rb'
] if ARGV.empty?

#ARGV=['--verbose', '--project-name=rudl', '--output-dir=docs', 'test.c']

$verbose=false
$extras_dir=nil

class Array
	def remove_starting_at_class(cls)
		idx=-1
		each do |o|
			if o.is_a? cls
				if idx==-1
					replace([])
					return
				end
				replace(self[0..idx])
				return
			end
			idx+=1
		end
	end

	def contains_class?(cls)
		each do |o|
			if o.is_a? cls
				return true
			end
		end
		false
	end
end

class TagPosition
	attr_reader :project_name, :file_name, :tag_name

	def initialize(project_name, file_name, tag_name)
		@project_name=project_name
		@file_name=file_name
		@tag_name=tag_name
	end
end

=begin
@class Entry
An entry in the TOC. Subclasses specify specific kinds of entries.
An entry holds all entries below it in @children.
=end
class Entry
	attr_reader :children
	attr_reader :name

	# The first time an entry with this name has been found:
	def initialize(name)
		@name=name
		@children=[]
		@text=nil
	end

	# If an entry (like MethodEntry) can contain more than one line (for parameter lists),
	# this method will be called the second time and up.
	def reenter(name)
	end

	def <=>(other)
		@name<=>other.name
	end

	def ==(other)
		@name==other.name
	end

	def add_text(text)
		if @text
			@text=@text+text
		else
			@text=text
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
	def write(output_directory)
		children.sort.each do |child|
			project_dir="#{output_directory}/#{@name}"
			File.makedirs(project_dir)
			child.write(project_dir)
		end
	end
end

class FileEntry < Entry
	def write(output_directory)
		File.open(output_directory+'/'+@name.downcase+'.html', 'w') do |file|
			file.write(Output::html_header(@name))
			children.sort.each do |child|
				child.write(file)
			end
			file.write(Output::html_footer)
		end	end
end

class ClassEntry < Entry
	def write(file)
		file.write(Output::format_class(@name))
		file.write(Output::format_text(@text))
		children.sort.each do |child|
			child.write(file)
		end
	end
end

class ModuleEntry < Entry
	def write(file)
		file.write("<h2>#{@name}</h2>\n")
		file.write(Output::format_text(@text))
		children.sort.each do |child|
			child.write(file)
		end
	end
end

class SectionEntry < Entry
	def write(file)
		file.write("<h3>#{@name}</h3>\n")
		file.write("#{@text}")
		children.sort.each do |child|
			child.write(file)
		end
	end
end

class MethodEntry < Entry
	def initialize(name)
		@parameterlists=[]
		super(reenter(name))
	end

	def reenter(name)
		methodname, parameterlist=name.split('(') # uglyyyyyyyyyy, could theoretically split more than twice
		if parameterlist
			parameterlist='('+parameterlist
		else
			parameterlist=''
		end
		parameterlist.gsub(/->/, '&rarr;')
		@parameterlists.push(parameterlist)
		methodname
	end

	def write(file)
		@parameterlists.sort.each do |list|
			file.write(Output::format_method(@name, list))
		end
		file.write(Output::format_text(@text))
		children.sort.each do |child|
			child.write(file)
		end
	end
end

class ParameterEntry < Entry
end

class Path
	attr_reader :path
	def initialize(root)
		@path=[root]
	end

	def goto(name, search_entry)
		last_in_path=@path[-1]
		if last_in_path.children.include?(search_entry)
			entry=last_in_path.children[last_in_path.children.index(search_entry)]
			entry.reenter(name)
		else
			entry=search_entry
			last_in_path.children.push entry
		end
		@path.push(entry)	end

	def goto_file_level
		@path.remove_starting_at_class(FileEntry)
	end

	def goto_section_level
		@path.remove_starting_at_class(SectionEntry)
	end

	def goto_class_level
		@path.remove_starting_at_class(ModuleEntry)
		@path.remove_starting_at_class(ClassEntry)
	end

	def goto_method_level
		@path.remove_starting_at_class(MethodEntry)
	end

	def backup_to(path, class_type)
		while(@path.length>0 && @path[@path.length-1]!=class_type)
			@path=@path[0..-2]
		end
	end

	def deepest_node
		@path[-1]
	end

end

=begin
@class Dokumentat
The main class.
=end
class Dokumentat
	def initialize(project_name)
		say "Starting project #{project_name}"
		@project_name=project_name
		@project=ProjectEntry.new(@project_name)
		@root=RootEntry.new('root')
		@index_file=FileEntry.new('index')
		@root.children.push(@project)
		@project.children.push(@index_file)
		@matchers={
			:c => /^\w*\/\*\*\s*(.*?)\s*\*\//m,
			:ruby => /^=begin\s*(.*?)\s*^=end/m
		}
	end

	def process_dir(source_path)
		files=Dir[source_path]
		files.each do |file_name|
			path=Path.new(@root)
			path.goto('', @project)
			path.goto('', @index_file)
			process_file(file_name, path)
		end
	end

	def process_file(file_name, path)
		say "Processing file #{file_name}"
		File.open(file_name, 'r') do |file|
			commentmatcher=nil
			case file_name.downcase
				when /\.rbw?$/
					commentmatcher=@matchers[:ruby]
				when /\.c?$/
					commentmatcher=@matchers[:c]
				else
					say "Unknown extension"
					return
			end

			file.read.scan(commentmatcher)  do |block|
				lines=block.to_s.split("\n")
				section_mode=true
				text=""
				lines.each do |line|
					if line[0]!=?@
						section_mode=false
					end
					if section_mode
						type, name=(line.match /@([^ ]*) (.*)/)[1..2]
						case type.downcase
							when 'file'
								path.goto_file_level
								path.goto(name, FileEntry.new(name))
							when 'section'
								path.goto_section_level
								path.goto(name, SectionEntry.new(name))
							when 'method'
								path.goto_method_level
								path.goto(name, MethodEntry.new(name))
							when 'class'
								path.goto_class_level
								path.goto(name, ClassEntry.new(name))
							when 'module'
								path.goto_class_level
								path.goto(name, ModuleEntry.new(name))
							else
								say "Unknown section: #{line}"
						end
					else
						if line.strip.length==0
							text+="\n<p>"
						else
							text+=line+"\n"
						end
					end
				end
				text += "</p>\n\n"
				text.gsub! /([^@])@([\w?!=]+)/, '\1<b>\2</b>'
				text.gsub! /@@/, '@'
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

=begin
@method main
The main method.
=end
def main
	options = GetoptLong.new(
		["--project-name", 	"-p",	   GetoptLong::REQUIRED_ARGUMENT],
		["--output-dir",	"-o",	   GetoptLong::REQUIRED_ARGUMENT],
		["--verbose",		"-v",	   GetoptLong::NO_ARGUMENT ],
		["--extras-dir",	"-e",	   GetoptLong::REQUIRED_ARGUMENT]
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
			when '--extras-dir'
				$extras_dir=arg
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

main
