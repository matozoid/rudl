#!/usr/bin/env ruby

require '../RUDL'; include RUDL; include Constant

display=DisplaySurface.new([320,240], 0, 32)

# originalhond is in a weird format, so...
originalhond=Surface.load_new('media/hond.bmp')

# ... I create this empty surface with an alpha channel (that's 0xFF000000)...
hond=Surface.new(originalhond.size, 0, 32, [0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000])

# ... and blit hond into it.
hond.blit(originalhond, [0,0])

rotatedhond=nil
i=100
dest=[0,0]

while true
	# The last "true" is for antialiasing. Try setting it to false.
	rotatedhond=hond.rotozoom(Math.sin(i/100.0)*100+100, (Math.cos(i/67.0)+1)*0.9, true)
	dest[0]=(320-rotatedhond.w)/2+Math.sin(i/30.0)*20
	dest[1]=(240-rotatedhond.h)/2+Math.cos(i/44.0)*15
	display.fill([0,0,0])
	display.blit(originalhond, [0,0])
	display.blit(rotatedhond, dest)
	display.rectangle([dest[0], dest[1], rotatedhond.w, rotatedhond.h], 0xFFFFFFFF)
	display.flip
	rotatedhond.destroy
	exit if EventQueue.poll.type==KeyDownEvent
	i=i+1
end
