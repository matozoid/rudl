puts '* Setting up for compilation'

require 'mkmf'

#uncomment next line if you don't know why things don't work
#$DEBUG=true

#When compiling under no cygwin Windows, quote your command line options!


sdl_config = with_config("sdl-config", "sdl-config")

dir_config('mixer')
dir_config('image')
dir_config('ttf')
dir_config('pthread')
dir_config('sdl')

mswin32=/mswin32/ =~ RUBY_PLATFORM
cygwin=/cygwin/ =~ RUBY_PLATFORM
windows=mswin32||cygwin
unix=!windows

$CFLAGS+="-Wall " if cygwin

if unix
	$CFLAGS += `#{sdl_config} --cflags`.chomp
	$LDFLAGS += `#{sdl_config} --libs`.chomp
end

puts '* Checking for optional files'
have_library('pthread')
have_header('SDL_mixer.h') if have_library('SDL_mixer','Mix_OpenAudio')
have_header('SDL_image.h') if have_library('SDL_image','IMG_Load')
have_header('SDL_ttf.h') if have_library('freetype','FT_Init_FreeType') and have_library('SDL_ttf','TTF_Init')

puts '* Checking for required files'
if have_library('SDL', 'SDL_Quit') and 
		have_library('SDLmain') and 
		have_header('SDL.h')

	create_makefile('RUDL')
	puts '* Done! You may now run make (nmake if you are using VC)'
else
	puts '* Core SDL libraries could not be found...'
end
