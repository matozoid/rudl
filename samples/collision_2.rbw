#-- An "almost" literal translation from the SGE's
#-- collision.cpp sample, adapted for RUDL's Bitmask.
#--                  [- nauglin -]

require 'RUDL'; include RUDL; include Constant

screen = DisplaySurface.new( [640,480], SWSURFACE, 16 )
screen.set_caption( 'Collision', 'collision' )

screen.fill( 0 )

#-- THE BUFFER FOR THE BALL
img1 = Surface.new( [30,30], SWSURFACE|SRCCOLORKEY, 16, [0,0,0,0] )
img1.fill( 0 )
img1.filled_circle( [15,15], 14, [150,200,50] )
img1.set_colorkey( 0, SRCCOLORKEY|RLEACCEL )

#-- DRAW A BACKGROUND
screen.fill( [0,0,255] ,[300,80, 40,320] )
screen.fill( [0,0,255] ,[0,360, 200,40] )
screen.fill( [0,0,255] ,[340,260, 210,40] )
screen.fill( [0,0,255] ,[100,80, 40,280] )
screen.fill( [0,0,255] ,[500,80, 50,50] )
#-- buffer=SDL_DisplayFormat(screen); //The buffer for the background
buffer = Surface.new( screen.size, screen )
buffer.blit( screen, [0,0] )
screen.flip

#-- MAKE THE COLLISION MAPS
cimg1 = CollisionMap.new( img1 )

buffer.set_colorkey( 0 )
cbg = CollisionMap.new( buffer )
buffer.unset_colorkey


#-- If the delay between two PollEvent is greater than 100 and all events are added to the
#-- event queue, the queue will grow *fast*. You're program will have no change to catch
#-- up. You *must* try to avoid adding events unnecessarily - mousemotions for example.
#-- SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
# EventQueue.blocked = MouseMotionEvent

x,y = 30,10
oldx,oldy = x,y
right,down = true,true

#-- sge_Update_OFF();
screen.unlock

#-- SETS START TIME
loops = 0
tstart = Timer.ticks

#-- MAIN LOOP
while true
        Timer.delay 10

        #-- CHECK BORDERS
        if x + img1.w >= screen.w - 1
                right=false
        elsif x <= 1
                right=true
        end
        if y + img1.h >= screen.h - 1
                down=false
        elsif y <= 1
                down=true
        end

        #-- CHECK FOR COLLISION
        cxy = cbg.collides_with( [0,0], cimg1, [x,y] )
        if cxy
                if (cxy[0]-(x+img1.w/2)).abs < (cxy[1]-(y+img1.h/2)).abs
                        if cxy[1]-(y+img1.h/2) < 0
                                down=true
                        else
                                down=false
                        end
                else
                        if cxy[0]-(x+img1.w/2) < 0
                                right=true
                        else
                                right=false
                        end
                end
        end

        #-- UPDATE POS
        if right
                x+=1
        else
                x-=1
        end
        if down
                y+=1
        else
                y-=1
        end

        #-- UPDATE LAST POS FROM BUFFER
        screen.blit( buffer, [oldx,oldy], [oldx,oldy, img1.w , img1.h] )

        #-- DRAW THE CIRCLE (...THE LAST RECT IS ACTUALLY UNNECESSARY IN RUDL...)
        screen.blit( img1, [x,y], [0,0, img1.w , img1.h] )

        oldx,oldy = x,y

        #-- UPDATE SCREEN
        screen.lock
        screen.update( [[x-1, y-1, img1.w+2, img1.h+2]] )
        screen.unlock

        loops+=1

        #-- CHECK EVENTS
        event = EventQueue.poll
        break if event.type==KeyDownEvent or event.type==QuitEvent
end

#-- PRINT FPS
tstart = Timer.ticks - tstart
screen.print( [10,10], "#{ (loops*1000.0)/tstart } FPS (target: 100)", [255,255,0] )
screen.flip
true until EventQueue.poll.type == KeyUpEvent
