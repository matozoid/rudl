
require "RUDL"
include RUDL
include Constant


# open a window
window = DisplaySurface.new([400,300])

# set window title
window.set_caption "Empty RUDL program"

loop do
    # wait for an input event
    ev = EventQueue.wait

    # exit if the window was closed
    break if ev.is_a? QuitEvent
end

