#!/usr/local/bin/ruby

# $Log: makedocs.rb,v $
# Revision 1.5  2003/10/05 13:56:52  tsuihark
# Finally did something about the ugly docs
#

require 'ftools'

puts 'This creates the RUDL documentation.'
puts 'You can make it create documentation in another directory by giving that directory as an argument to this program.'
puts 'This will also create some fake 0 bytes documentation for files that had no documentation in them.'
puts 'Use it on Windows.'

sourcefiles=Dir['../rudl*.c']+Dir['*.rd2'].sort

sourcefiles.each {|source|
	basename=File.basename(source, '.c')
	destination="#{basename}.html"
	puts basename
	contents=`rd2.bat #{source}` # Changing rd2.bat to rd2 will probably make it work on Unix.
	if !contents.index('DONT_GENERATE_RD2_DOCS') && contents.index('label-0')
		index="<h1>Index</h1>\n"
		contents.scan(/<a name="(label-\d+)" id="label-\d+">(.*)<\/a>/) { |id, label|
			index+="<br><a href='#{destination}##{id}'>#{label}</a>\n"
		}
		
		contents.gsub! /<title>Untitled<\/title>/, "<title>#{basename}<\/title>"
		contents.gsub! /<head>/, "<link rel=stylesheet type='text/css' href='rudl.css'>"
		contents.gsub! /<body>/, "<body>\n<a href='index.html'>Back to index</a>\n#{index}"
		contents.gsub! /<\/body>/, "<a href='index.html'>Back to index</a>\n</body>"

		contents=contents.split("\n")
		contents.delete_at(0) # Delete stupid empty line
		contents.delete_at(0) # Delete stupid commandline echo?!?
		File.open(destination, 'w') do |file|
			file.write(contents.join("\n"))
		end
	else
		puts "ignored"
	end
}
