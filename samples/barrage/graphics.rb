# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
# See comments in barrage.rbw

require 'RUDL';  include RUDL;  include Constant

class Graphics

  def initialize sz, full_screen, caption
    @sz = sz
    @full_screen = full_screen
    @caption = caption
    graphics_setup
  end

    def overlay
        @display
    end
    
    def copy
        @display.blit $backbuffer, [0,0]
    end
  
    def flip
        @display.flip
    end

  def shutdown
    $backbuffer.destroy
    $background_buffer.destroy
    @display.destroy
  end

  private
  
  def graphics_setup
    $backbuffer.destroy if $backbuffer
    $background_buffer.destroy if $background_buffer
    @display.destroy if @display
    
    flags = HWSURFACE | DOUBLEBUF
    flags |= FULLSCREEN if @full_screen
    @display = DisplaySurface::new @sz, flags, 32
    #~ Mouse::visible = false  # Really slows down my machine, I think it's a problem with my drivers or video card
    @display.set_caption @caption
    $backbuffer = Surface::new @sz, @display
    $backbuffer.fill [0,0,0]
    $background_buffer = Surface::new @sz, @display
    $background_buffer.fill [0,0,0]
  end

end