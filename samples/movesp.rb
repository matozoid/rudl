=begin
@file Samples
@class MoveSp
(This was written by Nauglin)

This is a porting from the Ohai's script to RUDL. The Ohai's original 
code is at the end. I've tried this to be the most similar as posible 
to the original, but RUDL and Ruby/SDL are different approaches to 
SDL, so...
=end


require 'RUDL'; include RUDL; include Constant

screen = DisplaySurface.new( [640,480], SWSURFACE, 16 )

image = Surface.load_new( 'media/icon.gif' )
image.set_colorkey( image[0,0], RLEACCEL )

$image = image.convert     #-- I believe RUDL can do this directly on 
                           #-- the surface itself, but I'm not sure. 
                           #-- Anyway, this way the comparing 
                           #-- should be more reliable.

srand 10     #-- This is for always having the same random numbers, 
             #-- so that speed comparing makes any sense.

class Sprite
  def initialize
    @x=rand(640)
    @y=rand(480)
    @dx=rand(11)-5
    @dy=rand(11)-5
  end
  
  def move
    @x += @dx
    if @x >= 640 then
      @dx *= -1
      @x = 639
    end
    if @x < 0 then
      @dx *= -1
      @x = 0
    end
    @y += @dy           #-- I wonder why Ohai does this twice...
    if @y >= 480 then
      @dy *= -1
      @y = 479
    end
    @y += @dy           #-- ...but only in the Y axis ??
    if @y < 0 then
      @dy *= -1
      @y = 0
    end
  end
  
  def draw(screen)
    screen.blit( $image, [@x,@y], [0,0,32,32] )
  end
  
end

sprites = []
for i in 1..5
  sprites << Sprite.new
end

class MovableSp
  def initialize()
    @ud=@lr=0;
  end
  
  def move()
    @ud=@lr=0;
    @lr=-1 if Key.pressed?[K_h] or Key.pressed?[K_LEFT]
    @lr=1  if Key.pressed?[K_l] or Key.pressed?[K_RIGHT]
    @ud=1  if Key.pressed?[K_j] or Key.pressed?[K_DOWN]
    @ud=-1 if Key.pressed?[K_k] or Key.pressed?[K_UP]
  end
  
  def draw(screen)
    screen.blit( $image, [300+@lr*50,200+@ud*50], [0,0,32,32] )
    screen.flip # comment this out for comparing speed
  end
end

sprites << MovableSp.new

event = EventQueue.pump   #-- Just to put something with sense where 
                          #-- Ohai did  SDL::Event.new

fps = 0           #-- These two are for calculating the frames per 
t = Timer.ticks   #-- second. Not in the original.

while true   #-- Start the main loop

   #-- Next is a variable assignment not so evident in the Ohai's
   #-- original. Of course, Ruby/SDL should also do this assignment
   #-- internally anyway. Maybe this is more time consuming, as is
   #-- Ruby instead inner C, but it doesn't seem to slow things visibly.
   #
   #-- I still think this could be a bit more clear:
   #
   #-- if event = EventQueue.poll
   #--   break if event.class == QuitEvent
   #--   break if event.class == KeyDownEvent and event.key == K_ESCAPE
   #-- end
   #
   #-- But I'm trying to duplicate Ohai's code as much as I can.

  if  event = EventQueue.poll
    if event.class == QuitEvent
      break
    end
    if event.class == KeyDownEvent
      break if event.key == K_ESCAPE
    end
  end
  
   #-- This would work the same in RUDL without the Rect.
  screen.fill( 0, [0,0,640,480] )
  
   #-- This is to keep the keyboard queue up to date.
   #-- I hope it being similar to Ohai's  Key.scan
  EventQueue.pump            
  
  sprites.each {|i|
    i.move
    i.draw(screen)
  }

   #-- This would work the same in RUDL without the arg.
  screen.update( [ [0,0,0,0] ] )

   #-- This is for counting the frames.
  fps += 1

end

#-- These are not from the original. They are the fps calculation.
t = Timer.ticks - t
screen.set_caption "#{fps/(t/1000.0)} fps"

#-- Just a wait before quitting...
true while EventQueue.poll
true until [QuitEvent, KeyDownEvent].include? EventQueue.poll.class



__END__

#---------------------------------------------------------------------
#This is the Ohai's original code.  I've put some minnor changes for 
#testing speed, but they're commented. Just uncomment'em to get fps 
#calculation.   [-nauglin-]
#---------------------------------------------------------------------


require 'sdl'

SDL.init( SDL::INIT_VIDEO )

screen = SDL::setVideoMode(640,480,16,SDL::SWSURFACE)

image = SDL::Surface.loadBMP("icon.bmp")
image.setColorKey( SDL::SRCCOLORKEY || SDL::RLEACCEL ,0)
$image = image.displayFormat

#srand 10   #-- Uncomment to always test with the same randomnes.

class Sprite
  def initialize
    @x=rand(640)
    @y=rand(480)
    @dx=rand(11)-5
    @dy=rand(11)-5
  end
  
  def move
    @x += @dx
    if @x >= 640 then
      @dx *= -1
      @x = 639
    end
    if @x < 0 then
      @dx *= -1
      @x = 0
    end
    @y += @dy
    if @y >= 480 then
      @dy *= -1
      @y = 479
    end
    @y += @dy
    if @y < 0 then
      @dy *= -1
      @y = 0
    end
  end
  
  def draw(screen)
    SDL.blitSurface($image,0,0,32,32,screen,@x,@y)
  end
  
end

sprites = []
for i in 1..5
  sprites << Sprite.new
end

class MovableSp
  def initialize()
    @ud=@lr=0;
  end
  
  def move()
    @ud=@lr=0;
    @lr=-1 if SDL::Key.press?(SDL::Key::H) or SDL::Key.press?(SDL::Key::LEFT)
    @lr=1  if SDL::Key.press?(SDL::Key::L) or SDL::Key.press?(SDL::Key::RIGHT)
    @ud=1  if SDL::Key.press?(SDL::Key::J) or SDL::Key.press?(SDL::Key::DOWN)
    @ud=-1 if SDL::Key.press?(SDL::Key::K) or SDL::Key.press?(SDL::Key::UP)
  end
  
  def draw(screen)
    SDL.blitSurface($image,0,0,32,32,screen,300+@lr*50,200+@ud*50)
  end
end

sprites << MovableSp.new

event=SDL::Event.new

#fps = 0           #-- Uncoment to test frames per second.
#t = SDL.getTicks  #-- This one too.

while true 
  if  event.poll != 0 then
    if event.class==SDL::Event::QUIT then
      break
    end
    if event.class==SDL::Event::KEYDOWN then
      break if event.keySym==SDL::Key::ESCAPE
    end
  end
  screen.fillRect(0,0,640,480,0)
  SDL::Key.scan
  
  sprites.each {|i|
    i.move
    i.draw(screen)
  }
  screen.updateRect(0,0,0,0)
  #fps += 1  #-- Uncomment to count frames.
end

#-- Uncomment next lines to get fps result.
#t = SDL.getTicks - t
#SDL::WM::setCaption "#{fps/(t/1000.0)} fps", $0
#sleep 3
