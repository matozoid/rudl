=begin
---------------------------------------------------------------------
This is a porting from the pygame script to RUDL. The original pygame 
code is at the end, followed by the BlitzBasic source from where the 
pygame was implemented. The Ruby version seems to be visibly slower 
than the Python one, mainly (I guess) because of the two nested 
"stepped" loops. The liquid.py runs at 20-25 fps on my duron-600, 
while the liquid.rb runs at 13-16 fps. I think this slow down has 
nothing to do with the RUDL wrap, but with Ruby itself. Probably the 
'polling' has part in the guilt, but the 'peeking' method doesn't 
work as expected on RUDL - it just return whether there is any event 
awaiting, not the event itself.

Maybe someone more used to Ruby could try to accelerate the loops. 
If so, please let us see the resultant code.

NOTE: You'll need a 690x530x16bpp BMP for the script to work.

   [- nauglin -]
---------------------------------------------------------------------
=end

require 'RUDL'; include RUDL; include Constant


#-- initialize and setup screen
screen = DisplaySurface.new [640,480], HWSURFACE|DOUBLEBUF, 8

#-- load image. Rename the file or change the name here
bitmap = Surface.load_new 'media/liquid.gif'

#-- get the image and screen in the same format
if screen.bitsize == 8
	screen.set_palette 0, bitmap.palette
else
	bitmap=bitmap.convert
end

#-- prep some variables
anim = 0.0

#-- this is for testing fps
STEP = 20
x = y = fps = 0
t = Timer.ticks

#-- mainloop
while not [QuitEvent, KeyDownEvent, MouseButtonDownEvent].include?( EventQueue.poll.type )
	anim += 0.4
	0.step(639, STEP) { |x|
		0.step(379, STEP) { |y|
			xpos = (x + (Math.sin(anim + x * 0.01) * 15)) + STEP
			ypos = (y + (Math.sin(anim + y * 0.01) * 15)) + STEP
			screen.blit bitmap, [x, y], [xpos, ypos, STEP, STEP]
		}
	}
	screen.flip
	fps += 1
end

#-- get the fps speed result
t = Timer.ticks - t
screen.print [10,10], "#{fps/(t/1000)} FPS", [255,0,0]
screen.flip

#-- just a wait event before quitting
true while EventQueue.poll
true until [QuitEvent, KeyDownEvent].include? EventQueue.poll.class


#---------------------------------------------------------------------
#---------------------------------------------------------------------
#---------------------------------------------------------------------

__END__


"""This examples demonstrates a simplish water effect of an
image. It attempts to create a hardware display surface that
can use pageflipping for faster updates. Note that the colormap
from the loaded GIF image is copied to the colormap for the
display surface.

This is based on the demo named F2KWarp by Brad Graham of Freedom2000
done in BlitzBasic. I was just translating the BlitzBasic code to
pygame to compare the results. I didn't bother porting the text and
sound stuff, that's an easy enough challenge for the reader :]"""

import pygame, os
from pygame.locals import *
from math import sin


#initialize and setup screen
pygame.init()
screen = pygame.display.set_mode((640, 480), HWSURFACE|DOUBLEBUF, 8)

#load image
imagename = os.path.join('data', 'liquid.bmp')
bitmap = pygame.image.load(imagename)

#get the image and screen in the same format
if screen.get_bitsize() == 8:
    screen.set_palette(bitmap.get_palette())
else:
    bitmap = bitmap.convert()

#prep some variables
anim = 0.0

fps = 0
t = pygame.time.get_ticks()

#mainloop
while not pygame.event.peek([QUIT, KEYDOWN, MOUSEBUTTONDOWN]):
    anim = anim + 0.4
    for x in range(0, 640, 20):
        for y in range(0, 480, 20):
            xpos = (x + (sin(anim + x * .01) * 15)) + 20
            ypos = (y + (sin(anim + y * .01) * 15)) + 20
            screen.blit(bitmap, (x, y), (xpos, ypos, 20, 20))
    pygame.display.flip()
    fps = fps + 1

t = pygame.time.get_ticks() - t
print fps/(t/1000)



"""BTW, here is the code from the BlitzBasic example this was derived
from. This code runs a little faster, yet reads a lot slower. again
i've snipped the sound and text stuff out.
-----------------------------------------------------------------
; Brad@freedom2000.com

; Load a bmp pic (800x600) and slice it into 1600 squares
Graphics 640,480					
SetBuffer BackBuffer()				
bitmap$="f2kwarp.bmp"					
pic=LoadAnimImage(bitmap$,20,15,0,1600)

; use SIN to move all 1600 squares around to give liquid effect
Repeat
f=0:w=w+10:If w=360 Then w=0
For y=0 To 599 Step 15
For x = 0 To 799 Step 20
f=f+1:If f=1600 Then f=0
DrawBlock pic,(x+(Sin(w+x)*40))/1.7+80,(y+(Sin(w+y)*40))/1.7+60,f 
Next:Next:Flip:Cls
Until KeyDown(1)
"""
