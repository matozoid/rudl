#!/usr/bin/env ruby

#
# * Copyright (c) 1993-1997, Silicon Graphics, Inc.
# * ALL RIGHTS RESERVED 
# * Permission to use, copy, modify, and distribute this software for 
# * any purpose and without fee is hereby granted, provided that the above
# * copyright notice appear in all copies and that both the copyright notice
# * and this permission notice appear in supporting documentation, and that 
# * the name of Silicon Graphics, Inc. not be used in advertising
# * or publicity pertaining to distribution of the software without specific,
# * written prior permission. 
# *
# * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
# * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
# * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
# * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
# * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
# * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
# * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
# * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
# * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
# * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
# * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
# * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
# * 
# * US Government Users Restricted Rights #

# * Use, duplication, or disclosure by the Government is subject to
# * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
# * (c)(1)(ii) of the Rights in Technical Data and Computer Software
# * clause at DFARS 252.227-7013 and/or in similar or successor
# * clauses in the FAR or the DOD or NASA FAR Supplement.
# * Unpublished-- rights reserved under the copyright laws of the
# * United States.  Contractor/manufacturer is Silicon Graphics,
# * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
# *
# * OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
# */
#
#
# This code was created by Jeff Molofee '99
# (ported to SDL by Sam Lantinga '2000)
#
# If you've found this code useful, please let me know.
#
# Visit me at www.demonews.com/hosted/nehe 
#
##################################################################### 
# Port of NeHe's OpenGL tutorials to Ruby (http://www.ruby-lang.org)
# by Martin Stannard (martin@optushome.com.au)
# 
# Requires the following Ruby libraries:
#
# opengl : http://www2.giganet.net/~yoshi/
# RUDL : http://froukepc.dhs.org/rudl/index.html
# 
# lesson09.rbw :
#
# stars 
# 
# */

require 'RUDL'
include RUDL
require "opengl"
require 'glut'
require "rational"

include Constant

class Star

	attr_accessor :dist, :angle, :r, :g, :b, :index

	def initialize(anIndex, aDist)
	  	@dist = aDist
		@index = anIndex
		@angle = 0.0
		@r = rand(256)
		@g = rand(256)
		@b = rand(256)
  	end
end      
$StarNum = 50
$Stars = []
$Textures = []

tilt = 90.0		# tilt the view
z = -15.0 		# depth into the screen.
spin = 15.0
spininc = 0.01

# white ambient light at half intensity (rgba) 
$LightAmbient = [0.5, 0.5, 0.5, 1.0]

# super bright, full intensity diffuse light.
$LightDiffuse = [1.0, 1.0, 1.0, 1.0]

# position of light (x, y, z, (position of light)) 
$LightPosition = [0.0, 0.0, 2.0, 1.0]

# rotation angles
xrot, yrot = 0.0, 0.0

$Filter = 0

ScreenWidth, ScreenHeight = 640, 480

STDOUT.sync = TRUE # flush all output to screen immediately

$d = DisplaySurface.new([ScreenWidth, ScreenHeight], OPENGL|DOUBLEBUF) #|FULLSCREEN)

# Load Bitmaps And Convert To Textures
def loadGLTextures
	# Load Texture
	raw_image = Surface.load_new("Data09/Star.bmp")
	if (raw_image.nil?)
		SDL_Quit
		exit (1)
	end
	# convert bmp from BGR to RGB - no BGR mask on Windows
    image = Surface.new(raw_image.size, SWSURFACE, 24, [0x000000FF, 0x0000FF00, 0x00FF0000, 0])
    image.blit(raw_image, [0,0])
	
	# Create Texture   
	$Textures = GL.GenTextures(1);

	# texture 0 (linear scaling)
	GL.BindTexture(GL::TEXTURE_2D, $Textures[0]);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::LINEAR);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::LINEAR);
	GL.TexImage2D(GL::TEXTURE_2D, 0, 3, image.w, image.h, 0, GL::RGB, GL::UNSIGNED_BYTE, image.pixels)
	
end

def init(w,h)
  	loadGLTextures
	GL.ClearColor(0.0, 0.0, 0.0, 0.0)
	GL.ClearDepth(1.0)
	GL.DepthFunc(GL::LESS)
	#GL.Enable(GL::DEPTH_TEST)
	GL.ShadeModel(GL::SMOOTH)
   
	GL.MatrixMode(GL::PROJECTION)
	GL.LoadIdentity()
   
	GLU.Perspective(45.0, w.to_f/h.to_f, 0.1, 100.0)
	GL.MatrixMode(GL::MODELVIEW)
	GL.LoadIdentity()
	GLU.LookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
	# set up light number 1.
    GL.Light(GL::LIGHT0, GL::AMBIENT, $LightAmbient)  # add lighting. (ambient)
    GL.Light(GL::LIGHT0, GL::DIFFUSE, $LightDiffuse);  # add lighting. (diffuse).
    GL.Light(GL::LIGHT0, GL::POSITION, $LightPosition); # set light position.
    GL.Enable(GL::LIGHT0);                             # turn light 1 on.
	# setup blending 
    GL.BlendFunc(GL::SRC_ALPHA,GL::ONE);			# Set The Twinkleing Function For Translucency
	GL.Enable(GL::BLEND)
    GL.Color4f(1.0, 1.0, 1.0, 0.5);    
	# set up the stars 
    $StarNum.times do |i|
		$Stars.push(Star.new(i, i.to_f / $StarNum * 5.0))
	end
end

display = Proc.new {
	GL.Clear(GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT) # Clear The Screen And The Depth Buffer

	GL.Enable(GL::TEXTURE_2D);
	#GL.TexEnvf(GL::TEXTURE_ENV, GL::TEXTURE_ENV_MODE, GL::DECAL);
	GL.BindTexture(GL::TEXTURE_2D, $Textures[0]);

	$Stars.each do |star|
		GL.LoadIdentity()
		GL.Translate(0.0, 0.0, z)				# Move Right 3 Units, and back into the screen 7
		GL.Rotate(tilt, 1.0, 0.0, 0.0) 			# tilt the view.
		GL.Rotate(star.angle, 0.0, 1.0, 0.0) 	# rotate to the current star's angle.
		GL.Translate(star.dist, 0.0, 0.0) 		# move forward on the X plane (the star's x plane).
		GL.Rotate(-star.angle, 0.0, 1.0, 0.0)	# cancel the current star's angle.
		GL.Rotate(-tilt, 1.0, 0.0, 0.0) 		# cancel the screen tilt.
		if $Twinkle 								# twinkling stars enabled ... draw an additional star.
												# assign a color using bytes
	    	GL.Color4ub($Stars[($StarNum - 1) - star.index].r, 
						$Stars[($StarNum - 1) - star.index].g, 
						$Stars[($StarNum - 1) - star.index].b,
						255)
			GL.Begin(GL::QUADS)                  # begin drawing the textured quad.
	    		GL.TexCoord(0.0, 0.0); GL.Vertex3f(-1.0, -1.0, 0.0)
	    		GL.TexCoord(1.0, 0.0); GL.Vertex3f( 1.0, -1.0, 0.0);
	    		GL.TexCoord(1.0, 1.0); GL.Vertex3f( 1.0,  1.0, 0.0);
	    	GL.End()							# done drawing the textured quad.
		end

		# main star
		GL.Rotate(spin, 0.0, 0.0, 1.0)			# rotate the star on the z axis.
												# Assign A Color Using Bytes
		GL.Color4ub(star.r,star.g,star.b,255);
		GL.Begin(GL::QUADS)			#Begin Drawing The Textured Quad
			GL.TexCoord(0.0, 0.0); GL.Vertex3f(-1.0,-1.0, 0.0);
			GL.TexCoord(1.0, 0.0); GL.Vertex3f( 1.0,-1.0, 0.0);
			GL.TexCoord(1.0, 1.0); GL.Vertex3f( 1.0, 1.0, 0.0);
			GL.TexCoord(0.0, 1.0); GL.Vertex3f(-1.0, 1.0, 0.0);
		GL.End()				# Done Drawing The Textured Quad
	
	spin += 0.01 # used to spin the stars.
		star.angle += (star.index * 1.0 / $Stars.size * 1.0) + spininc    # change star angle.
		star.dist  -= 0.01              # bring back to center.

		if star.dist < 0.0             # star hit the center
	    	star.dist += 5.0;            # move 5 units from the center.
	    	star.r = rand(256)        # new red color.
	    	star.g = rand(256)        # new green color.
	    	star.b = rand(256)        # new blue color.
		end
	end
}

reshape = Proc.new { |w, h|
	GL.Viewport(0, 0,  w,  h) 
	GL.MatrixMode(GL::PROJECTION)
	GL.LoadIdentity()
	GLU.Perspective(60.0, w.to_f/h.to_f, 1.0, 20.0)
	GL.MatrixMode(GL::MODELVIEW)
	GL.LoadIdentity()
	GLU.LookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
}

init(ScreenWidth, ScreenHeight)

Key.set_repeat(0,0)

$Light = true
$Twinkle = true

keydown = false
done=false

while !done
	event=EventQueue.poll
	case event
	  #	when MouseButtonDownEvent
	  #	restart
		when QuitEvent
			done = false
		when KeyDownEvent 
			if event.key == K_ESCAPE
				done = false
			end
			if event.key == K_t
	       		puts "Twinkle was #{$Twinkle}"
	    		$Twinkle = !$Twinkle # switch the current value of light, between 0 and 1.
	        	puts "Twinkle is now #{$Twinkle}"
			end
			exit if event.key == K_ESCAPE
	end
	z -= 0.1 if Key.pressed?[K_PAGEUP]
	z += 0.1 if Key.pressed?[K_PAGEDOWN]
	tilt -= 0.5 if Key.pressed?[K_UP]
	tilt += 0.5 if Key.pressed?[K_DOWN]
	spininc -= 0.02 if Key.pressed?[K_LEFT]
	spininc += 0.02 if Key.pressed?[K_RIGHT]
	
	reshape.call(ScreenWidth, ScreenHeight)
	display.call
	$d.flip
end
 
