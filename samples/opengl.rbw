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
# *  aapoly.c
# *  This program draws filled polygons with antialiased
# *  edges.  The special GL_SRC_ALPHA_SATURATE blending 
# *  function is used.
# *  Pressing the 't' key turns the antialiasing on and off.
# */

require '../RUDL'; include RUDL
require "opengl"
require 'glut'
require "rational"

STDOUT.sync = TRUE

$year = 0; $day = 0;

def init
   GL.ClearColor(0.0, 0.0, 0.0, 0.0);
   GL.ShadeModel(GL::FLAT);
end

display = Proc.new {
   GL.Clear(GL::COLOR_BUFFER_BIT);
   GL.Color(1.0, 1.0, 1.0);

   GL.PushMatrix();
   GLUT.WireSphere(1.0, 20, 16);   # draw sun */
   GL.Rotate($year, 0.0, 1.0, 0.0);
   GL.Translate(2.0, 0.0, 0.0);
   GL.Rotate($day, 0.0, 1.0, 0.0);
   GLUT.WireSphere(0.2, 10, 8);    # draw smaller planet */
   GL.PopMatrix();
}

reshape = Proc.new { |w, h|
   GL.Viewport(0, 0,  w,  h); 
   GL.MatrixMode(GL::PROJECTION);
   GL.LoadIdentity();
   GLU.Perspective(60.0,  w.to_f/h.to_f, 1.0, 20.0);
   GL.MatrixMode(GL::MODELVIEW);
   GL.LoadIdentity();
   GLU.LookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

include Constant

$d=DisplaySurface.new([640,480], OPENGL|DOUBLEBUF|FULLSCREEN)
init

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
	reshape.call(400,400)
	display.call
	$d.flip
end
