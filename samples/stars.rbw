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
$numstars = 10

class Star
	def initialize
		dir = rand(100000)
		velmult = rand(0)*.6+.4
		@vel = [Math.sin(dir) * velmult, Math.cos(dir) * velmult]
		@pos=$wincenter
	end

	def draw(surface, color)
		surface.plot(@pos, color)
	end

	def move
		@pos[0] += @vel[0]
		@pos[1] += @vel[1]
		if @pos[0]<0 || @pos[0]>$winsize[0] || @pos[1]<0 || @pos[1]>$winsize[1]
			return false
		else
			@vel[0] = @vel[0] * 1.05
			@vel[1] = @vel[1] * 1.05
			return true
		end
	end
end

def initialize_stars
	stars = []
	(0..$numstars).each {|x|
		star=Star.new
		vel, pos = star
		steps = 2
		#pos[0] = pos[0] + (vel[0] * steps)
		#pos[1] = pos[1] + (vel[1] * steps)
		#vel[0] = vel[0] * (steps * .09)
		#vel[1] = vel[1] * (steps * .09)
		stars.push star
	}
	move_stars(stars)
	stars
end

def draw_stars(surface, stars, color)
	stars.each {|star|
		star.draw(surface, color)
	}
end

def move_stars(stars)
	p stars
	stars.collect! {|star|
		if !star.move
			Star.new
		else
			star
		end
	}
	p stars
	exit
end	

#create our starfield
srand
$stars = initialize_stars

#initialize and prepare screen
display= DisplaySurface.new($winsize)
display.set_caption 'pygame Stars Example'
white = [255, 240, 200]
black = [20, 20, 40]
display.fill(black)

# main loop

done = false
while !done do
#	p $stars
	draw_stars(display, $stars, black)
	move_stars($stars)
	draw_stars(display, $stars, white)
	display.flip

	event=EventQueue.poll
	case event
		when QuitEvent
			done = true
		when MouseButtonDownEvent
			$wincenter = event.pos
	end
end
