# package.rb
# ----------
# This version selects files by inclusion!

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


# recursively list the contents of the dir, ignoring the
# dir names and dirs named "CVS" (silly WinCVS..?)
def lsdir(name)
    if name
        name += "/" unless name =~ /\/$/
        name += "**/*"
    else
        name = "**/*"
    end
    files = Dir[name]
    files.reject! {|f|
        File.stat(f).directory? or f =~ %r{(^|/)CVS(/|$)}i
    }
    return files
end


# create either the source or setup archive
def zipit(what)

    # which archive are we creating?
    case what
        when "source"
            files = lsdir("docs") + lsdir("include") +
                    lsdir("samples") + lsdir("utility") +
                    Dir["*.{c,h,txt,rb}"] << "configure" << "make.bat"
            files.delete("package.rb")

        when "setup"
            files = lsdir("dll") + lsdir("docs") +
                    lsdir("samples") + lsdir("utility") +
                    Dir["*.txt"] << "RUDL.so" << "install-on-windows.rb"

        else
            raise "Oh God please no!"
    end

    filelist = '"' + files.join('" "') + '"'
    outfile = "#$destdir/rudl-#$version-#{what}.zip"

    Dir.mkdir($destdir) unless File.exists?($destdir)
    File.delete(outfile) if File.exists?(outfile)

    # I hope the command line doesn't get truncated :)
    puts "Zipping #{outfile}..."
    system("zip -9 -q #{outfile} " + filelist)
end

main

