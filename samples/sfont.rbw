#!/usr/bin/env ruby

require 'RUDL'; include RUDL; include Constant

display=DisplaySurface.new([640,480])

font=BitmapFont.new(Surface.load_new('media/24P_Copperplate_Blue.png'))

font.print(display, [10,10], 'This is BitmapFont#print in action')
display.flip
Timer.delay(2000)

font.print_centered(display, 100, 'This is centered')
display.flip
Timer.delay(2000)

font.print(display, [0,200], 'The word SPAM is '+font.size('SPAM').join('x')+' pixels big')
display.flip
Timer.delay(3000)
