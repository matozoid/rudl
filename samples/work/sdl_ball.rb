# $Log: sdl_ball.rb,v $
# Revision 1.1  2003/10/06 22:34:52  tsuihark
# Converted sdl_ball, as found in libsdl.org's library pages. Odd thing it is.
#

require 'RUDL'
include RUDL
include Constant

$c=32
$GForce=3.2
$useps=true
$usebg=false

class Ball
	attr_reader :pos
	attr_reader :velocity, :mass, :accel, :elast, :upforce, :fadecount
	attr_reader :gfx
	attr_reader :action
	
	def do_action
		@action.call if @action
	end
	
	def bounce
		@accel=@velocity*0.33
		@pos.y-=@accel
		if @velocity<$GForce
			@action=proc {fall}
		else
			@velocity-=@accel
		end		
	end
	
	def fall
		@accel=@velocity*0.33
		@pos.y+=@accel
		if @pos.y+@gfx.h >= @display.h
			@pos.y=@display.h-@gfx.h
			if @accel>0
				@action=proc {initbounce}
			else
				@action=proc {fadeout}
			end
		else
			@velocity+=@accel
		end
	end
	
	def fadeout
		if @fadecount<300
			if @fadecount%2==0
				#@gfx=nil
			else
				#@gfx=$BallBMP
			end
			@fadecount+=1
		else
			@action=proc {killball}
		end
	end
	
	def initbounce
		@velocity/=1.2
		@action=proc {bounce}
	end
	
	def initfall
		@velocity=$GForce-@upforce
		@action=proc {fall}
	end
	
	def initialize(display, image)
		@display=display
		@accel=0
		@fadecount=0
		@velocity=0
		@upforce=0
		@pos=Mouse.pos
		@gfx=image
		@action=proc {initfall}
	end
	
	def paletteswitch
		if $useps
			$c+=1
			@gfx.set_palette(0, [[0,0,0],[$c,$c-16,$c-32]])
		end
	end
	
	def do_render
		paletteswitch
		@display.blit(@gfx, @pos)
	end
end

def main
	doexit=false
	display=DisplaySurface.new [320,240], FULLSCREEN|HWSURFACE | DOUBLEBUF, 8
	
	ball_image=Surface.load_new('media/ball.bmp').convert
	ball_image.set_colorkey [0,0,0]
	balls=[]
	
	while !doexit
		EventQueue.get.each do |event|
			case event
				when KeyDownEvent
					case event.key
						when K_q, K_ESCAPE
							doexit=true
							break
					end
				when MouseButtonDownEvent
					balls.push Ball.new(display, ball_image)
			end
		end
		display.fill([0,0,0])
		balls.each do |ball|
			ball.do_action
			ball.do_render
		end
		display.flip
	end
end

main