#!/usr/bin/env ruby

# I use ../ because I want to use the RUDL.so that is made by extconf.rb and the makefile.
# You normally use just:
# require 'RUDL'
require '../RUDL'

# Namespaces are boring, let's get rid of them.
include RUDL
include Constant

$display=DisplaySurface.new([320, 200], HWSURFACE|DOUBLEBUF|FULLSCREEN)

$MaxBadThings=50

# contained_images parses the surface made from "crapola.bmp" in the following way:
# 1. get color on 0,0 - call it bordercolor
# 2. Start going right, until another bordercolor is found.
# 3. Go down, until yet another bordercolor is found.
# 4. Between these positions, there's an image. Make a new surface with it,
#    and store it in the 2-dimensional result array.
# 5. Repeat 2 until you run out of pixels.
# 6. Find the next "line" of images and go back to 2.
# 7. Return the two-dimensional array of found images
# This might be easier to see if you look at crapola.bmp.
# Changed! I've changed the image, there's only one line now, and thus it returns
# a one-dimensional array.
$images=Surface.load_new('crapola.bmp').contained_images

$score=0

$sound_on=true

if RUDL.used_libraries.include? 'SDL_mixer'
	begin
		Mixer.init(44100, 16, 2)
		$sound={
			"crash"		=> Sound.new('crapola_crash.wav'),
			"boom"		=> Sound.new('crapola_boom.wav'),
			"new ship"	=> Sound.new('crapola_new_ship.wav'),
			"ship boom"	=> Sound.new('crapola_ship_boom.wav'),
			"shoot"		=> Sound.new('crapola_shoot.wav')
		}
		$sound.each do |sound|
			sound[1].volume=0.2 # The music is just one channel, these sounds are much louder
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

		Rect.collide_lists(@@goodbulletlist, @@badthinglist) do |bullet, thing|
			bullet.stop
			thing.stop
			25.times do Explosion.new([thing.rect.x, thing.rect.y]) end
			if $sound_on
				if thing.instance_of?(Rock)
					$sound['boom'].play
				else
					$sound['crash'].play
				end
			end
			$score=$score+10
		end

		Rect.collide_lists(@@badthinglist, @@goodthinglist) do |enemy, friend|
			enemy.stop
			friend.stop
			$sound['ship boom'].play if $sound_on
			25.times do Explosion.new([enemy.rect.x, enemy.rect.y]) end
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
		@rect=Rect.new(position[0], position[1], @image.w, @image.h)
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
		$display.blit(@image, [@rect.x, @rect.y])
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
		@rect.x+=@direction[0]
		@rect.y+=@direction[1]
		super
	end
end

class Bullet < Sprite
	def initialize(position)
		bulletposition=[position[0]+3, position[1]-4]
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
		@rect.x+=@direction[0]
		@rect.y+=@direction[1]
		stop if @rect.y>200
		@direction[0]=@direction[0].abs if @rect.x<0
		@direction[0]=-(@direction[0].abs) if @rect.x>290
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
		@rect.x+=@direction
		@rect.y+=1
		stop if @rect.y>200
		Explosion.new([@rect.x, @rect.y])
	end
end

class Bomb < Sprite
	def initialize(posrect)
		super([0,0], $images[4])
		@rect=posrect.clone
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
				Bullet.new([@rect.x, @rect.y])
				$sound['shoot'].play if $sound_on
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
	Music.new('crapola_ha_ha_thump.mod').play(-1) if $sound_on
	Sprite.reset  # Empty spritelists
	ship=Ship.new

	lives=5
	$score=0
	deadtimer=-1  # Statevariable: if bigger than zero, do "GET READY" bit

	moment=Timer.ticks

	running=1.0
	frameskip=1

	while lives>0

		while event=EventQueue.poll
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

def makescrolltextimage
	scrolltext='                                        '+
	'Welcome to CRAPOLA, a crappy game to demonstrate RUDL, the accellerated multimedia '+
	'extension to Ruby. The cursor keys move your ship left and right. Left CTRL shoots '+
	' and ALT-ENTER toggles fullscreen. The libraries used in RUDL are SDL ofcourse, '+
	'SDL_gfxPrimitives '+
	'(here only used for the text, but it does lots more, as you can see in the docs) and '+
	'SDL_rotozoom for the rotating rocks (I really ought to do some centering on those '+
	'things, they wobble around like they\'re drunk, and let\'s not forget they\'re eating '+
	'memory and CPU!)... Jeff Minter RU13Z... This game took a few hours to program... '+
	'Don\'t look at the frame-skipping code, I\'m still figuring out what the best way to '+
	'do that is... ¡Laamella Gad! - the wave of the past... Greetz in no order fly to '+
	'the Dynamic Duo  -  Hotline  -  911  -  the Yak Society  -  Papillon ... Ah whatever, '+
	'those groups are long dead.  Greetings to Toshiro Kuwabara (thanks for rdtool), '+
	'Yoshiyuki Kusano (I gave up on OpenGL for now...) Andrew Hunt, David Thomas, '+
	't h e - e l f  from NFA for the music which was ripped from some vague Amiga demo\'s, '+
	'Karl Bartel (for SFont), Andreas Schiffler (next bugreport will come in when I find '+
	'some time <:) ) Yukihiro "Matz" Matsumoto for Ruby, Sam Lantinga for SDL, Gerard, '+
	'Judith, Marieke, Gijs, Katja, Patrick, Mat(t)hieu, Femnijl(c), Mimsje(c), Rachel, '+
	'Bert, DiDi, PeterJ, Dossey, dhr. ing. Edelwater, Frouke!, Dennis, Ishi, Nekiwa, '+
	'Joepi, Jumbo, Able Lakes King, KLinZ, Margooks, McDuvel, Sletje, Ufor Anders, '+
	'Bernadette, Roelof, Marc, Huiskokkie, '+
	'Maurice, Martijn, Carline, the girl next door, Peggy, Manon and the mice running '+
	'through this house. Wrap is coming up. In producing RUDL, I used 30 Valkenburgs Wit, '+
	'a P166/64MB running Win98, a 486/80 20MB running Linux (it\'s froukepc!), MSVC (for '+
	'editing), CygWin (for compiling: it\'s a piece of sh*t, but nothing else works), '+
	'a Wacom tablet for drawing graphics (yeah, I could just as well have drawn them '+
	'with a mouse), boejon, tons of mail, mostly useless discussions on IRC, uh-oh, '+
	'wrap time!'
	img=Surface.new([8*scrolltext.length, 8])
	img.print([0,0],scrolltext, [255, 100, 200])
end

def gameover
	Music.new('crapola_fire.mod').play(-1) if $sound_on
	font=SFont.new(Surface.load_new('24p_copperplate_blue.png'))
	logo=Surface.new(font.size('Crapola'))
	font.puts(logo, [0,0], 'Crapola')
	logo_x=(320-logo.w)/2
	logo_y=(200-logo.h)/2
	scrollpos=0
	counter=0
	scrollimg=makescrolltextimage
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
								$display=DisplaySurface.new([320, 200], HWSURFACE|DOUBLEBUF|($display.flags^FULLSCREEN))
								prepare_images
							end
					end
			end
		end
		$display.fill([0, 0, 0])
		$display.blit(logo, [logo_x,logo_y+Math.sin(counter/90.0)*80])
		$display.print([(320-8*9)/2, (200-8)/2], 'GAME OVER', 0xFFFFFFFF)
		$display.blit(scrollimg, [-scrollpos, 190])
		scrollpos=scrollpos+1
		counter=counter+1
		scrollpos=0 if scrollpos>scrollimg.w
		$display.flip
	end
end

prepare_images

while true
	gameover
	play
end
