#!/usr/local/bin/ruby

# *** STOLEN FROM PYGAME SAMPLES ***

# A simple starfield example. Note you can move the 'center' of
# the starfield by leftclicking in the window. This example show
# the basics of creating a window, simple pixel plotting, and input
# event management"""


require 'RUDL'
include RUDL
include Constant

#constants
$winsize= [640, 480]
$wincenter = [320, 240]

class Star
	def initialize
		dir = rand(100000)
		velmult = rand(0)*.6+.4
		@dx=Math.sin(dir) * velmult
		@dy=Math.cos(dir) * velmult
		@x=$wincenter[0]
		@y=$wincenter[1]
		@time=0
	end

	def draw(surface)
		color=[@time,@time,@time]
		surface.filled_circle([@x,@y], @time/100, color)
	end

	def move
		@x += @dx
		@y += @dy
		@time+=3
		if @x<0 || @x>$winsize[0] || @y<0 || @y>$winsize[1]
			Star.new
		else
			@dx *= 1.05
			@dy *= 1.05
			self
		end
	end
end

class Stars
	def initialize(num_stars)
		srand
		@stars = []
		num_stars.times do
			@stars.push(Star.new)
		end
	end

	def move
		@stars.collect! {|star|
			star.move
		}
	end

	def draw(surface)
		@stars.each {|star|
			star.draw(surface)
		}
	end
end

#create our starfield
stars=Stars.new(150)

#initialize and prepare screen
display= DisplaySurface.new($winsize, FULLSCREEN|DOUBLEBUF|HWSURFACE)
display.set_caption 'RUDL Stars Example'

# main loop

done = false
while !done do
	display.fill([0,0,0])
	stars.move
	stars.draw(display)
	display.antialiased_circle(Mouse.pos, 5, [255,0,0])
	display.flip

	while event=EventQueue.poll
		case event
			when QuitEvent,KeyDownEvent
				done = true
			when MouseButtonDownEvent
				$wincenter = event.pos
		end
	end
end
