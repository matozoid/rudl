require "RUDL"
include RUDL
include Constant

win = DisplaySurface.new [300,200]

win.set_icon(Surface.load_new("myicon.bmp"))

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent or ev.is_a? MouseButtonDownEvent
end
