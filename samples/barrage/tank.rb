# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
# See comments in barrage.rbw

require 'RUDL';  include RUDL;  include Constant
require 'particle'

# Weapon Name | Projectile Init Code | Starting Amount
$weapons = []
$weapons[0] = ["Pea shooter", "Projectile::new(pos, vec, [255,255,255], 25)", 0]
$weapons[1] = ["Dirt Vac", "DirtClearingProjectile::new(pos, vec, [128,255,128], 60)", 2]
$weapons[2] = ["Barrage", "SplitProjectile::new(false, pos, vec, [255,255,255], 25)", 3]
$weapons[3] = ["Super Barrage", "SplitProjectile::new(true, pos, vec, [255,255,255], 25)", 1]
$weapons[4] = ["Battering Ram", "PusherProjectile::new(pos, vec, [128,128,255], 15)", 1]

class Tank
    HALF_LENGTH = 16
    HALF_HEIGHT = 10
    MAX_HEALTH = 100
    
    TANK_PIC = Surface::load_new "rsc/tank.png"
    
    attr_accessor :pos, :rect, :vel, :fire_angle, :fire_strength, :fire_direction, :health, :name, :weapons, :cur_weapon, :body_color
    
    def initialize xpos, name, body_color
        y = $playfield.flatten xpos
        @pos = [xpos, y]
        @fire_angle = 45
        @fire_strength = 10.0
        @fire_direction = 1
        @health = MAX_HEALTH
        @name = name
        @vel = [0, 0]
        @cur_weapon = 0
        @weapons = []
        $weapons.length.times { |i|
            @weapons[i] = $weapons[i][2]
        }
        @body_color = body_color
    end
    
    def slide vec
        @vel[0] += vec
    end
    
    def health= new_health
        @health = new_health
        if @health <= 0
            $tanks.delete self
        end
    end
  
    def x
        @pos[0]
    end
      
    def y
        @pos[1]
    end
    
    def flip
        @fire_direction *= -1
    end
      
    def power_inc delta
        @fire_strength += delta
        @fire_strength = 0.1 if @fire_strength < 0.1
        @fire_strength = 15.0 if @fire_strength > 15.0
    end
    
    def angle_inc delta
        @fire_angle += delta if @fire_direction < 0
        @fire_angle -= delta if @fire_direction > 0
        if @fire_angle > 90
            @fire_angle = 180-@fire_angle
            flip
        end
        @fire_angle = 1 if @fire_angle < 1
        @fire_angle = 90 if @fire_angle > 90
    end
    
    def weapon_inc
        @cur_weapon += 1
        @cur_weapon = 0 if @cur_weapon >= $weapons.length
    end

    def draw
        $backbuffer.blit TANK_PIC, [@pos[0]-HALF_LENGTH, @pos[1]-HALF_HEIGHT*2]
        $backbuffer.line [@pos[0]-HALF_LENGTH, @pos[1]], [@pos[0]+HALF_LENGTH, @pos[1]], @body_color
        $backbuffer.line [@pos[0]-HALF_LENGTH, @pos[1]-HALF_HEIGHT*2], [@pos[0]+HALF_LENGTH, @pos[1]-HALF_HEIGHT*2], @body_color
        $backbuffer.line [@pos[0]-HALF_LENGTH, @pos[1]], [@pos[0]-HALF_LENGTH, @pos[1]-HALF_HEIGHT*2], @body_color
        $backbuffer.line [@pos[0]+HALF_LENGTH, @pos[1]], [@pos[0]+HALF_LENGTH, @pos[1]-HALF_HEIGHT*2], @body_color
    end
    
    def erase
        @rect = [@pos[0]-HALF_LENGTH, @pos[1]-HALF_HEIGHT*2, HALF_LENGTH*2+1, HALF_HEIGHT*2+1]
        $backbuffer.blit $background_buffer, rect, rect
        $backbuffer.blit $playfield_buffer, rect, rect if curState == GameState
    end
  
end