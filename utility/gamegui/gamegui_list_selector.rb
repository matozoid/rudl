=begin
@file gamegui
@class ListSelector
Lets the user pick an item from a list of strings.
=end
module GameGUI
	class ListSelector < Selector
		attr_accessor :list, :current
		def initialize(text, id, list)
			super(text, id)
			@list=list
			@current=0
		end
		
		def draw_variable(coordinate)
			@screen.display_surface.blit(@currently_selected_text, coordinate)
		end

		def change_current_item
			@currently_selected_text=@screen.font.render('< '+@list[@current].to_s+' >', true, @screen.color)
		end
		
		def screen=(screen)
			super(screen)
			change_current_item
		end
		
		def handle_keyboard_event(event)
			super(event)
			case event
				when KeyDownEvent
					case event.key
						when Constant::K_LEFT
							@current=(@current-1)%@list.length
							change_current_item
						when Constant::K_RIGHT
							@current=(@current+1)%@list.length
							change_current_item
					end
			end
		end
		
		def value
			@list[@current].to_s
		end
	end
end
