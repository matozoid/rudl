#!/usr/local/bin/ruby

# $Log: makedocs.rb,v $
# Revision 1.7  2003/10/05 16:17:14  tsuihark
# Improved the docs script a bit
#
# Revision 1.6  2003/10/05 14:01:00  tsuihark
# Finally did something about the ugly docs
#
# Revision 1.5  2003/10/05 13:56:52  tsuihark
# Finally did something about the ugly docs
#

require 'ftools'

puts 'This creates the RUDL documentation.'
puts 'Use it on Windows.'

sourcefiles=Dir['../rudl*.c']+Dir['*.rd2'].sort

def adjust_level_to_level(current_level, wanted_level)
	retval=''
	while current_level<wanted_level
		retval+='<ul>'
		current_level+=1
	end
	while current_level>wanted_level
		retval+='</ul>'
		current_level-=1
	end
	retval
end

sourcefiles.each {|source|
	basename=File.basename(source, '.c')
	destination="#{basename}.html"
	puts basename
	contents=`rd2.bat #{source}` # Changing rd2.bat to rd2 will probably make it work on Unix.
	if !contents.index('DONT_GENERATE_RD2_DOCS') && contents.index('label-0')
		# Index creation
		index="<h1>Index</h1>\n"
		current_level=0
		last_label=''
		contents.scan(/(<h(.?)>)?<a name="(label-\d+)" id="label-\d+">(.*)<\/a>(<\/h.?>)?/) { |blah, level, id, label|
			label.gsub!(/>\w*?[#.]/, '>') # Take out all Music# and Sound. prefixes
			if label!=last_label
				level='3' if !level
				level=level.to_i
				index+=adjust_level_to_level(current_level, level)+
						"<li><a href='#{destination}##{id}'>#{label}</a>\n"
				current_level=level
				last_label=label
			end
		}
		index+=adjust_level_to_level(current_level, 0)
		
		#
		contents.gsub! /<title>Untitled<\/title>/, "<title>#{basename}<\/title>"
		contents.gsub! /<head>/, "<head>\n<link rel=stylesheet type='text/css' href='rudl.css'>"
		contents.gsub! /<body>/, "<body>\n<a href='index.html'>Back to index</a>\n#{index}"
		contents.gsub! /<\/body>/, "<a href='index.html'>Back to index</a>\n</body>"

		contents=contents.split("\n")
		contents.delete_at(0) # Delete stupid empty line
		contents.delete_at(0) # Delete stupid commandline echo?!?
		
		#
		File.open(destination, 'w') do |file|
			file.write(contents.join("\n"))
		end
	else
		puts "ignored"
	end
}
