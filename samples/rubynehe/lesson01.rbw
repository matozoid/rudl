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
# * US Government Users Restricted Rights 
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
# */

require 'RUDL'; include RUDL
require "opengl"
require 'glut'
require "rational"

include Constant

$ScreenWidth, $ScreenHeight = 640, 480

STDOUT.sync = TRUE # flush all output to screen immediately

$d=DisplaySurface.new([640,480], OPENGL|DOUBLEBUF) #|FULLSCREEN)

rot = 0.0

def init(w,h)
  #	GL.Viewport(0, 0,  w,  h); 
	GL.ClearColor(0.0, 0.0, 0.0, 0.0)
	GL.ClearDepth(1.0)
	GL.DepthFunc(GL::LESS)
	GL.Enable(GL::DEPTH_TEST)
	GL.ShadeModel(GL::FLAT)
   
	GL.MatrixMode(GL::PROJECTION)
	GL.LoadIdentity()
   
	GLU.Perspective(45.0, w.to_f/h.to_f, 0.1, 100.0)
	GL.MatrixMode(GL::MODELVIEW)
	GL.LoadIdentity()
	GLU.LookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
end

display = Proc.new {
	rot = rot + 0.01
	GL.Clear(GL::COLOR_BUFFER_BIT | GL::DEPTH_BUFFER_BIT) # Clear The Screen And The Depth Buffer
	GL.Color(1.0, 1.0, 1.0)
	GL.LoadIdentity()
	
	GL.Translate(-1.5, 0.0, -6.0); # Move Left 1.5 Units And Into The Screen 6.0
	
	# draw a triangle
	GL.Begin(GL::POLYGON);				# start drawing a polygon
	GL.Vertex3f( 0.0, 1.0, 0.0);		# Top
	GL.Vertex3f( 1.0,-1.0, 0.0);		# Bottom Right
	GL.Vertex3f(-1.0,-1.0, 0.0);		# Bottom Left	
	GL.End();					# we're done with the polygon

	GL.Translate(3.0,0.0,0.0);		        # Move Right 3 Units
	
	# draw a square (quadrilateral)
	GL.Begin(GL::QUADS);				# start drawing a polygon (4 sided)
	GL.Vertex3f(-1.0, 1.0, 0.0);		# Top Left
	GL.Vertex3f( 1.0, 1.0, 0.0);		# Top Right
	GL.Vertex3f( 1.0,-1.0, 0.0);		# Bottom Right
	GL.Vertex3f(-1.0,-1.0, 0.0);		# Bottom Left	
	GL.End();					# done with the polygon
}

reshape = Proc.new { |w, h|
   GL.Viewport(0, 0,  w,  h) 
   GL.MatrixMode(GL::PROJECTION)
   GL.LoadIdentity()
   GLU.Perspective(60.0,  w.to_f/h.to_f, 1.0, 20.0)
   GL.MatrixMode(GL::MODELVIEW)
   GL.LoadIdentity()
   GLU.LookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)
}

init($ScreenWidth, $ScreenHeight)

Key.set_repeat(1,10)

done=false

while !done
	event=EventQueue.poll
	case event
		when QuitEvent
			done=true
		when KeyDownEvent
			case event.key
				when K_ESCAPE
					done=true
				when K_LEFT
					$day = ($day + 10) % 360
				when K_RIGHT
					$day = ($day - 10) % 360
				when K_UP
					$year = ($year + 5) % 360
				when K_DOWN
					$year = ($year - 5) % 360
			end
	end
	reshape.call($ScreenWidth, $ScreenHeight)
	display.call
	$d.flip
end
