
# Work in progress

module RUDL

	# Sprite is a basic sprite class. It can include modules or add extras that will give
	# it more functionality. Build your own new fancy Sprite class!
	class Sprite
		attr_accessor :x, :y
		attr_accessor :surface

		def tick
			@extras.each do |extra|
				extra.tick
			end
		end
		
		def draw(display)
			display.blit(@surface, [@x,@y])
		end

		def erase(display)
		end

		def move(dx, dy)
			@x+=dx
			@y+=dy
		end
		
		def w
			return @surface.w if @surface
			nil
		end
		
		def h
			return @surface.h if @surface
			nil
		end
		
		def rect
			[@x,@y,@w,@h]
		end
		
		def rect=(r)
			@x=r.x
			@y=r.y
			@w=r.w
			@h=r.h
		end
		
		def pos
			[@x,@y]
		end
		
		def pos=(p)
			x,y=p
		end
		
		def area_collision_check(other_sprite)
		end
	end

	module BackgroundHandling
		module Paste
			attr_accessor :background
			def draw(display)
			end
			def erase(display)
			end
		end
	
		module Save
			def draw(display)
			end
			def erase(display)
			end
		end
	end

	module PixelPerfectCollisionDetection
		def surface=(surface)
			if !surface.collision_map
				surface.collision_map=CollisionMap.new(surface)
			end
			@surface=surface
		end
	end

	class Animation
		attr_reader :surfaces

		def initialize(new_surfaces)
			surfaces=new_surfaces
		end

		def surfaces=(surfaces)
			@surfaces=surfaces
			if included_modules.include?(PixelPerfectCollisionDetection)
				@surfaces.each do |surface|
					if !surface.collision_map
						surface.collision_map=CollisionMap.new(surface)
					end
				end
			end
		end

	end

	class AnimationPlayer
		attr_accessor :frame, :speed, :animation

		def initialize(animation, speed=1)
			@animation=animation
			@speed=speed
			reset
		end

		def reset
			@frame=0.0
		end

		def start(speed=1)
			@speed=speed.to_f
		end

		def stop
			@speed=0.0
		end

		def tick
			@frame=@frame+@speed
			@surface=@animation.surfaces[@frame.to_i]
		end
	end

	module Extra

		# Module Movement gives automatic movement to a sprite by updating
		# the x and y coordinates with speed dx and dy.
		class Movement
			attr_accessor :dx, :dy

			def initialize(dx, dy)
				@dx=dx
				@dy=dy
			end

			def tick
				@x+=@dx
				@y+=@dy
			end
		end

		# Module Border defines actions to be taken when a sprite is on the
		# border of an area
		class Border
			Types=[:bounce, :disappear, :stop, :wrap]
			attr_accessor :type

			def initialize(type)
				@type=type
			end

			def tick
				
				case @type
					when :bounce
					when :disappear
					when :stop
					when :wrap
					else
						raise "Unknown type for sprite's Border!"
				end
			end
		end

		# Module Acceleration changes speed dx, dy by ax, ay, resulting in
		# acceleration of the sprite
		class Acceleration
	
			attr_accessor :ax, :ay
	
			def initialize(ax, ay)
				@ax=ax
				@ay=ay
			end
	
			def tick
				@dx+=@ax
				@dy+=@ay
			end
		end
	end
end
