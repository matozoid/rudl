#!/usr/local/bin/ruby

# $Log: koeitest.rb,v $
# Revision 1.2  2003/10/05 10:53:20  tsuihark
# Ready for packaging
#

require 'koei'
include Koei
require 'RUDL'
include RUDL
include Constant

$display=DisplaySurface.new [320,240]

main_window=Window.new("Testjeuh") {

	@rect=[10,10,300,150]
	@status_bar=	StatusBar.new {
		@resize_thing=ResizeBitton.new
	}

	@left_bar=	ScrollBar.new	{ @alignment=Koei::Left|Koei::Top|Koei::Bottom }
	@right_bar=	ScrollBar.new
	@left_panel=	LoweredPanel.new {
=begin
		@p1=RaisedPanel.new {
			on_mouse_down.add proc {|pos, button|
				@color1=[@color1[0]+button,0,0]
				set_image_dirty
				true
			}
			@labbie=Label.new("porneaux")
		}
		@p2=RaisedPanel.new
=end
			@labbie=Label.new("spam")
	}
	@right_panel=	LoweredPanel.new
}

p main_window

done=false

while !done

	$display.fill [100,200,0]

	begin
		event=EventQueue.poll
		case event
			when QuitEvent
				done=true
			when KeyDownEvent
				case event.key
					when K_ESCAPE
						done=true
				end
		end
		main_window.process_event event
	end while event

	main_window.draw $display
	$display.flip
end
