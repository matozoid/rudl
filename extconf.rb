require 'mkmf'

#uncomment next line if you don't know why things don't work
#$DEBUG=true

sdl_config = with_config("sdl-config", "sdl-config")

windows=/mswin32|cygwin/ =~ RUBY_PLATFORM

$CFLAGS+="-Wall "

if !windows then
	$CFLAGS += `#{sdl_config} --cflags`.chomp
	$LDFLAGS += `#{sdl_config} --libs`.chomp
end

have_library('pthread')
have_header('SDL_mixer.h') if have_library('SDL_mixer','Mix_OpenAudio')
have_header('SDL_image.h') if have_library('SDL_image','IMG_Load')
have_header('SDL_ttf.h') if have_library('freetype','FT_Init_FreeType') and have_library('SDL_ttf','TTF_Init')

if have_library('SDL', 'SDL_Quit') and 
		have_library('SDLmain') and 
		have_header('SDL.h') then

	create_makefile('RUDL')
end
