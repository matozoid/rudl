require 'RUDL'

puts "Events example\n"
puts "Read the source to see a normal RUDL eventloop."
puts "There are two ways of handling input: polling or events."
puts "Polling means looking at a piece of hardware when you feel like it."
puts "Events means that whatever happens with the hardware is stored and can"
puts "be retrieved even after it has happened."
puts "For example: you could poll every now and then whether a key is pressed,"
puts "or you can ask RUDL for the events every now and then, and watch for a"
puts "keyboard event.\n"
puts "You will see a window appearing. It will print all events that it gets to"
puts "the console."

# There has to be a RUDL::DisplaySurface for most events to fire
RUDL::DisplaySurface.new([100,100])

# This outer "while true do" simulates your game loop. Event handling is done in a loop within the game loop.
while true do
	# This loop is taken from Crapola.rb

	# RUDL::EventQueue is the main event system. It contains a queue with events that have happened.
	# You can get events from it in several ways. Normally, you want to get the next event to handle, and
	# you do this with poll, which returns the next event, or nil if there was no event.
	while event=RUDL::EventQueue.poll
		# a case statement is a comfortable way to decide what to do with these events. In this case
		# statement, we are testing the class of the event.
		case event
			# Seems that the event's class is RUDL::QuitEvent, which is fired when the window is closed.
			when RUDL::QuitEvent
				puts "QuitEvent received, exiting"
				exit
			when RUDL::KeyDownEvent
				case event.key
					when RUDL::Constant::K_ESCAPE
						puts "KeyDownEvent with key set to K_ESCAPE received, exiting"
						exit
					else
						p event
				end
			else
				p event
		end
	end
end