=begin
@file Samples
@class MoveIt
(This was written by Nauglin)

This is a porting from the pygame script to RUDL. The original pygame 
code is at the end. I've found the RUDL version running at similar speed 
than the pygame one.

The main difference is that pygame seems to accept a Rect as a 
coordinate pair, and it also stores in the Rect object the right and 
bottom coordinates besides the left, top, with and height. This has 
forced me to do the GameObject.move method and the background 
restoring blit a bit different, as I must to calculate the rigth coordinate 
for each sprite on the fly every loop. I first tried to make the @pos 
instance variable look like a [left, top, width, height] array instead of 
a [left, top] coordinate pair, but the background restoring blit become 
a bit slower that way, becouse you have to get a slice from the @pos 
rect to obtain the position, two times. It seems to be faster adding two 
arrays than slicing one twice ?? 

I'm new to Ruby and RUDL, and indeed I'm new to OOP, so maybe 
someone more used to Ruby could try to improve all of this. If so, please 
let us see the resultant code.

NOTE: You'll need a 690x530x16bpp BMP for the script to work.
=end

#-- require and include everything
require 'RUDL'; include RUDL; include Constant

#-- our game object class
class GameObject
	attr_reader :pos, :image
	def initialize( image, height, speed )
		@speed = speed
		@image = image
		@pos = [ 0, height ]
		 #-- comment the above line and uncomment the next to try 
		 #-- the other way
		#@pos = [ 0, height, image.w, image.h ]
	end
	def move
		@pos[0] = @pos[0] + @speed
		if  @pos[0] + @image.w > 600
			@pos[0] = 0
		end
	end
end


#-- quick function to load an image
def load_image(name)
	return Surface.load_new( name ).convert()
end


#-- here's the full code
def main
	screen = DisplaySurface.new( [640, 480] )
	
	player = load_image('media/player1.gif')
	background = load_image('media/liquid.gif')  #-- YOU NEED SOME FILE HERE
	screen.blit( background, [0, 0] )
	
	objects = []
	for x in 0...10
		objects << GameObject.new( player, x*40, x )
	end

	while 1
		for event in EventQueue.get
			return if [QuitEvent, KeyDownEvent].include?( event.class )
		end

		for o in objects
			screen.blit( background, o.pos,  o.pos + o.image.size )
			 #-- comment the above line and uncomment the next to try 
			 #-- the other way
			#screen.blit( background, o.pos[0,2], o.pos )
		end
		for o in objects
			o.move
			screen.blit( o.image, o.pos )
			 #-- comment the above line and uncomment the next to try 
			 #-- the other way
			#screen.blit( o.image, o.pos[0,2] )
		end

        screen.update
	end
end



main


#---------------------------------------------------------------------
#---------------------------------------------------------------------
#---------------------------------------------------------------------

__END__

#!/usr/bin/env python

"""
This is the full and final example from the Pygame Tutorial,
"How Do I Make It Move". It creates 10 objects and animates
them on the screen.

Note it's a bit scant on error checking, but it's easy to read. :]
Fortunately, this is python, and we needn't wrestle with a pile of
error codes.
"""


#import everything
import os, pygame
from pygame.locals import *

#our game object class
class GameObject:
    def __init__(self, image, height, speed):
        self.speed = speed
        self.image = image
        self.pos = image.get_rect().move(0, height)
    def move(self):
        self.pos = self.pos.move(self.speed, 0)
        if self.pos.right > 600:
            self.pos.left = 0


#quick function to load an image
def load_image(name):
    path = os.path.join('data', name)
    return pygame.image.load(path).convert()


#here's the full code
def main():
    pygame.init()
    screen = pygame.display.set_mode((640, 480))

    player = load_image('player1.gif')
    background = load_image('liquid.bmp')
    screen.blit(background, (0, 0))

    objects = []
    for x in range(10):
        o = GameObject(player, x*40, x)
        objects.append(o)

    while 1:
        for event in pygame.event.get():
            if event.type in (QUIT, KEYDOWN):
                return

        for o in objects:
            screen.blit(background, o.pos, o.pos)
        for o in objects:
            o.move()
            screen.blit(o.image, o.pos)

        pygame.display.update()



if __name__ == '__main__': main()
