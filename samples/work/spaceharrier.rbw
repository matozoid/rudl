require 'RUDL'

include RUDL
include Constant

$display=DisplaySurface.new([320,240], FULLSCREEN)


$color=[]
$color[0]=[255,0,100]
$color[1]=[100,100,255]

$i=-100
$lasty=0


400.times {

	$display.fill([0,0,0])

	$lasty=100

	(1...200).each { |distance|
		distance=201-distance
		if(distance+$i > 0)
			y=1.0/(distance+$i)*120+120
			if y<240
				$display.fill($color[distance%2], [0, $lasty, 320, $lasty-y])
				$lasty=y
			else
				if $lasty>0
					$display.fill($color[distance%2], [0, $lasty, 320, 240])
					$lasty=0
				end
			end
		end
	}
	$i-=0.03
	$display.flip
}