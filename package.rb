# package.rb
# ----------

# todo: tar.gz for sources

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
    what = ARGV[0]

    # if the first argument is "list", only list the files to be zipped
    if what =~ /list/i
        what = ARGV[1]
        $list = true
    end

    # What shall we package?
    case what
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
# dir names and dirs named "CVS"
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


# helper method to exclude the files that we don't want to include
class Array
    def reject_list!(ary)
        self.reject! {|f|
            # got to use this break trick because "return true" exits the whole method
            found = ary.each do |re|
                if re.is_a? Regexp
                    # why does Regexp.new(re, Regexp::IGNORECASE) ignore my flags?
                    break "exclude" if f.downcase =~ re
                else
                    # compare strings as case-insensitive
                    break "exclude" if f.downcase == re.downcase
                end
            end
            found == "exclude"
        }
    end
end


# create either the source or setup archive
def zipit(what)

    # which archive are we creating?
    case what
        when "source"
            files = lsdir("docs") + lsdir("include") +
                    lsdir("samples") + lsdir("utility") +
                    Dir["*.{c,h,txt,rb}"] << "configure" << "make.bat"

        when "setup"
            files = lsdir("dll") + lsdir("docs") +
                    lsdir("samples") + lsdir("utility") +
                    Dir["*.txt"] << "RUDL.so" << "install-on-windows.rb"

        else
            raise "Oh God please no!"
    end

    files.reject_list! [
        /\.(obj|def|exp|pdb|lib)$/,
        "makefile",
        "mkmf.log",
        /^package.*\.rb$/,
        $destdir,
        /^samples\/work\//,
    ]

    if not $list
        outfile = "#$destdir/rudl-#$version-#{what}.zip"

        Dir.mkdir($destdir) unless File.exists?($destdir)
        File.delete(outfile) if File.exists?(outfile)

        # zip em up! one at a time, to avoid truncated command lines
        puts "Zipping #{outfile}..."
        files.each do |filename|
            system("zip -9 -q #{outfile} \"#{filename}\"")
        end
    else
        puts files
    end
end

main

