
# this example could use some more rewriting, and the guidelines
# presented here belong in the main documentation :)


require 'RUDL'
# we would usually "include RUDL" and "include Constant" to avoid
# having to write RUDL:: or RUDL::Constant:: everywhere

puts <<_end_
Events example
--------------

Read the source to see a normal RUDL input handler loop.

There are two ways of handling input: events or polling.
Events means monitoring the changes in the keyboard or mouse's state.
Polling means reading the state of keys or the mouse, when you need it.

Events are the preferred method, because when using polling you can miss
keypresses or mouse clicks completely if you don't poll often enough,
and the slower the computer is the greater the risk. It is very annoying
to the user and makes the application seem unresponsive. When using
events, it is impossible or at least very difficult to miss clicks and
keypresses.

This program will now print to the console all events that the window
receives.
_end_

# There has to be a RUDL::DisplaySurface for most events to fire
RUDL::DisplaySurface.new([150,100], RUDL::Constant::RESIZABLE)

# This outer loop simulates your game loop.
# Event handling is done in a loop within the game loop.
loop do

    # RUDL::EventQueue is the main event system. It contains a queue with events
    # that have happened. You can get events from it in several ways. EventQueue.get
    # returns an array with all the unprocessed events. EventQueue.wait pauses the
    # program until an event occurs, and returns that event. EventQueue.poll gets
    # the next unprocessed event or nil if there aren't any. (Note: don't confuse
    # this with the polling mentioned above. Polling for events is preferable to
    # polling for keyboard/mouse state. Unless you busy loop that is, see below.)

    # Normally, you'll want to process all events, and then proceed to draw the
    # next frame.

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

    # Now that we've processed the events, we should draw the next frame of animation
    # or whatever the game is doing.


    # Important! Do not busy poll! This means that if there aren't any events you just
    # check the events again without any delay. This WILL use 100% CPU time, even when
    # the program is idle! But even a delay of 1 millisecond is enough to reduce CPU
    # usage to 0% on a modern computer. 10 milliseconds of delay means checking for
    # events 100 times a second, this should be enough for anything. In this example,
    # we have no smooth animation to run or anything, so we'll check the events only
    # 20 times a second. (1000/50 = 20)

    RUDL::Timer.delay(50)

    # If this wasn't a simulation of a game's event loop, we'd use EventQueue.wait
    # instead of EventQueue.get since we have nothing to do when there are no events.
    # Then the delay wouldn't be needed.

    # If you're running full screen, DisplaySurface#flip will wait for the next screen
    # refresh (vertical blanking period); this will limit your loop to about 85 times
    # a second or whatever refresh rate the user has. In that case, no extra delay is
    # needed. But in a window, DisplaySurface#flip will not wait and you should do your
    # own delays. This is a problem for very smooth animation (e.g. scrolling), since
    # that would require precise synchronization with the monitor's refresh rate.

end
