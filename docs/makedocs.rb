#!/usr/local/bin/ruby

require 'ftools'

puts 'This creates the RUDL documentation.'
puts 'You can make it create documentation in another directory by giving that directory as an argument to this program.'
puts 'This will also create some fake 0 bytes documentation for files that had no documentation in them.'

prefix='.'

prefix=ARGV[0] if ARGV.length>0

sourcefiles=Dir['../rudl*.c']

sourcefiles.each {|source|
	basename=File.basename(source, '.c')
	destination="#{prefix}/#{basename}.html"
	puts basename
	#`rd2 #{source} > #{destination}`
}

File.copy 'index.html', prefix
File.copy 'howto.html', prefix

