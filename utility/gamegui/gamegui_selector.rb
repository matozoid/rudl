=begin
@file gamegui
@class Selector
The base class for all selectors, this should be treated as abstract and never instantiated.
=end
module GameGUI
	class Selector
		attr_accessor :text, :screen, :enabled
		attr_accessor :on_exit, :on_enter, :on_key
		attr_accessor :selected
	
		def initialize(text, id)
			@text=text
			@id=id
			@screen=nil
			@enabled=true
			@selected=false
		end
		
		def on_exit
			@selected=false
		end
		
		def on_enter
			@selected=true
		end
		
		def draw(coordinate)
			draw_text(coordinate)
			draw_variable([coordinate.x+@screen.left_column_width, coordinate.y])
		end
		
		def draw_text(coordinate)
			if @selected
				@screen.display_surface.blit(@selected_rendered_text, coordinate)
			else
				@screen.display_surface.blit(@rendered_text, coordinate)
			end
		end
		
		def draw_variable(coordinate)
		end
		
		def w
			@screen.font.size(@text).x
		end
		
		def screen=(screen)
			@screen=screen
			@rendered_text=@screen.font.render(@text, true, @screen.color)
			@selected_rendered_text=@screen.font.render(@text, true, @screen.select_color)
			@screen.selectors_by_id[@id]=self
		end
		
		def update
		end
		
		def handle_keyboard_event(event)
			case event
				when KeyDownEvent
					case event.key
						when Constant::K_DOWN
							@screen.select_next
						when Constant::K_UP
							@screen.select_previous
						when Constant::K_RETURN
							on_select
					end
			end
		end
		
		def handle_event(event)
		end
		
		def on_select
		end
		
		def value
			nil
		end
	end
end
