require 'ftools'
require 'rbconfig'
include Config

version=CONFIG['MAJOR'].to_i*100+CONFIG['MINOR'].to_i*10+CONFIG['TEENY'].to_i

puts 'UGLY RUDL ON WINDOWS INSTALLER'
puts 'Will copy the RUDL library to the Ruby tree.'
if version<166
	puts 'Lower versions are not supported because of an incompatible base system.'
	puts 'RUDL is compiled with Microsoft Visual C, and the resulting library is'
	puts 'not compatible with the GCC compiled version of Ruby that was used before'
	puts 'version 166-0.'
	puts "\nYour version seems to be too old!"
	puts "\nYou can get a newer one at"
	puts 'http://www.pragmaticprogrammer.com/ruby/downloads/ruby-install.html'
else
	puts "\nContinue? (y/n)"

	if gets.upcase[0]==?Y
		
		destdir="#{CONFIG['archdir']}#{CONFIG['target_prefix']}"
		puts "Destination: #{destdir}"

		puts 'Making directory if necessary...'
		File::makedirs(CONFIG['rubylibdir'], destdir)

		
		puts "Installing RUDL.so..."
		File::install('RUDL.so', destdir+'/RUDL.so')
		
		puts 'done!'

		paths={}
		letter='0'
		paths[letter.succ!]=CONFIG['bindir']

		begin
			paths[letter.succ!]='c:\windows\system' if File.stat('C:\windows\system').directory?
		rescue
		end

		begin
			paths[letter.succ!]='c:\winnt\system' if File.stat('C:\winnt\system').directory?
		rescue
		end

		paths[letter.succ!]='somewhere else'
		somewhere_else=letter.dup
		paths[letter.succ!]='don''t copy dll''s right now'
		dont_copy=letter.dup

		puts "\nBecause RUDL depends on a lot of DLL's, they will have to be"
		puts "installed somewhere where they can be found."
		puts "Where do you want them?"
		
		done=false
		chosen_path=nil
		while !done
			paths.sort.each do |letter, path|
				puts "#{letter} => #{path}"
			end

			done=true
			chosen_path_key=gets[0].chr
			if chosen_path_key==dont_copy
				#
			elsif chosen_path_key==somewhere_else
				puts "Type a path:"

			chosen_path=gets.chop
				begin
					done=File.stat(chosen_path).directory?
				rescue
					done=false
				end
				puts "doesn't exist" if !done
			elsif paths[chosen_path_key]
				chosen_path=paths[chosen_path_key]
			else
				done=false
			end
		end
		
		if chosen_path
			puts "Installing DLL's..."
			Dir.chdir('dll')
			Dir['*'].each do |filename|
				if (!File.stat(filename).directory?) &&
						File.stat(filename).readable?
					File::install(filename, chosen_path+'/'+filename)
				end
			end
			Dir.chdir('..')
		end
	end
end

puts "\nDone!\n[ENTER]"

gets
