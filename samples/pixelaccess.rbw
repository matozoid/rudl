#!/usr/bin/env ruby
require '../RUDL'; include RUDL

display=DisplaySurface.new([320,200])

while true do
	event=EventQueue.poll
	case event
		when MouseMotionEvent
			display.plot(event.pos, [255,255,255])
			# Does the same:
			# display[event.pos[0], event.pos[1]]=[255,255,255]
			display.update
		when QuitEvent, KeyDownEvent, MouseButtonDownEvent
			exit
	end
end
