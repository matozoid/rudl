#!/usr/bin/env ruby

puts 'This is not supposed to work'

require '../RUDL'; include RUDL; include Constant
display=DisplaySurface.new([320, 240])

map=Mappy.new('test.fmp').reset_animations

x=0
y=0

while true
	event=EventQueue.poll
	case event
		when QuitEvent, MouseButtonDownEvent
			exit
	end
	pressed=Key.pressed?
	
	exit if pressed[K_ESCAPE]
	x=x-1 if pressed[K_LEFT] && x>0
	x=x+1 if pressed[K_RIGHT] && x<32*30
	y=y-1 if pressed[K_UP] && y>0
	y=y+1 if pressed[K_DOWN] && y<80

	display.line([0,0],[200,200],0xFFFFFFFF)
	map.draw_background(display, [x,y], [x,y,200,200])
	display.flip
end

