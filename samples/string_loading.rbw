#! /usr/bin/env ruby

require 'RUDL'; include RUDL

display=DisplaySurface.new([640,480])
#
# Alright:
# File <= class for file access
# .new <= start doing something with a file
# ('media/bounce.bmp', <= filename. The seperator (slash) could have been more crossplatform.
# 'rb') <= access mode. when not specified it's 'r'. We need the 'b' for binary access, or Ruby will stop
#	reading on the first EOF character (which is supposed to be normal for text access)
# .read <= read everything in the file into a string
# .to_surface <= interpret the data in the string as an image in some format, then convert it to a surface.
#
# This example adds nothing to what Surface.load_new already does. You could use it when the bmp files
# aren't in seperate files, but stuck together in some archive file. You read the archive file, get the bmp files
# in memory only, then convert them to surfaces with to_surface.
#
ball=File.new('media/bounce.bmp','rb').read.to_surface
lastrect=rect=[0,0,0,0]

while true
	if EventQueue.poll.class==QuitEvent then exit end
	pos=[(Timer.ticks%1024-512).abs, ((Timer.ticks+200)%704-352).abs]
        display.update([(rect=display.blit(ball, pos)).union(lastrect)])
	display.fill([0, 0, 0], lastrect=rect)
end
