#!/usr/bin/env ruby

require 'RUDL'; include RUDL; include Constant

display=DisplaySurface.new [400, 250]

bitmap1=Surface.new [20,20]
bitmap2=Surface.new [40,40]

bitmap1.line [0,0], [20,20], [255,255,255]
bitmap1.line [10,10], [0,15], [255,255,255]
bitmap1.set_colorkey [0,0,0]
bitmap1.collision_map=CollisionMap.new(bitmap1)

bitmap2.ellipse [20,20], 19, 19, [255,255,255]
bitmap2.ellipse [20,20], 18, 19, [255,255,255]
bitmap2.ellipse [20,20], 19, 18, [255,255,255]
bitmap2.ellipse [20,20], 18, 18, [255,255,255]
bitmap2.set_colorkey [0,0,0]
bitmap2.collision_map=CollisionMap.new(bitmap2)

keys=nil

x,y=70,70

while true
    ev=EventQueue.poll
    exit if ev.is_a? QuitEvent

    Timer.delay(5)

    keys=Key.pressed?
    exit if keys[K_ESCAPE]
    x-=1 if keys[K_LEFT]
    x+=1 if keys[K_RIGHT]
    y-=1 if keys[K_UP]
    y+=1 if keys[K_DOWN]

    display.fill [0,0,0]
    display.blit bitmap1, [x,y]
    display.blit bitmap2, [100,100]

    if bitmap1.collision_map.collides_with([x,y], bitmap2.collision_map, [100,100])
        display.print [10,10], 'collision!', 0xffffffff
    end

    display.flip
end
