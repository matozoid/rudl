#!/usr/bin/env ruby
require "RUDL"
include RUDL

win = DisplaySurface.new([320,200])

while true
    event = EventQueue.wait

    case event
        when MouseMotionEvent
            win.plot(event.pos, [255,255,255])
            # you can also plot a white pixel with win[x,y] = [255,255,255]
            # or win[event.pos] = [255,255,255]
            win.update

        when MouseButtonDownEvent
            win.fill [0,0,0]
            win.update

        when QuitEvent, KeyDownEvent
            exit
    end

end
