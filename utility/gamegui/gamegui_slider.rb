=begin
@file gamegui
@class Slider
<b>Not written yet</b> Lets the user select a value on a slider, like volume.
=end
module GameGUI
	class Slider < Selector
		def initialize(text, id, min, max, default, step)
			super(text, id)
			@min=min
			@max=max
			@current=default
			@step=step
		end
		
		def draw_variable(pos)
			current_coordinate=@screen.right_column_width*((@current-@min*1.0)/(@max-@min))/2
			@screen.display_surface.fill(@screen.select_color, [pos.x, pos.y+@screen.row_height*0.2, current_coordinate, @screen.row_height*0.8])
			@screen.display_surface.fill(@screen.color, [pos.x+current_coordinate, pos.y+@screen.row_height*0.2, (@screen.right_column_width/2)-current_coordinate, @screen.row_height*0.8])
		end

		def handle_keyboard_event(event)
			super(event)
			case event
				when KeyDownEvent
					case event.key
						when Constant::K_LEFT
							@current=@current-@step
							@current=@min if @current<@min
						when Constant::K_RIGHT
							@current=@current+@step
							@current=@max if @current>@max
						when Constant::K_END
							@current=@max
						when Constant::K_HOME
							@current=@min
					end
			end
		end
		
		def value
			@current
		end
	end
end
