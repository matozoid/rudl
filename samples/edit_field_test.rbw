require 'RUDL'
require '../utility/edit_field.rb'

include RUDL
include Constant

done=false

display=DisplaySurface.new [600,300]

Key.set_repeat(300,50)

# Comment out another one of these to use another font type:

# SDL_GFX 8x8 font:
#editfield=EditField.new [10,10,200,10]

# Truetype font:
editfield=TTFEditField.new TrueTypeFont.new('media/polarbear.ttf', 45), [10,10,500,50]

# SFont bitmap font:
#editfield=SFontEditField.new SFont.new(Surface.load_new('media/24p_copperplate_blue.png')), [10,10,500,100]

while !done

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
	editfield.process_event event

	display.fill [100,200,0]
	editfield.draw display
	display.flip
end
