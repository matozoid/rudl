#!/usr/bin/env ruby

require '../RUDL'; include RUDL; include Constant

display=DisplaySurface.new([320,200])

font=TrueTypeFont.new('media/adlibn.ttf', 48)

display.blit(font.render('Testing TTF', true, [255,255,255]), [0,0])
display.flip

Timer.delay(2000)

