require 'RUDL'

include RUDL
include Constant

$display=DisplaySurface.new [640,480]
$TurnResolution=3.0

class Tank
	def initialize(position, color)
		
		@position=position
		@images=[]
		@original_images=[]
		@rotation=0.0
		@speed=0.0
		@acceleration=1.0
		@animationpos=0.0
		(1..3).each do |i|
			image=Surface.load_new("media/#{color}_tank_#{i}.bmp")
			image.set_colorkey([255,255,255], RLEACCEL)
			@original_images.push(image)
		end
		(0..360/$TurnResolution).each do |angle_counter|
			angle=angle_counter*$TurnResolution
			new_images=[]
			@original_images.each do |image|
				new_images.push image.rotozoom(angle, 1.0, true)
			end
			@images.push(new_images)
		end
	end
	
	def step
		@rotation+=1.0
		@rotation-=360 if @rotation>=360
		@animationpos+=@acceleration/10
		@position.x-=Math.sin((@rotation/180)*3.14)*@acceleration
		@position.y-=Math.cos((@rotation/180)*3.14)*@acceleration
		while @animationpos>=@original_images.size
			@animationpos-=@original_images.size
		end
		while @animationpos<0
			@animationpos+=@original_images.size
		end
	end
	
	def draw
		image=@images[@rotation/$TurnResolution][@animationpos]
		$display.blit(image, @position.move([image.w/-2.0, image.h/-2.0]))
	end
end

class Game
	def run
		@tanks=[Tank.new([100,100],'red'), Tank.new([300,100],'blue')]
		while true
			@tanks.each do |tank|
				tank.step
				tank.draw
			end
			$display.flip
		end		
	end
end

Game.new.run