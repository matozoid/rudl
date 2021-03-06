#!/usr/bin/env ruby

require 'RUDL'
require '../utility/contained_images.rb'

# Namespaces are boring, let's get rid of them.
include RUDL
include Constant

$display=DisplaySurface.new([320, 200], HWSURFACE|DOUBLEBUF|FULLSCREEN)

$MaxBadThings=15

# contained_images parses the surface made from "crapola.bmp" in the following way:
# 1. get color on 0,0 - call it bordercolor
# 2. Start going right, until another bordercolor is found.
# 3. Go down, until yet another bordercolor is found.
# 4. Between these positions, there's an image. Make a new surface with it,
#    and store it in the 2-dimensional result array.
# 5. Repeat 2 until you run out of pixels.
# 6. Find the next "line" of images and go back to 2.
# 7. Return the two-dimensional array of found images, or one dimensional if there was

#    only one line.
# This might be easier to see if you look at crapola.bmp.
$images=Surface.load_new('media/crapola.bmp')
$images=$images.contained_images

$score=0


$sound_on=true
if RUDL.versions.include? 'SDL_mixer'
	begin
		Mixer.init(44100, 16, 2)
		$sound={
			"crash"		=> Sound.new('media/crapola_crash.wav'),
			"boom"		=> Sound.new('media/crapola_boom.wav'),
			"new ship"	=> Sound.new('media/crapola_new_ship.wav'),
			"ship boom"	=> Sound.new('media/crapola_ship_boom.wav'),
			"shoot"		=> Sound.new('media/crapola_shoot.wav')
		}
		$sound.each_value do |sound|
			sound.volume=0.2 # The music is just one channel, these sounds are much louder
		end
	rescue
		$sound_on=false

	end
else
	$sound_on=false
end

# Give the monitor some time to switch modes.
Timer.delay(1000)

# Converts all images to match the display's color type,
# and set black to be the transparent color.
# Precalcs the rotated rocks
$rotated_rocks=[]
def prepare_images
	$images.each do |image|
		image.convert
		image.set_colorkey([0,0,0], RLEACCEL)
	end
	(0...100).each do |rotation|
		$rotated_rocks[rotation]=$images[2].rotozoom(rotation*3.6, 1, false)
	end
end

# A """"sprite engine""""
class Sprite

	# All sprites that are active
	@@spritelist=[]

	# For collision detection
	@@goodbulletlist=[]
	@@badthinglist=[]
	@@goodthinglist=[]

	def Sprite.update
		bulletrect=nil

		@@spritelist.each do |sprite|
			sprite.update
		end

		Array.collide_lists(@@goodbulletlist, @@badthinglist) do |bullet, thing|
			bullet.stop
			thing.stop
			25.times do Explosion.new(thing.rect) end
			if $sound_on
				if thing.instance_of?(Rock)
					$sound['boom'].play
				else
					$sound['crash'].play
				end
			end
			$score=$score+10
		end

		Array.collide_lists(@@badthinglist, @@goodthinglist) do |enemy, friend|
			enemy.stop
			friend.stop
			$sound['ship boom'].play if $sound_on
			25.times do Explosion.new(enemy.rect) end
		end
	end

	def Sprite.draw
		@@spritelist.each do |sprite|
			sprite.draw
		end
	end

	def Sprite.reset
		@@spritelist=[]
		@@goodbulletlist=[]
		@@badthinglist=[]
		@@goodthinglist=[]
	end

	attr_accessor :rect, :image

	def initialize(position, image)
		@image=image
		@rect=[position.x, position.y, @image.w, @image.h]

		start
	end

	def update
	end

	def start
		@@spritelist.push(self)
	end

	def stop
		@@spritelist.delete(self)
	end

	def draw
		$display.blit(@image, @rect)
	end
end

class Explosion < Sprite
	def initialize(position)
		super(position, $images[rand(3)+4])
		@direction=[(rand(99)-49)/30.0, (rand(99)-49)/30.0]
		@timetolive=rand(50)
	end

	def update
		@timetolive-=1
		if @timetolive<0 then
			stop
		end
		@rect.move! @direction
		super
	end
end

class Bullet < Sprite
	def initialize(position)
		bulletposition=position.dup.move [3,-4]
		super(bulletposition, $images[1])
	end

	def update
		@rect.y-=2
		if @rect.y<-30 then stop end
		super
	end

	def start
		if @@goodbulletlist.size<9 then
			@@goodbulletlist.push(self)
			super
		end
	end

	def stop
		@@goodbulletlist.delete(self)
		super
	end
end

class Rock < Sprite
	def initialize
		super([rand(300), -30], $images[2])
		@direction=[(rand(99)-49)/20.0, rand(2)+1]
		@rotation=0
		@rotationspeed=rand(5)
	end

	def update
		@rect.move! @direction
		stop if @rect.y>200
		@direction.x=@direction.x.abs if @rect.x<0
		@direction.x=-(@direction.x.abs) if @rect.x>290
		@image=$rotated_rocks[@rotation]
		@rotation=@rotation+@rotationspeed
		@rotation=@rotation-100 while @rotation>=100  # Is this faster than modulo?
		super
	end

	def start
		if @@badthinglist.size<$MaxBadThings
			super
			@@badthinglist.push(self)
		end
	end

	def stop
		super
		@@badthinglist.delete(self)
	end
end

class Enemy < Sprite
	def initialize
		if rand(2)==1
			super([320, rand(100)], $images[3])
			@direction=-rand(5)
		else
			super([-30, rand(100)], $images[3])
			@direction=rand(5)
		end
		@center=rand(320)
	end

	def update
		@rect.x+=@direction
		@direction+=1 if @rect.x<@center-50
		@direction-=1 if @rect.x>@center+50
		@direction=-3 if @direction<-3
		@direction=3 if @direction>3
		@center=rand(320) if rand(40)==0
		if rand(190)==0
			Bomb.new(@rect)
		end
		super
	end

	def start
		if @@badthinglist.size<$MaxBadThings
			super
			@@badthinglist.push(self)
		end
	end

	def stop
		@@badthinglist.delete(self)
		DestroyedEnemy.new(@rect, @direction)
		super
	end
end

class DestroyedEnemy < Sprite
	def initialize(posrect, direction)
		super([0,0], $images[3])
		@rect=posrect
		@direction=direction
	end

	def update
		@rect.move! [@direction, 1]
		stop if @rect.y>200
		Explosion.new([@rect.x, @rect.y])
	end
end

class Bomb < Sprite
	def initialize(posrect)
		super([0,0], $images[4])
		@rect.x=posrect.x
		@rect.y=posrect.y
	end

	def update
		@rect.y+=1
		stop if @rect.y>200
	end

	def start
		if @@badthinglist.size<$MaxBadThings
			super
			@@badthinglist.push(self)
		end
	end

	def stop
		super
		@@badthinglist.delete(self)
	end
end

class Ship < Sprite
	attr_accessor :dead

	def initialize
		super([160, 160], $images[0])
		@endlessbulletstreamstopper=false
		@dead=false
	end

	def update
		# This is a bit ugly, the keystate should be grabbed once every frame
		# and be stored somewhere so that everything can access it.
		# Also, it misses keypresses, so maybe it should be rewritten to use
		# events instead.
		keystate=Key.pressed?

		if keystate[K_LEFT] && @rect.x>0
			@rect.x-=2
		end

		if keystate[K_RIGHT] && @rect.x<300
			@rect.x+=2
		end

		if keystate[K_LCTRL]
			if !@endlessbulletstreamstopper
				Bullet.new(@rect)

				sound_x=@rect.x/300.0
				sound=$sound['shoot'].play
				sound.set_panning(sound_x, 1.0-sound_x) if $sound_on && sound
				@endlessbulletstreamstopper=true
			end
		else
			@endlessbulletstreamstopper=false
		end

		super
	end

	def start
		if @@goodthinglist.size<50
			super
			@@goodthinglist.push(self)
		end
	end

	def stop
		super
		@@goodthinglist.delete(self)
		@dead=true
	end
end

def play
	Music.new('media/crapola_ha_ha_thump.mod').play(-1) if $sound_on
	Sprite.reset  # Empty spritelists
	ship=Ship.new

	lives=5
	$score=0
	deadtimer=-1  # Statevariable: if bigger than zero, do "GET READY" bit

	moment=Timer.ticks

	running=1.0
	frameskip=1

	while lives>0

		EventQueue.get.each do |event|
			case event
				when QuitEvent
					exit
				when KeyDownEvent
					case event.key
						when K_ESCAPE
							exit
						when K_RETURN
							if event.mod & KMOD_ALT>0
								$display=DisplaySurface.new([320, 200], HWSURFACE|DOUBLEBUF|($display.flags^FULLSCREEN))
								prepare_images
							end
					end
			end
		end


		# I'm not sure if this is a very optimal timing scheme:
		begin
			passed=(Timer.ticks-moment)/20.0
			Timer.delay(5)
		end while passed<1

		passed=(Timer.ticks-moment)/10.0
		if passed>5 then passed=5 end  # to avoid gigantic leaps forward in time.
		moment=Timer.ticks

		running=(running*3+passed)/4
		frameskip=running.ceil if running>frameskip+0.5
		frameskip=running.floor if running<frameskip-0.5


		# Start a new frame
		$display.fill([0, 0, 0])

		# Update as many times as frames have passed.
		# Actually, frameskip is not counted in frames but in looks-good-enough units.
		$display[frameskip, 0]=[255,255,255]
		$display[0,0]=[255,255,255]
		frameskip.times do
			Sprite.update
			if deadtimer>0
				deadtimer=deadtimer-1
				if deadtimer==0
					ship=Ship.new
					$sound['new ship'].play if $sound_on
				end
			else
				if ship.dead
					deadtimer=200
					lives-=1
					return if lives<=0
				end
			end

			Rock.new if rand(20)==0
			Enemy.new if rand(50)==0
		end

		# blit lives
		(1..lives).each do |nr|
			$display.blit($images[0], [16*nr, 0])
		end

		# blit score
		scorestring=$score.to_s
		$display.print([320-8*scorestring.length, 0], scorestring, 0xFFFFFFFF);

		# blit all sprites in the sprite "engine"
		Sprite.draw

		# show it
		$display.flip
	end
end

def gameover
	scrolltext=<<ENDSCROLL
                                        Welcome to CRAPOLA, a crappy game to demonstrate RUDL, the accellerated multimedia
 extension to Ruby. The cursor keys move your ship left and right. Left CTRL shoots
 and ALT-ENTER toggles fullscreen. Jeff Minter RU13Z... This game took a few hours to program...
 Don\'t look at the frame-skipping code, I\'m still figuring out what the best way to
 do that is... Laamella Gad - the wave of the past... Greetz in no order fly to
 the Dynamic Duo  -  Hotline  -  911  -  the Yak Society  -  Papillon ... Ah whatever,
 those groups are long dead.  Greetings to Toshiro Kuwabara (thanks for rdtool),
 Yoshiyuki Kusano, Andrew Hunt, David Thomas, Nauglin, Leon Torres, Mike Sassak, Martin Stannard,
 Pete Shinners from Pygame, Patrick May, Ulf Ekstrom, Peter Thoman, Niklas Frykholm, Matthew Bloch,
 Massimiliano Mirra, Renne Nissinen,
 t h e - e l f  from NFA for the music which was ripped from some vague Amiga demo's,
 Karl Bartel (for SFont), Andreas Schiffler (next bugreport will come in when I find
 some time <:) ) Yukihiro "Matz" Matsumoto for Ruby, Sam Lantinga for SDL, Gerard,
 Judith, Marieke, Gijs, Katja, Patrick, Matje, Femnijl(c), Mimsje(c), Rachel,
 Bert, DiDi, PeterJ, Dossey, dhr. ing. Edelwater, Frouke, Dennis, Ishi, Nekiwa,
 Joepi, Jumbo, Able Lakes King, KLinZ, Margooks, McDuvel, Sletje, Ufor Anders,
 Bernadette, Roelof, Eric de kleine enz.,
 Maurice, Martijn, Carline, Cathelijne next door, Peggy, Manon and the mice running
 through this house. Wrap is coming up. In producing RUDL, I used 30 Valkenburgs Wit,
 a P166/64MB and a P1000/128MB (upgraded to 512MB) running Win98 (now WinXP,)
 a 486/80 20MB, a P166 16MB and now a P233 64MB running Linux
 (it\'s three generations of froukepc!), MSVC (for editing),
 a Wacom tablet for drawing graphics (yeah, I could just as well have drawn them
 with a mouse,) boejon, tons of mail, mostly useless discussions on IRC, uh-oh,
 wrap time!
ENDSCROLL
	scrolltext.gsub!(/\n/, '')
	Music.new('media/crapola_fire.mod').play(-1) if $sound_on

	font=BitmapFont.new(Surface.load_new('media/24p_copperplate_blue.png'))
	logo=Surface.new(font.size('Crapola'))
	font.puts(logo, [0,0], 'Crapola')
	logo_x=(320-logo.w)/2
	logo_y=(200-logo.h)/2
	scrollpos=0
	counter=0
	start=false
	while !start
		while event=EventQueue.poll
			case event
				when QuitEvent
					exit
				when KeyDownEvent
					case event.key
						when K_ESCAPE
							exit
						when K_LCTRL
							start=true
						when K_RETURN
							if event.mod & KMOD_ALT>0
								$display.toggle_fullscreen
								prepare_images
							end
					end
			end
		end
		$display.fill([0, 0, 0])
		$display.blit(logo, [logo_x, logo_y+Math.sin(counter/90.0)*80])
		$display.print([(320-8*9)/2, (200-8)/2], 'GAME OVER', 0xFFFFFFFF)
		$display.print([-(scrollpos.modulo(8)), 190], scrolltext[scrollpos/8..scrollpos/8+41], [255, 100, 200])
		scrollpos=scrollpos+1
		counter=counter+1
		scrollpos=0 if scrollpos>scrolltext.length*8
		$display.flip
	end

end

prepare_images

while true
	gameover
	play
end
