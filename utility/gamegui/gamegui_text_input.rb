=begin
@file gamegui
@class ImageSelector
Lets the user type some text. Note that it uses "edit_field.rb" from the utility directory.
=end
require 'RUDL'
include RUDL
require '../edit_field'

require  'gamegui_selector'

module GameGUI
	class TextInput < Selector
		def initialize(text, id, max_length, default_text)
			super(text, id)
			@max_length=max_length
			@default_text=default_text
		end
		
		def handle_keyboard_event(event)
			super(event)
			@field.process_event(event)
		end
		
		def handle_event(event)
			super(event)
			@field.process_event(event)
		end
		
		def on_enter
			super
			@field.enabled=true
			@field.reset_blink
		end
		
		def on_exit
			super
			@field.enabled=false
		end
		
		def screen=(screen)
			super(screen)
			w=@screen.area.right
			h=@screen.font.size('a').y
			@field=TTFEditField.new(@screen.font, [0,0, w,h], @screen.select_color, [0,0,0], @max_length)
			@field.set_colors(@screen.select_color, [50,50,0], @screen.color, true)
			@field.enabled=false
			@field.content=@default_text
		end

		def draw_variable(coordinate)
			@field.draw(@screen.display_surface, [coordinate.x, coordinate.y, 1000, 1000])
		end

		def value
			@field.content
		end
	end
end