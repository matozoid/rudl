require 'RUDL'
include RUDL
include Constant

display=DisplaySurface.new([500,500])

fli=FLCDecoder.new('media/Happy.flc')

loop do
	fli.surface.print([0,0], fli.frame.to_s, 0xffffffff)
	display.blit(fli.surface, [0,0])
	display.print([0,300], fli.frame.to_s, 0xffffffff)
	display.flip
	fli.delay
	fli.next
end