#!/usr/bin/env ruby

require 'RUDL'; include RUDL; include Constant

# Eek! Globals!
$sound_on=RUDL.versions.include? 'SDL_mixer'
$display=DisplaySurface.new([320,200], FULLSCREEN|DOUBLEBUF)
Mixer.num_channels=1 if $sound_on # True to the original, only a single voice <:)
$bop=Sound.new('media/pong_bop.wav') if $sound_on

Timer.delay(500) # Time for the monitor to switch modes

$BatSize=32

# This turns an ugly array like the one below this method into an array of Surfaces

def make_numbers(number_array)
	numbercolor=[0, 255, 0]
	background=[0, 128, 0]
	blocks={
		'~' => Surface.new([6,8]).fill(background).fill(numbercolor, [0,0,6,4]),
		'#' => Surface.new([6,8]).fill(numbercolor),
		' ' => Surface.new([6,8]).fill(background),
		'_' => Surface.new([6,8]).fill(background).fill(numbercolor, [0,4,6,4])}

	numbers=[]
	number_array.each do |number_data|
		sx=number_data[0].length
		sy=number_data.size
		number=Surface.new([sx*6, sy*8]).set_colorkey(background, RLEACCEL)
		(0...sx).each do |x|
			(0...sy).each do |y|
				number.blit(blocks[number_data[y][x].chr], [x*6, y*8])
			end
		end
		numbers.push(number)
	end
	numbers
end

$numbers=make_numbers(
[['   #~#',
  '   # #',
  '   #_#'],

 ['     #',
  '     #',
  '     #'],

 ['   ~~#',
  '   #~~',
  '   #__'],

 ['   ~~#',
  '   ~~#',
  '   __#'],

 ['   # #',
  '   ~~#',
  '     #'],

 ['   #~~',
  '   ~~#',
  '   __#'],

 ['   #  ',
  '   #~#',
  '   #_#'],

 ['   ~~#',
  '     #',
  '     #'],

 ['   #~#',
  '   #~#',
  '   #_#'],

 ['   #~#',
  '   ~~#',
  '     #'],

 ['#  #~#',
  '#  # #',
  '#  #_#'],

 ['#    #',
  '#    #',
  '#    #']])


class Player
	attr_reader :y, :x, :score_pos
	attr_accessor :score

	def initialize(side)
		@side=side
		if side==0
			@defaultX=16
			@score_pos=[100,10]
		else
			@defaultX=$display.w-16-8
			@score_pos=[200,10]
		end
		@defaultY=($display.h-$BatSize)/2
		reset
	end

	def to_a
		[@x, @y]
	end

	def y=(new_y)
		if new_y<0
			@y=0
		elsif new_y>$display.h-$BatSize
			@y=$display.h-$BatSize
		else
			@y=new_y
		end
	end

	def reset
		@x=@defaultX
		@y=@defaultY
		@score=0
	end

	def react(bat_pos)
	end

	def collide(ball_pos, ball_dir)
		if [@x, @y, 8, $BatSize].overlaps? [ball_pos[0], ball_pos[1], 8, 8]
			if @side==0
				ball_dir[0]=ball_dir[0].abs
			else
				ball_dir[0]=-(ball_dir[0].abs)
			end
			if ball_pos[1]<=@y
				ball_dir[1]-=1
			elsif ball_pos[1]>=$BatSize+@y-8
				ball_dir[1]+=1
			end
			$bop.play if $sound_on
		end
	end
end


class HumanPlayer < Player
	def initialize(x)
		super
		Mouse.pos=[0, @defaultY]
	end

	def react(ball_pos)
		self.y+=Mouse.rel[1]
	end
end


class ComputerPlayer < Player
	def react(ball_pos)
		if @y>ball_pos[1]+2
			self.y-=3
		elsif @y<ball_pos[1]-10
			self.y+=3
		end
	end
end


class Pong
	Foreground=[0,255,0]
	Background=[0,128,0]

	def initialize
		@players=[HumanPlayer.new(0), ComputerPlayer.new(1)]

		Mouse.visible=false

		@bat=Surface.new([8,$BatSize]).fill(Foreground)
		@ball=Surface.new([8,8]).fill(Foreground)

		@blerpblerp=Sound.new('media/pong_blerpblerp.wav') if $sound_on
		@bip=Sound.new('media/pong_bip.wav') if $sound_on

		restart
	end

	def restart
		@gameOver=false
		@players.each{|p| p.reset}
		@moment=Timer.ticks
		new_ball
	end

	def new_ball
		@ball_pos=[($display.w-8)/2, ($display.h-8)/2]
		@ball_dir=[rand(1)*3-1, rand(5)-2]
		@bip.play if $sound_on
	end

	def score(player)
		if !@gameOver
			if (@players[player].score+=1) >10
				@gameOver=true
				@blerpblerp.play
			else
				new_ball
			end
		end
	end

	def play
		while true
			
			event=EventQueue.poll
			case event
				when MouseButtonDownEvent
					restart
				when QuitEvent
					return false
				when KeyDownEvent
					if event.key==K_ESCAPE
						return false
					end
			end

			# This calculates the time that has passed since the last frame.
			# It is used to scale movements. It needs some fix that will keep
			# the framerates a little more steady.

			passed=(Timer.ticks-@moment)/(1000.0/150.0)
			if passed>=1
				if passed>10 then passed=10 end # to avoid gigantic leaps forward in time
				@moment=Timer.ticks
			end
	
			# The movement loop

			passed.truncate.times do
				@players.each do |player|
					player.react(@ball_pos)
					player.collide(@ball_pos, @ball_dir)
				end

				if !@gameOver
					@ball_pos.move! @ball_dir
				end

				if @ball_pos[1]>$display.h-8
					@ball_pos[1]=$display.h-8
					@ball_dir[1]=-(@ball_dir[1].abs)
					$bop.play if $sound_on
				elsif
					@ball_pos[1]<0
					@ball_pos[1]=0
					@ball_dir[1]=@ball_dir[1].abs
					$bop.play if $sound_on
				end

				score(1) if @ball_pos[0]<-8
				score(0) if @ball_pos[0]>$display.w
			end
			
			# The blitting is really, really, really simplistic

			if passed>=1
				$display.fill(Background)
	
				@players.each do |player|
					$display.blit(@bat, player.to_a)
					$display.blit($numbers[player.score], player.score_pos)
				end
	
				$display.blit(@ball, @ball_pos)
	
				$display.flip
			end
		end
	end
end

Pong.new.play
