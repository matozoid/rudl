=begin
(adapted from a Pygame demo example thing)
Name:unlimited 'bobs' - korruptor, http://www.korruptor.demon.co.uk
Desc:

I first saw this in the Dragons MegaDemo on the A500 back in '89 and it blew me away.
How gutted I was when Wayne "tripix" Keenan told me how easy it was and knocked up an
an example demo a couple of years later.

For those of you that didn't know how it was done, here's a pygame example. It's
basically a flick-book effect; you draw the same sprite in different positions on
25 different 'screens' and flick between them. When you've drawn on all 25 you loop
back to the beginning and keep on blitting.

Sprite offsets make it look like you're adding sprites. Simple.
=end

require 'RUDL'
include RUDL
include Constant
include Math

# ------------------------------------------------------------------------------------

RES = [640,480]
PI = 3.14159
DEG2RAD = PI/180

# ------------------------------------------------------------------------------------
def main
	surfaces= []
    
	# Initialise pygame, and grab an 8bit display.
	screen= DisplaySurface.new RES,FULLSCREEN, 8
	bob= Surface.load_new 'media/bob.gif'
       
        # load a sprite and set the palette
        bob.set_colorkey [255,255,255]
        screen.set_palette 0, bob.palette

        # Create 25 blank surfaces to draw on.
        (0..25).each do |i|
		surfaces.push(Surface.new(RES,0,8))
		surfaces[i].set_palette 0, bob.palette
	end
        xang    = 0.0
        yang    = 0.0
        surf    = 0
       
        # Fruity loops...
        while true
   
		# Have we received an event to quit the program?
		EventQueue.get.each do |event|
			if [QuitEvent, KeyDownEvent, MouseButtonDownEvent].include? event.class
				exit
			end
		end

		# Get some x/y positions
		x = (RES[0]/2)*sin((xang*DEG2RAD)*0.75)
		y = (RES[1]/2)*cos((yang*DEG2RAD)*0.67)
 
		# Inc the angle of the sine
		xang += 1.17
		yang += 1.39
       
		# blit our 'bob' on the 'active' surface
		surfaces[surf].blit(bob,[x+(RES[0]/2)-32,y+(RES[1]/2)-32])
           
		# blit the active surface to the screen
		screen.blit(surfaces[surf],[0,0])

		# display the results
		screen.flip

		# inc the active surface number
		surf = (surf+1) % 25
	end
end

main

# ------------------------------------------------------------------------------------

# End of sauce. Pass the chips...
