require "RUDL"
include RUDL
include Constant

win = DisplaySurface.new([300,200])

loop do
    event = EventQueue.wait
    break if event.is_a? QuitEvent
end
