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
# lesson12.rbw :
#
# display lists 
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
z = -10.0 		# depth into the screen.
spin = 15.0

# white ambient light at half intensity (rgba) 
$LightAmbient = [0.5, 0.5, 0.5, 1.0]

# super bright, full intensity diffuse light.
$LightDiffuse = [1.0, 1.0, 1.0, 1.0]

# position of light (x, y, z, (position of light)) 
$LightPosition = [0.0, 0.0, 4.0]

# rotation angles
xrot, yrot = 0.0, 0.0

# 
$TopColor = [0.5, 0.5, 0.5]
$CubeColor = [0.5, 0.5, 0.5]

boxcol = [[1.0,0.0,0.0],[1.0,0.5,0.0],[1.0,1.0,0.0],[0.0,1.0,0.0],[0.0,1.0,1.0]]
topcol = [[0.5,0.0,0.0],[0.5,0.25,0.0],[0.5,0.5,0.0],[0.0,0.5,0.0],[0.0,0.5,0.5]]

$Filter = 0

$ScreenWidth, $ScreenHeight = 640, 480

STDOUT.sync = TRUE # flush all output to screen immediately

$d = DisplaySurface.new([$ScreenWidth, $ScreenHeight], OPENGL|DOUBLEBUF) #|FULLSCREEN)

# Build Cube Display List
def buildList

    $Cube= GL.GenLists(6);
	puts "buildList $Cube [#$Cube]"
    GL.NewList($Cube,GL::COMPILE);
        GL.Begin(GL::QUADS);
            # Bottom Face
            GL.Normal( 0.0,-1.0, 0.0);
            GL.TexCoord(1.0, 1.0); GL.Vertex3f(-1.0, -1.0, -1.0);
            GL.TexCoord(0.0, 1.0); GL.Vertex3f( 1.0, -1.0, -1.0);
            GL.TexCoord(0.0, 0.0); GL.Vertex3f( 1.0, -1.0,  1.0);
            GL.TexCoord(1.0, 0.0); GL.Vertex3f(-1.0, -1.0,  1.0);
            # Front Face
            GL.Normal( 0.0, 0.0, 1.0);
            GL.TexCoord(0.0, 0.0); GL.Vertex3f(-1.0, -1.0,  1.0);
            GL.TexCoord(1.0, 0.0); GL.Vertex3f( 1.0, -1.0,  1.0);
            GL.TexCoord(1.0, 1.0); GL.Vertex3f( 1.0,  1.0,  1.0);
            GL.TexCoord(0.0, 1.0); GL.Vertex3f(-1.0,  1.0,  1.0);
            # Back Face
            GL.Normal( 0.0, 0.0,-1.0);
            GL.TexCoord(1.0, 0.0); GL.Vertex3f(-1.0, -1.0, -1.0);
            GL.TexCoord(1.0, 1.0); GL.Vertex3f(-1.0,  1.0, -1.0);
            GL.TexCoord(0.0, 1.0); GL.Vertex3f( 1.0,  1.0, -1.0);
            GL.TexCoord(0.0, 0.0); GL.Vertex3f( 1.0, -1.0, -1.0);
            # Right face
            GL.Normal( 1.0, 0.0, 0.0);
            GL.TexCoord(1.0, 0.0); GL.Vertex3f( 1.0, -1.0, -1.0);
            GL.TexCoord(1.0, 1.0); GL.Vertex3f( 1.0,  1.0, -1.0);
            GL.TexCoord(0.0, 1.0); GL.Vertex3f( 1.0,  1.0,  1.0);
            GL.TexCoord(0.0, 0.0); GL.Vertex3f( 1.0, -1.0,  1.0);
            # Left Face
            GL.Normal(-1.0, 0.0, 0.0);
            GL.TexCoord(0.0, 0.0); GL.Vertex3f(-1.0, -1.0, -1.0);
            GL.TexCoord(1.0, 0.0); GL.Vertex3f(-1.0, -1.0,  1.0);
            GL.TexCoord(1.0, 1.0); GL.Vertex3f(-1.0,  1.0,  1.0);
            GL.TexCoord(0.0, 1.0); GL.Vertex3f(-1.0,  1.0, -1.0);
        GL.End();
    GL.EndList();
    $Top = $Cube + 1;
    GL.NewList($Top,GL::COMPILE);
        GL.Begin(GL::QUADS);
            # Top Face
            GL.Normal( 0.0, 1.0, 0.0);
            GL.TexCoord(0.0, 1.0); GL.Vertex3f(-1.0,  1.0, -1.0);
            GL.TexCoord(0.0, 0.0); GL.Vertex3f(-1.0,  1.0,  1.0);
            GL.TexCoord(1.0, 0.0); GL.Vertex3f( 1.0,  1.0,  1.0);
            GL.TexCoord(1.0, 1.0); GL.Vertex3f( 1.0,  1.0, -1.0);
        GL.End();
    GL.EndList();
end


# Load Bitmaps And Convert To Textures
def loadGLTextures
	# Load Texture
	raw_image = Surface.load_new("Data12/cube.bmp")
	if (raw_image.nil?)
		SDL_Quit
		exit (1)
	end
	# convert bmp from RGB to RGB - no RGB mask on Windows
    image = Surface.new(raw_image.size, SWSURFACE, 24, [0x000000FF, 0x0000FF00, 0x00FF0000, 0])
    image.blit(raw_image, [0,0])
	image = Surface.load_new("Data12/cube.bmp")

	# Create Texture   
	$Textures = GL.GenTextures(1);

	# texture 0 (linear scaling)
	GL.BindTexture(GL::TEXTURE_2D, $Textures[0]);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MAG_FILTER, GL::LINEAR);
	GL.TexParameteri(GL::TEXTURE_2D, GL::TEXTURE_MIN_FILTER, GL::LINEAR);
	GL.TexImage2D(GL::TEXTURE_2D, 0, 3, image.w, image.h, 0, GL::RGB, GL::UNSIGNED_BYTE, image.pixels)
	GL.Enable(GL::TEXTURE_2D);
end

def init(w,h)
  	loadGLTextures
	buildList
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
	# set up light number 0
    GL.Light(GL::LIGHT0, GL::AMBIENT, $LightAmbient)  # add lighting. (ambient)
    GL.Light(GL::LIGHT0, GL::DIFFUSE, $LightDiffuse)  # add lighting. (diffuse).
    GL.Light(GL::LIGHT0, GL::POSITION, $LightPosition) # set light position.
    GL.Enable(GL::LIGHT0)                             # turn light 1 on.
	GL.Enable(GL::LIGHTING)
    GL.Enable(GL::COLOR_MATERIAL);
end

display = Proc.new {
	GL.Clear(GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT) # Clear The Screen And The Depth Buffer
	GL.BindTexture(GL::TEXTURE_2D, $Textures[0]);

    6.times do |y|
	  	y.times do |x|
			GL.LoadIdentity()
        	GL.Translate(1.4+(x*2.8)-(y)*1.4,((6.0-y)*2.4)-7.0,-15.0)
         	GL.Rotate(45.0-(2.0*y)+xrot, 1.0, 0.0, 0.0)
            GL.Rotate(45.0 + yrot, 0.0, 1.0 ,0.0)
			GL.Color3f(0.5, (1.0 / 6.0) * y, (1.0 / 6.0) * y)
			GL.Color3f(*boxcol[y-1])
            GL.CallList($Cube);
			GL.Color3f(*topcol[y-1])
            GL.CallList($Top);
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

init($ScreenWidth, $ScreenHeight)

Key.set_repeat(0, 0)

$Light = true
$Twinkle = true

keydown = false
done=false

time = 0.0
while !done
  	time += 0.01
	$TopColor = [(Math.sin(time) + 1.0) / 2.0, (Math.sin(time + 0.5) + 1.0) / 2.0, 0.5 + (Math.sin(time + 1.2) + 1.0) / 4.0]
	$CubeColor = [0.5 + (Math.cos(time * 0.7) + 1.0) / 4.0, 0.5 + (Math.cos(time * 1.8 + 0.5) + 1.0) / 4.0, (Math.sin(time * 3.6 + 1.2) + 1.0) / 2.0]
	event=EventQueue.poll
	case event
	  #	when MouseButtonDownEvent
	  #	restart
		when QuitEvent
			done = true
		when KeyDownEvent 
			if event.key == K_ESCAPE
				done = false
			end
			if event.key == K_l
	    		$Light = !$Light # switch the current value of light, between 0 and 1.
	        	puts "Light is now #{$Light}"
				if $Light == false
	        		GL.Disable(GL::LIGHTING)
				else
				  	GL.Enable(GL::LIGHTING)
				end
			end
			exit if event.key == K_ESCAPE
	end
	xrot -= 0.8 if Key.pressed?[K_UP]
	xrot += 0.8 if Key.pressed?[K_DOWN]
	yrot -= 0.8 if Key.pressed?[K_LEFT]
	yrot += 0.8 if Key.pressed?[K_RIGHT]
	reshape.call($ScreenWidth, $ScreenHeight)
	display.call
	$d.flip
end

