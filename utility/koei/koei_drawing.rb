# $Log: koei_drawing.rb,v $
# Revision 1.2  2003/10/05 10:53:20  tsuihark
# Ready for packaging
#

module Koei
	module Drawing

		attr_accessor :surface
		attr_accessor :color1, :color2, :color3

		def set_drawing_defaults
			if @parent
				@color1=parent.color1
				@color2=parent.color2
				@color3=parent.color3
			else
				@color1=Default.color1
				@color2=Default.color2
				@color3=Default.color3
			end

			@surface=nil
		end

		def right;		@rect[0]+@rect[2];end
		def left;		@rect[0];end
		def top;		@rect[1];end
		def bottom;		@rect[1]+@rect[3];end
		def top_left;		[left, top];end
		def top_right;		[right, top];end
		def bottom_right;	[right, bottom];end
		def bottom_left;	[left, bottom];end
		def w;			@rect[2];end
		def h;			@rect[3];end

		def right=(n);		@rect[2]=n-@rect[0];end
		def left=(n);		@rect[0]=n;end
		def top=(n);		@rect[1]=n;end
		def bottom=(n);		@rect[3]=n-@rect[1];end
		def w=(n);		@rect[2]=n;end
		def h=(n);		@rect[3]=n;end

		def draw_raised_border(color)
			@surface.vertical_line([0,0], h-3, 0xFFFFFF80)
			@surface.horizontal_line([0,0], w-3, 0xFFFFFF80)
			@surface.vertical_line([w-2,1], h-2, 0x00000080)
			@surface.horizontal_line([1,h-2], w-2, 0x00000080)
			@surface.vertical_line([w-1,0], h-1, 0x000000E0)
			@surface.horizontal_line([0,h-1], w-1, 0x000000E0)
		end

		def draw_lowered_border(color)
			@surface.vertical_line([0,0], h-3, 0x00000080)
			@surface.horizontal_line([0,0], w-3, 0x00000080)
			@surface.vertical_line([w-1,1], h-1, 0xFFFFFF80)
			@surface.horizontal_line([1,h-1], w-1, 0xFFFFFF80)
		end


		# DRAWING

		def redraw_internal(surface, dirty_size)
			draw_self
			children.each {|child|
				child.draw @surface, dirty_size
			}
		end

		def draw(surface, dirty_size=false)
			dirty_size=true if @dirty_size

			@dirty_image=true if dirty_size

			resize_internal if dirty_size
			redraw_internal(@surface, dirty_size) if @dirty_image

			@dirty_image=false
				
			surface.blit @surface, top_left if w>0 && h>0
		end

		def draw_self
		end
	end
end