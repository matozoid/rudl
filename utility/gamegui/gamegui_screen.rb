=begin
@file gamegui
@class Screen
A screen defines a list of so called selectors.
Selectors are the items you can go through, like "start," "quit," "name."
A screen takes a bunch of them and is able to "run" them, showing them
on screen, letting the user go through them and use them.
=end
module GameGUI
	class Screen
		attr_accessor :display_surface
		attr_accessor :area, :font, :color, :select_color, :selectors
		attr_reader :left_column_width, :right_column_width
		attr_reader :row_height
		attr_accessor :pre_draw, :post_draw
		attr_accessor :on_start, :on_quit
		attr_reader :selectors_by_id
		
		def initialize(display_surface, area, font, color, select_color, selectors)
			@display_surface=display_surface
			@area=area
			@font=font
			@color=color
			@select_color=select_color
			@selectors=selectors
			@left_column_width=0
			@selectors_by_id={}
			self.current_selector_index=0
			
			@selectors.each do |selector|
				selector.screen=self
				@left_column_width=selector.w if @left_column_width<selector.w
			end
			@left_column_width+=10
			@right_column_width=@area.w-@left_column_width
		end
		
		def current_selector_index=(new_index)
			@selectors[@current_selector_index].on_exit if @current_selector_index
			@current_selector_index=new_index
			@selectors[@current_selector_index].on_enter
		end
		
		def select_next
			new_index=(@current_selector_index+1)%@selectors.length
			self.current_selector_index=new_index
		end
		
		def select_previous
			new_index=(@current_selector_index-1)%@selectors.length
			self.current_selector_index=new_index
		end

		def quit
			@quit=true
		end

		def draw
			@pre_draw.call(self) if @pre_draw
			current_selector=0.0
			@row_height=@area.h/@selectors.length
			@selectors.each do |selector|
				selector.draw([@area.x, @area.y+(@row_height)*current_selector])
				current_selector+=1
			end
			@post_draw.call(self) if @post_draw
		end
		
		def update
			@selectors.each do |selector|
				selector.update
			end
		end
        
		def run
			@quit=false
			@on_start.call(self) if @on_start
			begin
				while !@quit
					EventQueue.get.each do |event|
						case event
							when QuitEvent
								return
							
							when KeyDownEvent
								case event.key
									when Constant::K_ESCAPE
										return
									else
										@selectors[@current_selector_index].handle_keyboard_event(event)
							end
							
							else
								@selectors[@current_selector_index].handle_event(event)
						end
						
					end
	
					update
					draw
					@display_surface.flip
				end
			ensure
				@on_quit.call(self) if @on_quit
			end
		end
	end
end