
require "RUDL"
include RUDL
include Constant

icon = Surface.load_new("apple.bmp")

win = DisplaySurface.new [300,200]
icon.set_colorkey [0,0,255]
win.set_icon(icon)

win.fill [160,160,160]
win.blit(icon, [134,84])
win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent or ev.is_a? MouseButtonDownEvent
end
