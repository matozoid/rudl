require 'RUDL'; include RUDL; include Constant

# Here is an example of how you can use the event system.
# It might not be the best ruby code ever written, but, just as the rest of the stuff in this
# directory, it's there to be copied and modified, or just as inspiration.

# EventMapper maps the events that EventQueue generates to handlers that you assign.
class EventMapper
	def initialize
		@event_map={
			QuitEvent=>proc {},
			KeyDownEvent=>proc {},
			QuitEvent=>proc {},
			KeyDownEvent=>proc {},
			KeyUpEvent=>proc {},
			ActiveEvent=>proc {},
			MouseMotionEvent=>proc {},
			MouseButtonDownEvent=>proc {},
			MouseButtonUpEvent=>proc {},
			JoyAxisEvent=>proc {},
			JoyBallEvent=>proc {},
			JoyHatEvent=>proc {},
			JoyButtonUpEvent=>proc {},
			JoyButtonDownEvent=>proc {},
			ResizeEvent=>proc {},
			TimerEvent=>proc {},
			EndOfMusicEvent=>proc {}
		}
	end
	# distribute processes all events in the queue and calls any assigned eventhandlers
	def distribute
		event=EventQueue.poll
		case event
			when QuitEvent
				@event_map[QuitEvent].call
			when KeyDownEvent
				@event_map[KeyDownEvent].call(event.key, event.mod, event.unicode)
			when KeyUpEvent
				@event_map[KeyUpEvent].call(event.key, event.mod, event.unicode)
			when ActiveEvent
				@event_map[ActiveEvent].call(event.gain, event.state)
			when MouseMotionEvent
				@event_map[MouseMotionEvent].call(event.pos, event.rel, event.button)
			when MouseButtonDownEvent
				@event_map[MouseButtonDownEvent].call(event.pos, event.button)
			when MouseButtonUpEvent
				@event_map[MouseButtonUpEvent].call(event.pos, event.button)
			when JoyAxisEvent
				@event_map[JoyAxisEvent].call(event.id, event.value, event.axis)
			when JoyBallEvent
				@event_map[JoyBallEvent].call(event.id, event.ball, event.rel)
			when JoyHatEvent
				@event_map[JoyHatEvent].call(event.id, event.hat, event.value)
			when JoyButtonUpEvent
				@event_map[JoyButtonUpEvent].call(event.id, event.button)
			when JoyButtonDownEvent
				@event_map[JoyButtonDownEvent].call(event.id, event.button)
			when ResizeEvent
				@event_map[ResizeEvent].call(event.size)
			when TimerEvent
				@event_map[TimerEvent].call(event.id)
			when EndOfMusicEvent
				@event_map[EndOfMusicEvent].call
		end
	end
	# Attach takes the name of an eventclass (like TimerEvent) and attaches the supplied block
	# to it. Next time the event is encountered in "EventMapper.distribute", this block will be called.
	# Only the last assigned block is called.
	def attach(event_classname, &block)
		@event_map[event_classname]=block	end
end
# --------------- end of recyclable code

# --------------- start of example/test code
# Events won't be handled without a display surface.
$d=DisplaySurface.new [100,100], RESIZABLE

event_mapper=EventMapper.new

# This happens when the close button on the window is pressed
event_mapper.attach(QuitEvent) {
	puts "bye!"
	exit
}

# This happens when a key is pressed.
event_mapper.attach(KeyDownEvent) { |key, mod, unicode| 
	# Show a few modifiers
	mods=''
	mods+='left shift ' if(mod & KMOD_LSHIFT)>0
	mods+='right shift ' if(mod & KMOD_RSHIFT)>0
	mods+='control ' if(mod & KMOD_CTRL)>0
	# key gets over 255 when pressing a few modifiers. I don't know what it is, but I just
	# AND it away :-)
	# Unicode is defined far far away.
	puts "#{(key&0xff).chr} (#{mods}) unicode: #{unicode}" 
}

# This happens when a mouse button is pressed.
event_mapper.attach(MouseButtonDownEvent) { |pos,button|
        puts "pos "+pos.inspect+" button "+button.inspect
}


# This happens when you resize the window. You need to pass "RESIZABLE" to
# DisplaySurface.new to make the window resizable at all.
event_mapper.attach(ResizeEvent) { |new_size|
	puts "Window now #{new_size.join('x')}"
	# Reallocate the display surface at the new size
	$d=DisplaySurface.new new_size, RESIZABLE
}

# Keep on distributing events forever. The QuitEvent block will exit the program.
loop do 
	event_mapper.distribute 
	Timer.delay(10)
end
