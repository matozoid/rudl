require 'RUDL'
# we would usually "include RUDL" and "include Constant" to avoid
# having to write RUDL:: in front of RUDL classes.

puts
puts "Events example"
puts "--------------"
puts
puts "Read the source to see a normal RUDL eventloop."
puts
puts "There are two ways of handling input: polling or waiting."
puts "Polling means checking if events have occurred, when you feel like it."
puts "Waiting means that the program is paused until some event occurs."
puts
puts "For example: you could poll every now and then whether a key is pressed,"
puts "and in the meanwhile keep running your animation on screen."
puts "Or you can wait for any event to occur if there's nothing else to do."
puts
puts "You will see a window appearing. It will print all events that it gets to"
puts "the console."

# There has to be a RUDL::DisplaySurface for most events to fire
RUDL::DisplaySurface.new([150,100])

# This outer loop simulates your game loop.
# Event handling is done in a loop within the game loop.
while true

    # RUDL::EventQueue is the main event system. It contains a queue with events that
    # have happened. You can get events from it in several ways. EventQueue.poll gets
    # the next unprocessed event or nil if there aren't any. EventQueue.get returns
    # an array with all the unprocessed events.

    # Normally, you'll want to process all events after you've drawn a frame,
    # and then draw the next frame.

    RUDL::EventQueue.get.each do |event|

        # A case statement is an elegant way to decide what to do with these events.
        # In this case statement, we are testing the class of the event.
        case event
            when RUDL::QuitEvent
                # Seems that the event's class is RUDL::QuitEvent,
                # which is fired when the window is closed.
                puts "QuitEvent received, exiting"
                exit

            when RUDL::KeyDownEvent
                # A key was pressed down; now let's process which key it was
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

    # Important! Do not busy poll! This means that if there aren't any events you just
    # check the events again without any delay. This WILL use 100% CPU time, even when
    # the program is idle! But even a delay of 1 millisecond is enough to reduce CPU
    # usage to 0% on a modern computer. 10 milliseconds of delay means checking for
    # events 100 times a second, this should be enough for anything. In this example,
    # we have no smooth animation to run or anything, so we'll check the events only
    # 20 times a second. (1000/50 = 20)

    RUDL::Timer.delay(50)

    # If you're running full screen, DisplaySurface#flip will wait for the next screen
    # refresh (vertical blanking period); this will limit your loop to about 85 times
    # a second or whatever refresh rate the user has. In that case, no extra delay is
    # needed. But in a window, DisplaySurface#flip will not wait and you should do your
    # own delays. This is a problem for very smooth animation (e.g. scrolling), since
    # that would require precise synchronization with the monitor's refresh rate.

end