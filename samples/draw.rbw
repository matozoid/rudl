#!/usr/bin/env ruby

require '../RUDL'; include RUDL
display=DisplaySurface.new([400, 400])

scaler=0
display.print([10,10], "SDL_gfxPrimitives", 0xFF0000FF)
display.polygon([[10,10],[20,30],[40,10]], [255,200,100]).flip
Timer.delay(1000)

(0..255).each do |i|
	display.print([10,10], "horizontal_line", [i,i,i])
	display.horizontal_line([i, i], 200, [255,i,255])
	display.flip
end

(0..199).each do |i|
	display.print([10,10], "fill and filled_ellipse", 0xFFFFFFFF)
	display.fill([255-i, i, 255-i], [i, i, (200-i)*2, (200-i)*2])
	display.filled_ellipse([199,199], i, 199-i, [255,255-i,255])
	display.flip
end

(0..399).each do |i|
	scaler=i/399.0*255
	display.print([10,10], "line and antialiased_line", 0xFFFFFFFF)
	display.line([i,0], [399-i, 399], [255, scaler, 255-scaler])
	display.antialiased_line([0,399-i], [399, i], [255, 255, scaler])
	display.flip
end
