# Made by Martin Stannard

require 'RUDL' 
include RUDL
include Constant


$ScreenX = 800
$ScreenY = 600

$ScreenSize = [$ScreenX, $ScreenY]
$ScreenXMid = $ScreenX / 2
$ScreenYMid = $ScreenY / 2
$Clock = 0.0

class Font

	attr_accessor :image
	
	def initialize(filename, size)
		@font = TrueTypeFont.new(filename, size)
	end
	
	def makeWord(word, colour)
		@image = @font.render(word, false, colour)
	end
end

class PolyElement

	attr_accessor :angle, :dist, :surface
	
	def initialize(image)
		@angle = 0.0
		@dist = 0.0
		loadImage(image)
	end
	
	def getXY
		x = (Math::sin(@angle) * dist) + $ScreenXMid
		y = (Math::cos(@angle) * dist) + $ScreenYMid
		[x - @offx, y - @offy]
	end
	
	def blit(surface)
		surface.blit(@surface , getXY)
	end
	
	def rotate(inc)
		@angle += inc
	end
	
	def move(inc)
		@dist += inc
	end
	
	def setColorKey(color)
		@surface.set_colorkey(color, RLEACCEL)
	end
	
	def loadImage(surface)
		@surface=surface
		@offx = @surface.w / 2
		@offy = @surface.h / 2
		#@surface.set_ColorKey([0,0,0])
	end
	
end
   
class Poly

	def initialize
		@elements = Array.new
		@rotspeed = (rand * 0.002)# + 0.01
		@rotspeed *= -1.0 if rand < 0.5
		@movespeed = (rand * 5.0) + 2.0
		@movespeed *= -1.0 if rand < 0.5
		@rotamp = rand * 5.0 
		@moveamp = (rand * 8.0) + 1.0
		newImage
#		@angleinc = rand 
#		puts "Poly @rotspeed #{@rotspeed} @movespeed #{@movespeed}"
	end						

	def addElement
		@elements.push(PolyElement.new(@surface))
	end
	
	def blit(surface)
		@elements.each { |e| e.blit(surface) }
	end
	
	def getElement(i)
		@elements[i]
	end
	
	def addElements(count)
		startangle = rand * 2 * Math::PI
		startdist = rand * 300.0
		
		count.times do |t|
			addElement
			@elements.last.angle = startangle + (Math::PI * 2 / count) * t
			@elements.last.dist = startdist	
		end
	end
	
	def rotate
#   	@elements.each { |e| e.rotate(@rotspeed) }
		@elements.each do |e|
#			a = e.angle 
			e.rotate(Math::sin($Clock * @rotspeed) * @rotamp) 
		end
	end
	
	def move
		@elements.each { |e| e.move(Math::sin($Clock * @movespeed) * @moveamp) }
	end
	
	def	update
		rotate
		move		
	end
	
	def changeImage
		
		@elements.each do |e|
			e.loadImage(@surface)
		end
	end	

	def newImage
		size=rand(20)+5
		@surface=Surface.new [size*2, size*2], SWSURFACE|SRCALPHA, 32
		
		@surface.filled_circle([size, size], size-1, [rand(255), rand(255), rand(255), rand(255)])
		@surface.convert_alpha!
		
	end
end

class PolyScope

	def initialize(count)
		@polys = Array.new
		count.times do
			@polys.push(Poly.new)
			@polys.last.addElements(rand(12) + 1)
		end
	end
	
	def blit(surface)
		@polys.each { |p| p.blit(surface) }
	end
					
	def update
		@polys.each { |p| p.update }
		add if rand(120) == 0 && @polys.size < 15 
		remove if rand(100) == 0 && @polys.size > 1 
		changeImage if rand(200) == 0 
	end
	
	def add
		@polys.push(Poly.new)
		@polys.last.addElements(rand(20) + 1)
	end
	
	def remove
		index = rand(@polys.size)
		@polys.delete_at(index)
	end
	
	def changeImage	
		index = rand(@polys.size)
		@polys[index].changeImage	
	end	
end


def resetPolys
	$Polys = PolyScope.new(rand(8) + 2)
	$Display.fill([0, 0, 0])
	$Display.blit($Font.image , [0, 0])
end
	



$Font = Font.new("goose.ttf", 50)
$Font.makeWord("PolyScope", [205, 50, 240])
$Display=DisplaySurface.new($ScreenSize, HWSURFACE|FULLSCREEN)  #DOUBLEBUF| 
$Display.fill([0, 0, 0])
$Display.flip

backImage = Surface.new([$ScreenX, $ScreenY]) 
backImage.blit($Font.image , [0, 0])

resetPolys
#backBuffer=Surface.new($ScreenSize, display.flags)
#backBuffer.fill([0, 0, 0])

i = 0
waitflag = true

while true
#	$Display.fill([0, 0, 0])
#	$Display.flip
	$Polys.update
	$Polys.blit($Display)
#	$Display.flip
	if EventQueue.poll.class==KeyDownEvent then
		keys = Key.pressed?
		if keys.has_key?(K_SPACE)
			resetPolys
			i = 0
		end
		waitflag = !waitflag if keys.has_key?(K_w)	   
		exit if keys.has_key?(K_ESCAPE)
	end
	sleep(0.01) if waitflag
	$Clock += 0.02
	i += 1
	$Display.blit($Font.image , [0, 0]) ; $Display.fill([0, 0, 0]) if rand(1000) == 0
end



