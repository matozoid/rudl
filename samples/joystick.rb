#!/usr/bin/env ruby
require '../RUDL'
include RUDL

display=DisplaySurface.new([100,100])

if Joystick.count==0
	puts 'No joysticks, no point'
	exit
end

j=Joystick.new(0)
done=false

puts 'This part shows joystick zero\'s axes. Press a button to end.'
while !done
	event=EventQueue.poll
	case event
		when JoyAxisEvent
			display[event.value*49+50, event.axis*5]=[255,255,255]
			display.flip
		when JoyButtonDownEvent
			done=true
		when QuitEvent
			exit
	end
end

done=false
puts 'This part shows joystick zero\'s x and y axis. Press button 1 to end.'
while !done
	display[j.axis(0)*49+50, j.axis(1)*49+50]=[255,255,0]
	display.flip
	done=j.button(1)
	EventQueue.pump
end
