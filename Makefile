SHELL = /bin/sh

#### Start of system configuration section. ####

srcdir = .
topdir = /usr/local/lib/ruby/1.6/i486-linux
hdrdir = /usr/local/lib/ruby/1.6/i486-linux
VPATH = $(srcdir)

CC = gcc

CFLAGS   = -fPIC -g -O2 -Wall -I/usr/local/include -I/usr/local/include/SDL -D_REENTRANT
CPPFLAGS = -I$(hdrdir) -I/usr/local/include -DHAVE_SDL_MIXER_H -DHAVE_SDL_IMAGE_H -DHAVE_SDL_TTF_H -DHAVE_SDL_H  
CXXFLAGS = $(CFLAGS)
DLDFLAGS =  -L$(topdir) -L/usr/local/lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL -lpthread
LDSHARED = gcc -shared 
LIBPATH = 

RUBY_INSTALL_NAME = ruby
RUBY_SO_NAME = 

prefix = $(DESTDIR)/usr/local
exec_prefix = $(DESTDIR)/usr/local
libdir = $(DESTDIR)/usr/local/lib/ruby/1.6
archdir = $(DESTDIR)/usr/local/lib/ruby/1.6/i486-linux
sitelibdir = $(DESTDIR)/usr/local/lib/ruby/site_ruby/1.6
sitearchdir = $(DESTDIR)/usr/local/lib/ruby/site_ruby/1.6/i486-linux

#### End of system configuration section. ####

LOCAL_LIBS =  
LIBS = $(LIBRUBY_A) -lSDLmain -lSDL -lSDL_ttf -lfreetype -lSDL_image -lSDL_mixer -lpthread -lc
OBJS = SDL_gfxPrimitives.o SDL_rotozoom.o SFont.o rudl.o rudl_audio.o rudl_cdrom.o rudl_events.o rudl_joystick.o rudl_keyboard.o rudl_mouse.o rudl_rect.o rudl_sfont.o rudl_timer.o rudl_ttf.o rudl_video.o SDLMappy.o rudl_mappy.o

TARGET = RUDL
DLLIB = $(TARGET).so

RUBY = ruby
RM = $(RUBY) -r ftools -e 'File::rm_f(*Dir[ARGV.join(" ")])'

EXEEXT = 

all:		$(DLLIB)

clean:;		@$(RM) *.o *.so *.sl *.a $(DLLIB)
		@$(RM) $(TARGET).lib $(TARGET).exp $(TARGET).ilk *.pdb

distclean:	clean
		@$(RM) Makefile extconf.h conftest.*
		@$(RM) core ruby$(EXEEXT) *~

realclean:	distclean

install:	$(archdir)/$(DLLIB)

site-install:	$(sitearchdir)/$(DLLIB)

$(archdir)/$(DLLIB): $(DLLIB)
	@$(RUBY) -r ftools -e 'File::makedirs(*ARGV)' $(libdir) $(archdir)
	@$(RUBY) -r ftools -e 'File::install(ARGV[0], ARGV[1], 0555, true)' $(DLLIB) $(archdir)/$(DLLIB)

$(sitearchdir)/$(DLLIB): $(DLLIB)
	@$(RUBY) -r ftools -e 'File::makedirs(*ARGV)' $(libdir) $(sitearchdir)
	@$(RUBY) -r ftools -e 'File::install(ARGV[0], ARGV[1], 0555, true)' $(DLLIB) $(sitearchdir)/$(DLLIB)


.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
$(DLLIB): $(OBJS)
	$(LDSHARED) $(DLDFLAGS) -o $(DLLIB) $(OBJS) $(LIBS) $(LOCAL_LIBS)
