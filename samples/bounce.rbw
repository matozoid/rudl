#! /usr/bin/env ruby

require 'RUDL'; include RUDL

display=DisplaySurface.new([640,480])
ball=Surface.load_new('media/bounce.bmp')
lastrect=rect=[0,0,0,0]

while true
	if EventQueue.poll.class==QuitEvent then exit end
	pos=[(Timer.ticks%1024-512).abs, ((Timer.ticks+200)%704-352).abs]
        display.update([(rect=display.blit(ball, pos)).union(lastrect)])
	display.fill([0, 0, 0], lastrect=rect)
end
