# package.rb
# ----------
# This version selects files by exclusion!

# todo: don't exclude anything from samples/ ?

# by Rennex on 24 Sep 2003


# settings
$destdir = "rudl_packages"
$version = "0.7"

# print warning to fend off mugglers
puts "This will create a RUDL package."
puts "It is only used by developers on the RUDL project."
puts


#--------------
# main program
#--------------

def main
    # What shall we package?
    case ARGV[0]
        when /source|src/i
            zipit("source")

        when /setup/i
            zipit("setup")

        else
            zipit("setup")
            zipit("source")
    end

    puts "Done!"
end


# create either the source or setup archive
def zipit(what)

    # files to always exclude
    ignoretypes = [ /\.(obj|def|exp|pdb|lib)$/i, /^makefile$/i, /^mkmf.log$/i,
                    /^package.*\.rb$/i, /^#$destdir\//i ]

    # list everything
    files = Dir["**/*"]

    # exclude directory names and dirs named "CVS" (courtesy of WinCVS)
    files.reject! {|f|
        File.stat(f).directory? or f =~ %r{(^|/)CVS(/|$)}i
    }

    # helper method to exclude the files that we never want to include
    def files.reject_list!(ary)
        self.reject! {|f|
            # got to use this trick because "return true" exits the whole method
            found = ary.each do |re|
                break "exclude" if f =~ re
            end
            found == "exclude"
        }
    end

    # exclude the defaults
    files.reject_list!(ignoretypes)

	# which archive are we creating?
	case what
		when "source"
		files.reject_list! [
				/^dll\//i,
				/^lib\//i,
				/\.dll$/i,
				/^RUDL.so$/i 
		]
	
	when "setup"
		files.reject_list! [ 
				/^include\//i, 
				/^lib\//i, 
				/\.(c|h)$/i,
				/^configure$/i, 
				/^make.bat$/i, 
				/^extconf.rb$/i 
		]

        else
            raise "Oh God please no!"
    end

    outfile = "#$destdir/rudl-#$version-#{what}.zip"

    Dir.mkdir($destdir) unless File.exists?($destdir)
    File.delete(outfile) if File.exists?(outfile)

    # I hope the command line doesn't get truncated :)
    puts "Zipping #{outfile}..."
    files.each do |filename|
    	system("zip -9 -q #{outfile} \"#{filename}\"")
    end
end

main

