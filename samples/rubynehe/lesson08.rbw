#!/usr/bin/env ruby

#/*
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
#/*
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
# lesson08.rbw :
#
# blending demo 
# 
# */

require 'RUDL'
include RUDL
require "opengl"
require 'glut'
require "rational"

include Constant

$Textures = []

xspeed = 0.0
yspeed = 0.0
z = -5.0 # depth into the screen.

# white ambient light at half intensity (rgba) 
$LightAmbient = [0.5, 0.5, 0.5, 1.0]

# super bright, full intensity diffuse light.
$LightDiffuse = [1.0, 1.0, 1.0, 1.0]

# position of light (x, y, z, (position of light)) 
$LightPosition = [0.0, 0.0, 2.0, 1.0]

# rotation angles
xrot, yrot = 0.0, 0.0

$Filter = 0

$ScreenWidth, $ScreenHeight = 640, 480

STDOUT.sync = TRUE # flush all output to screen immediately

$d = DisplaySurface.new([$ScreenWidth, $ScreenHeight], OPENGL|DOUBLEBUF) #|FULLSCREEN)

# Load Bitmaps And Convert To Textures
def loadGLTextures
	# Load Texture

	raw_image = Surface.load_new("Data08/glass.bmp")
	if (raw_image.nil?)
		SDL_Quit
		exit (1)
	end
	# convert bmp from BGR to RGB - no BGR mask on Windows
    image = Surface.new(raw_image.size, SWSURFACE, 24, [0x000000FF, 0x0000FF00, 0x00FF0000, 0])
    image.blit(raw_image, [0,0])
	
	# Create Texture   
	$Textures = GL.GenTextures(3);

	# texture 1 (poor quality scaling)
	GL.BindTexture(GL::TEXTURE_2D, $Textures[0]);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::NEAREST);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::NEAREST);
	GL.TexImage2D(GL::TEXTURE_2D, 0, 3, image.w, image.h, 0, GL::RGB, GL::UNSIGNED_BYTE, image.pixels)

	# texture 2 (linear scaling)
	GL.BindTexture(GL::TEXTURE_2D, $Textures[1]);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::LINEAR);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::LINEAR);
	GL.TexImage2D(GL::TEXTURE_2D, 0, 3, image.w, image.h, 0, GL::RGB, GL::UNSIGNED_BYTE, image.pixels)
	
	# texture 3 (mipmapped scaling)
	GL.BindTexture(GL::TEXTURE_2D, $Textures[2]);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::LINEAR);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::LINEAR_MIPMAP_NEAREST);
	GL.TexImage2D(GL::TEXTURE_2D, 0, 3, image.w, image.h, 0, GL::RGB, GL::UNSIGNED_BYTE, image.pixels)

	# 2d texture, 3 colors, width, height, RGB in that order, byte data, and the data.
    GLU.Build2DMipmaps(GL::TEXTURE_2D, 3, image.w, image.h, GL::RGB, GL::UNSIGNED_BYTE, image.pixels); 
end

def init(w,h)
  	loadGLTextures
	GL.ClearColor(0.0, 0.0, 0.0, 0.0)
	GL.ClearDepth(1.0)
	GL.DepthFunc(GL::LESS)
	GL.Enable(GL::DEPTH_TEST)
	GL.ShadeModel(GL::SMOOTH)
   
	GL.MatrixMode(GL::PROJECTION)
	GL.LoadIdentity()
   
	GLU.Perspective(45.0, w.to_f/h.to_f, 0.1, 100.0)
	GL.MatrixMode(GL::MODELVIEW)
	GL.LoadIdentity()
	GLU.LookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
	# set up light number 1.
    GL.Light(GL::LIGHT1, GL::AMBIENT, $LightAmbient)  # add lighting. (ambient)
    GL.Light(GL::LIGHT1, GL::DIFFUSE, $LightDiffuse);  # add lighting. (diffuse).
    GL.Light(GL::LIGHT1, GL::POSITION, $LightPosition); # set light position.
    GL.Enable(GL::LIGHT1);                             # turn light 1 on.
	# setup blending 
    GL.BlendFunc(GL::SRC_ALPHA,GL::ONE);			# Set The Blending Function For Translucency
    GL.Color4f(1.0, 1.0, 1.0, 0.5);    
end

display = Proc.new {
	GL.Clear(GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT) # Clear The Screen And The Depth Buffer

	GL.Enable(GL::TEXTURE_2D);
	GL.TexEnvf(GL::TEXTURE_ENV, GL::TEXTURE_ENV_MODE, GL::DECAL);
	GL.BindTexture(GL::TEXTURE_2D, $Textures[$Filter]);

	GL.LoadIdentity();				# make sure we're no longer rotated.
	GL.Translate(0.0, 0.0, z)		# Move Right 3 Units, and back into the screen 7
	
	GL.Rotate(xrot, 1.0, 0.0, 0.0)		# Rotate The Cube On X, Y, and Z
	GL.Rotate(yrot, 0.0, 1.0, 0.0)		# Rotate The Cube On X, Y, and Z
	
	# draw a cube (6 quadrilaterals)
	GL.Begin(GL::QUADS);				# start drawing the cube.

	# top of cube
	GL.TexCoord(0.0, 1.0);	GL.Vertex3f( 1.0, 1.0, -1.0)		# Top Right Of The Quad (Top)
	GL.TexCoord(1.0, 1.0);	GL.Vertex3f(-1.0,  1.0, -1.0)		# Top Left Of The Quad (Top)
	GL.TexCoord(1.0, 0.0);	GL.Vertex3f(-1.0,  1.0,  1.0)		# Bottom Left Of The Quad (Top)
	GL.TexCoord(0.0, 0.0);	GL.Vertex3f( 1.0,  1.0,  1.0)		# Bottom Right Of The Quad (Top)

	# bottom of cube
	GL.TexCoord(0.0, 1.0);	GL.Vertex3f( 1.0, -1.0,  1.0)		# Top Right Of The Quad (Bottom)
	GL.TexCoord(1.0, 1.0);	GL.Vertex3f(-1.0, -1.0,  1.0)		# Top Left Of The Quad (Bottom)
	GL.TexCoord(1.0, 0.0);	GL.Vertex3f(-1.0, -1.0, -1.0)		# Bottom Left Of The Quad (Bottom)
	GL.TexCoord(0.0, 0.0);	GL.Vertex3f( 1.0, -1.0, -1.0)		# Bottom Right Of The Quad (Bottom)

	# front of cube
	GL.TexCoord(0.0, 1.0);	GL.Vertex3f( 1.0,  1.0,  1.0)		# Top Right Of The Quad (Front)
	GL.TexCoord(1.0, 1.0);	GL.Vertex3f(-1.0,  1.0,  1.0)		# Top Left Of The Quad (Front)
	GL.TexCoord(1.0, 0.0);	GL.Vertex3f(-1.0, -1.0,  1.0)		# Bottom Left Of The Quad (Front)
	GL.TexCoord(0.0, 0.0);	GL.Vertex3f( 1.0, -1.0,  1.0)		# Bottom Right Of The Quad (Front)

	# back of cube.
	GL.TexCoord(0.0, 1.0);  GL.Vertex3f( 1.0, -1.0, -1.0)		# Top Right Of The Quad (Back)
	GL.TexCoord(1.0, 1.0); 	GL.Vertex3f(-1.0, -1.0, -1.0)		# Top Left Of The Quad (Back)
	GL.TexCoord(1.0, 0.0); 	GL.Vertex3f(-1.0,  1.0, -1.0)		# Bottom Left Of The Quad (Back)
	GL.TexCoord(0.0, 0.0); 	GL.Vertex3f( 1.0,  1.0, -1.0)		# Bottom Right Of The Quad (Back)

	# left of cube
	GL.TexCoord(0.0, 1.0);	GL.Vertex3f(-1.0,  1.0,  1.0)		# Top Right Of The Quad (Left)
	GL.TexCoord(1.0, 1.0);	GL.Vertex3f(-1.0,  1.0, -1.0)		# Top Left Of The Quad (Left)
	GL.TexCoord(1.0, 0.0);	GL.Vertex3f(-1.0, -1.0, -1.0)		# Bottom Left Of The Quad (Left)
	GL.TexCoord(0.0, 0.0);	GL.Vertex3f(-1.0, -1.0,  1.0)		# Bottom Right Of The Quad (Left)

	# Right of cube
	GL.TexCoord(0.0, 1.0);	GL.Vertex3f( 1.0,  1.0, -1.0)	      	# Top Right Of The Quad (Right)
	GL.TexCoord(1.0, 1.0);	GL.Vertex3f( 1.0,  1.0,  1.0)		# Top Left Of The Quad (Right)
	GL.TexCoord(1.0, 0.0);	GL.Vertex3f( 1.0, -1.0,  1.0)		# Bottom Left Of The Quad (Right)
	GL.TexCoord(0.0, 0.0);	GL.Vertex3f( 1.0, -1.0, -1.0)		# Bottom Right Of The Quad (Right)
	GL.End()

	GL.Disable(GL::TEXTURE_2D)
	xrot += xspeed
	yrot += yspeed
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

init($ScreenWidth, $ScreenHeight)

$Light = true
$Blend = true

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
			if event.key == K_l
	       		puts "Light was #{$Light}"
	    		$Light = !$Light # switch the current value of light, between 0 and 1.
	        	puts "Light is now #{$Light}"
	        	$Light ? GL.Disable(GL::LIGHTING) : GL.Enable(GL::LIGHTING)
			end
			if event.key == K_f
	    		puts "Filter was #{$Filter}"
	        	$Filter += 1 
				$Filter = 0 if $Filter > 2
	        	puts "Filter is now #{$Filter}"
			end
			if event.key == K_b
	       		puts "Blend was #{$Blend}"
	    		$Blend = !$Blend # switch the current value of light, between 0 and 1.
	        	puts "Blend is now #{$Blend}"
	        	if $Blend
					GL.Enable(GL::BLEND)
					GL.Disable(GL::DEPTH_TEST)
				else
					GL.Disable(GL::BLEND)
					GL.Enable(GL::DEPTH_TEST)
				end
			end
			exit if event.key == K_ESCAPE
	end
	z -= 0.02 if Key.pressed?[K_PAGEUP]
	z += 0.02 if Key.pressed?[K_PAGEDOWN]
	
	xspeed -= 0.01 if Key.pressed?[K_UP]
	xspeed += 0.01 if Key.pressed?[K_DOWN]
	yspeed -= 0.01 if Key.pressed?[K_LEFT]
	yspeed += 0.01 if Key.pressed?[K_RIGHT]
	
	reshape.call($ScreenWidth, $ScreenHeight)
	display.call
	$d.flip
end

