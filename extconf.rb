# $Log: extconf.rb,v $
# Revision 1.19  2004/05/13 12:51:08  rennex
# Added more spaces around compiler flags, just in case
#
# Revision 1.18  2003/11/28 22:24:58  rennex
# Fixed bugs that caused errors on Linux.
#
# Revision 1.17  2003/09/26 22:59:45  tsuihark
# Fixed damn EOLs and old SGE reference.
#
# Revision 1.16  2003/09/26 21:00:24  rennex
# Me too, testing that is! ;)
#
# Revision 1.15  2003/09/26 20:52:29  tsuihark
# testing keywords
#
puts '* Setting up for compilation'

require 'mkmf'

#uncomment next line if you don't know why things don't work
#$DEBUG=true

#When compiling under plain Windows, double quote your command line options!

# Why isn't this in mkmf.rb?
def have_flag(flag, default)
    return false if arg_config("--no-#{flag}")
    return false unless val = arg_config("--#{flag}", default)
    flag = "-D" << flag.tr('-a-z', '_A-Z')
    flag << "=#{val}" if String === val
    $defs.push(flag)
    true
end

sdl_config = with_config("sdl-config", "sdl-config")

debug_version=have_flag('debug', false)

if debug_version
    puts '*** DEBUG VERSION ***'
    $defs.push('-DDEBUG_RUDL')
else
    puts 'for a debug version, pass --debug to extconf.rb'
end

dir_config('mixer')
dir_config('image')
dir_config('ttf')
dir_config('pthread')
dir_config('sdl')
dir_config('gfx')
dir_config('smpeg')
dir_config('net')

mswin32=/mswin32/ =~ RUBY_PLATFORM
cygwin=/cygwin/ =~ RUBY_PLATFORM
windows=mswin32||cygwin
unix=!windows

$CFLAGS+=" -funroll-loops " if !mswin32

$CFLAGS+=" -Wall " if unix||cygwin
$CPPFLAGS+=" -GX " if mswin32

if unix
    $CFLAGS += " " + `#{sdl_config} --cflags`.chomp
    $LDFLAGS += " " + `#{sdl_config} --libs`.chomp
end

puts '* Checking for optional files'

puts ' - smpeg, video playing library'
have_header('smpeg/smpeg.h') if have_library('smpeg')

if !windows
    puts ' - pthreads, multithreading library (not used in RUDL, but has to be linked if SDL uses it)'
    have_library('pthread')
end

puts ' - Drawing: SDL_gfx from http://www.ferzkopp.net/'
have_header('SDL_gfxPrimitives.h') and
    have_header('SDL_framerate.h') and
#   have_header('SDL_gfxPrimitives_font.h') and # Don't seem to be using this.
    have_header('SDL_imageFilter.h') and
    have_header('SDL_rotozoom.h') if have_library('SDL_gfx')

puts ' - Multi-channel audio mixer library: SDL_mixer from http://www.libsdl.org/projects/SDL_mixer/'
have_header('SDL_mixer.h') if have_library('SDL_mixer','Mix_OpenAudio')

puts ' - More than just a lousy BMP reader: SDL_image from http://www.libsdl.org/projects/SDL_image/'
have_header('SDL_image.h') if have_library('SDL_image','IMG_Load')

puts ' - Truetype fonts: SDL_ttf from http://www.libsdl.org/projects/SDL_ttf/'
have_header('SDL_ttf.h') if have_library('freetype','FT_Init_FreeType') and have_library('SDL_ttf','TTF_Init')

#puts ' - Flexible networking: SDL_net from http://www.libsdl.org/projects/SDL_net'
#have_header('SDL_net.h') if have_library('SDL_net', 'SDLNet_Init')

if debug_version
    puts ' - Memory checking: Fortify from http://www.geocities.com/SiliconValley/Horizon/8596/'
    puts '    (Make sure fortify.c is in this directory before running extconf.rb)'
    $defs.push('-DFORTIFY') if have_header('fortify.h')
end

puts '* Checking for required files'
puts ' - SDL from http://www.libsdl.org/download-1.2.html'
if have_library('SDL', 'SDL_Quit') and have_header('SDL.h')
    create_makefile('RUDL')
    puts '* Done! You may now run make.'
else
    puts '* Core SDL libraries could not be found...'
end
