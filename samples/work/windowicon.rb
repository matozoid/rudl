
require "RUDL"
include RUDL
include Constant


# usage: windowicon.rb [iconfile [transparentcolor]]

colorkey = nil

if ARGV.size > 0
    iconfile = ARGV[0]
    colorkey = eval("["+ARGV[1]+"]") if ARGV.size > 1
else
    iconfile = "apple.bmp"
    colorkey = [0,0,255]
end

icon = Surface.load_new(iconfile)
icon.set_colorkey(colorkey) if colorkey


win = DisplaySurface.new [300,200]
win.set_icon(icon)

win.fill [160,160,160]
win.blit(icon, [134,84])
win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent or ev.is_a? MouseButtonDownEvent
end
