# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
# See comments in barrage.rbw

require 'utils/alg3d';  include GLIT

# base class for physics particles
class ParticleSystem
  
    def initialize
        @particles = []
    end
  
    def any_projectiles?
        @particles.each { |p|
            return true if p.respond_to?(:projectile)
        }
        false
    end

def line part1, part2
    points = []
    part1 = Vec::new [part1[0].to_i, part1[1].to_i]
    part2 = Vec::new [part2[0].to_i, part2[1].to_i]
    while part1[0] != part2[0] || part1[1] != part2[1]
        part1[0] += 1 if part1[0] < part2[0]
        part1[0] -= 1 if part1[0] > part2[0]
        part1[1] += 1 if part1[1] < part2[1]
        part1[1] -= 1 if part1[1] > part2[1]
        points << part1.copy
    end
    points
end
  
    def kill part
        @particles.delete part
    end
  
    def do_collisions playfield
        @particles.each { |part|
            if part.collide
                if part.explode playfield, @particles
                    @particles.delete part
                end
            end
            part.collide = false
        }
    end
  
    def << part
        @particles << part
    end
  
    def each
        @particles.each { |p| yield p }
    end
  
    def erase
        @particles.each { |part|
            if part.size == 1
                $backbuffer[part.pos[0], part.pos[1]] = $background_buffer[part.pos[0], part.pos[1]]
            else
                half = part.size/2
                part_pos = [part.pos[0]-half, part.pos[1]-half]
                part_rect = [part.pos[0]-half, part.pos[1]-half, part.size, part.size]
                $backbuffer.blit $background_buffer, part_pos, part_rect
                $backbuffer.blit $playfield_buffer, part_pos, part_rect
            end
        }
    end
  
    def draw
        @particles.each { |part|
            if part.size == 1
                $backbuffer[part.pos[0], part.pos[1]] = part.color
            else
                half = part.size/2
                $backbuffer.fill part.color, [part.pos[0]-half, part.pos[1]-half, part.size, part.size]
            end
        }
    end

end

class Particle

    DIRT = 1
    APEX = 2
    TANK = 50

    attr_accessor :pos, :bounce, :vel, :friction, :color, :size, :collide, :collide_with

    def explode playfield, particles
        return false if @collide_with == APEX
        true
    end
    
    def initialize pos, sz = 1, vel = [0,0], bounce = 0.4, friction = 0.2, color = [255, 255, 255]
        @pos = Vec::new [pos[0].to_f, pos[1].to_f]
        @size = sz
        @vel = Vec::new [vel[0].to_f, vel[1].to_f]
        @bounce = bounce
        @friction = friction
        @color = color
        @collide = false
    end
  
    def draw
        GL::Color3f @color[0], @color[1], @color[2]
        GL::PointSize 3.0
        GL::Begin GL::POINTS
        GL::Vertex2f @pos[0], @pos[1]
        GL::End()
    end
  
end

class Dirt < Particle

    def initialize pos, vel = [0,0], color = [255,255,255], sz = 2
        super pos, sz, vel, 0, 0.1, color
    end

    def explode playfield, particles
        return false if @collide_with == APEX
        playfield.add_dirt @pos, @size, @color
        true
    end
  
end

class Projectile < Particle

    attr_accessor :low_dirt

    def initialize pos, vel = [0,0], color = [255,255,255], radius = 5, low_dirt = false
        @radius = radius
        @low_dirt = low_dirt
        super pos, 3, vel, 1, 0.1, color
    end
    
    def projectile
    end
    
    def explode playfield, particles
        return false if @collide_with == APEX
        if @collide_with == DIRT
            playfield.remove_dirt @pos, @radius
            col = playfield.topsoil
            if @low_dirt
                dirt_amt = (@radius/5.0).floor
            else
                dirt_amt = (@radius*2.0).floor
            end
            dirt_amt.times {
                x_vel = (-@vel[0]*1.5)
                x_vel += rand()*2 if x_vel > 0
                x_vel -= rand()*2 if x_vel < 0
                x_vel += rand()*2-1 if x_vel == 0
                particles << Dirt::new([@pos[0]+rand(4)-2,@pos[1]+rand(4)-2], [x_vel,@vel[1]/3+rand()*2], col)
            }
        elsif @collide_with >= TANK
            tank = @collide_with - TANK
            $tanks[tank].slide(@vel[0]/10.0) if @low_dirt
            $tanks[tank].slide(@vel[0]) if not @low_dirt
            $tanks[tank].health -= @radius/2
        end
        true
    end
  
end

class DirtClearingProjectile < Projectile
    def explode playfield, particles
        return false if @collide_with == APEX
        if @collide_with >= TANK
            playfield.remove_dirt @pos, (@radius*1.5).floor
        else
            playfield.remove_dirt @pos, @radius
        end
        true
    end
end

class SplitProjectile < Projectile

    def initialize super_strength, *args
        super(*args)
        @super_strength = super_strength
    end

    def explode playfield, particles
        i = @super_strength ? 10 : 5
        rad = @super_strength ? 30 : 20
        i.times {
            @vel[0] += (rand(11)-5)/7.2
            particles << Projectile::new(@pos, @vel, [255,150,255], rad, true);
        }
        true
    end
end

class PusherProjectile < Projectile

    def explode playfield, particles
        return if @collide_with == APEX
        if  @collide_with >= TANK
            tank = @collide_with - TANK
            $tanks[tank].slide @vel[0]*4
            $tanks[tank].health -= @radius/6.0
        end
        true
    end
    
end