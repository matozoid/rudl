require 'RUDL'; include RUDL; include Constant

$display=DisplaySurface.new [640,480]

class Map
	def initialize(filename)
		load_map(filename)
	end
	
	def load_map(filename)
		
	end
end


m=Map.new('media/testmap.txt')
