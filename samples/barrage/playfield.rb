# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
# See comments in barrage.rbw

require 'RUDL';  include RUDL
require 'tank'
require 'particle'

# The game is all ready to accept 800x600 bitmap playfields as well as randomly-generated ones,
# I just need to add code to load the bitmap in.  Black is empty space.
class Playfield

    def Playfield.steepness
        case $game_option_values["Map Steepness"]
            when "Flat"
            [1,0,0]
            when "Gentle"
            [3,1,25]
            when "Rolling"
            [5,2,15]
            when "Mountainous"
            [7,3,15]
            when "Steep"
            [9,4,10]
            when "Cliffs"
            [11,5,8]
        end
    end

  def initialize
    $playfield_buffer = Surface::new [800,600], SWSURFACE, 24  # because we'll be manipulating pixels often
    $playfield_buffer.fill [0,0,0]
    $playfield_buffer.set_colorkey [0,0,0]
    # generate random playfield
    which_way = rand(2)
    x = 0 if which_way == 0
    x = 799 if which_way == 1
    y = rand(450) + 150
    dir = 0
    steep, steepHalf, chance = Playfield::steepness
    until ( (which_way == 0) ? x == 800 : x == 0 )
      if rand(15) == 0 then dir = rand(steep)-steepHalf; end
      dir = -dir if y < 75 && dir < 0
      dir = -dir if y > 800 && dir > 0
      y += dir
      $playfield_buffer.fill topsoil, [x, y, 1, 3]
      $playfield_buffer.fill dirt, [x, y+3, 1, 600-y+3]
      (which_way == 0) ? x += 1 : x -= 1
    end
  $backbuffer.blit $playfield_buffer, [0,0]
  @drop_y = 0
  end
  
    def flatten center_x
        y = 0
        # we need to make sure there's ground underneath the tank, or the game simply locks up, looking for the ground forever
        y += 1 until collide [center_x, y] or y == 590
        y = 590 if y > 590
        x = center_x - Tank::HALF_LENGTH*2
        until x == center_x + Tank::HALF_LENGTH*2
            $playfield_buffer.fill [0,0,0], [x,0,1,y]
            $playfield_buffer.fill topsoil, [x,y,1,3]
            $playfield_buffer.fill dirt, [x,y+3,1, 600-y+3]
            $backbuffer.fill [0,0,0], [x,0,1,y]
            $backbuffer.fill topsoil, [x,y,1,3]
            $backbuffer.fill dirt, [x,y+3,1, 600-y+3]
            x += 1
        end
        y-1
    end
    
    def topsoil
        [200,150,0]
    end
    
    def dirt
        [100,60,20]
    end
  
    def destroy
        $playfield_buffer.destroy if $playfield_buffer
    end
    
    def remove_dirt point, radius
        col = $playfield_buffer[point[0],point[1]]
        $playfield_buffer.filled_circle([point[0],point[1]], radius, [0,0,0])
        $backbuffer.filled_circle([point[0],point[1]], radius, [0,0,0])
    end
  
    def add_dirt point, size, color
        half = (size.to_i/2).to_i
        arr = [point[0]-half, point[1]-half, size, size]
        $playfield_buffer.fill color, arr
        $backbuffer.fill color, arr
    end
  
    def draw
    end
    
    def collide point
        return false if point[0] < 0 || point[0] > 800
        if $playfield_buffer.get([point[0], point[1]]) != [0,0,0,255]
            return true
        end
        false
    end

end