
# Mauno Mato v1.0 (22 Dec 2002)
# © Renne Nissinen <rennex@iki.fi>
# http://www.iki.fi/rennex/

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# The GNU General Public License can be found at
# http://www.gnu.org/copyleft/gpl.html


# This is my first RUDL game. I had done some experimenting with
# RUDL, and one day I just sat down and decided to write a little
# worm game with it. I decided to release it because there doesn't
# seem to be many example games for RUDL. I hope you get something
# out of it! I think the code turned out rather neat.


# Some words about the code. There's a block of settings at the
# beginning where you could change the window size, colors and
# such things, as well as disable the "u-turn protection".

# The worm is an array of [x,y] coordinates, one for each block (or
# "slice") of the worm. The first item in the array is the head's
# coordinates, and the last item is the tail. The worm is moved into
# its new position by inserting a new block for the head and
# discarding the tail block. We only need to erase the tail block
# on screen, because we're going to redraw all the other blocks of
# the worm. Then we need to redraw the length text on screen in case
# the worm is moving under it.

# Just before drawing a new frame the event queue is checked for
# interesting events such as keypresses. Arrow keypresses are
# queued in an Array so that quick moves would not be lost.
# After the worm has been moved to its new position, we check if
# it just ate the food. If it did, we grow the worm and find a new
# place for the food where the worm is not already on it.

# The worm's speed depends on how long it is; the formula can be
# seen in Worm#speedupdate. In the beginning it moves 7.4 times
# per second, and when it's 100 blocks long it moves 21 times per
# second. This is getting pretty difficult to control, and the
# longer it gets, the slower its speed increases. After each frame,
# we achieve the required delay by repeating 1ms delays until the
# correct amount of time has passed. We do this because a single
# call to Timer.delay() is less accurate.


# That's all I can think of about the source code. Please drop me
# a line at rennex@iki.fi and tell me what you think of this game!
# And see the home page for more RUDL puzzle games in the future.


# History
# 0.1:  * everything before 1.0
#
# 1.0:  * works at least on my windows machine
#       * rewrote some unclear comments
#       * reorganized some code snippets, moved main game code to the top
#       * no longer erases the whole window every frame
#
# 1.1:  * added the page system
#       * implemented the game over page
#       * came up with the final name for the game :)
#       * misc cleaning up


# --- Settings ---
# screen size in blocks
$screenw, $screenh = 40, 30

# thickness of the worm (in pixels)
$wormsize = 10

# colors
$bgcolor = [153, 153, 102]
$wormcolor = [0, 0, 0]
$deadcolor = [255, 0, 0]
$foodcolor = [0, 0, 255]
$textcolor = [255, 255, 0]

# worm is a color slide?
$fancyworm = true

# allow turning back (which leads to instant death)?
$allow_u_turn = false

# ----------------

$verstring = "1.1"

require "RUDL"
include RUDL
include Constant


# This exception is raised when we want to quit the game.
# It is inherited from Exception, so a mere "rescue" won't catch it.
class Quit < Exception
end


class WormGame

    @@logo = nil

    # methods that set what the game should do when
    # the previous page finishes
    def logo
        @page = self.method(:logopage)
    end
    def game
        @page = self.method(:gamepage)
    end
    def gameover
        @page = self.method(:gameoverpage)
    end


    def run
        # open the window
        @win = DisplaySurface.new [$screenw*$wormsize, $screenh*$wormsize]
        @win.set_caption "Maggot v#$verstring © Renne Nissinen <rennex@iki.fi>"

        # first up: show the logo
        logo()

        # next up: whatever the previous page says to do next!
        begin
            loop do
                page = @page
                # if the page doesn't set the next page, it's an error!
                @page = method(:errorpage)
                page.call
            end
        rescue Quit
        end
    end


    # page: display the logo if it's found
    def logopage
        begin
            unless @@logo
                @@logo = Surface.load_new("maggotlogo.png")
            end

            # blit to the upper left corner - the window size isn't meant to be changed :)
            @win.blit @@logo, [0,0]
            @win.flip

            # wait for a key press before moving on
            loop do
                ev = EventQueue.wait
                raise Quit if ev.is_a? QuitEvent
                break if ev.is_a? KeyDownEvent or ev.is_a? MouseButtonDownEvent
            end

        rescue
            # in case the logo picture isn't found, ignore the error
        end

        # next up: the actual game
        game()

    end


    # page: run the game
    def gamepage
        win = @win

        # initialize our worm and food etc
        worm = Worm.new
        worm.window = win
        food = createfood(worm)
        dir = nil
        dirq = []

        # fill the window with the background color
        win.fill($bgcolor)

        # the moment we started displaying the previous frame
        prevframe = Timer.ticks

        # main game loop
        loop do
            # is the worm still alive?
            break unless worm.alive

            # anything happening?
            EventQueue.get.each do |ev|
                raise Quit if ev.is_a? QuitEvent

                if ev.is_a? KeyDownEvent
                    # this line sets the direction if an arrow key was pressed, else nil
                    dir = { K_UP => :u, K_DOWN => :d, K_RIGHT => :r, K_LEFT => :l }[ev.key]
                    # queue each arrow keypress
                    dirq.push dir if dir
                end
            end

            # erase the part of the window where the length is printed
            win.fill($bgcolor, [70,0,30,20])

            # move the worm and draw it in the new position
            worm.move(dirq.shift)

            # did it eat the food?
            if worm.pos == food
                worm.grow
                food = nil
            end

            # if there's no food on screen, create more
            if not food
                food = createfood(worm)
            end

            # draw the food (yes, every frame, in case it's under the text)
            ws = $wormsize
            win.fill($foodcolor, [food.x*ws, food.y*ws, ws, ws])

            # print the score (worm length) on screen
            # (yes, every frame, in case the worm is moving under it :)
            win.print [8,8], "Length: #{worm.len}", $textcolor

            win.flip
            dir = nil

            # wait until the next movement
            Timer.delay(1) while (now=Timer.ticks) - prevframe < worm.frametime
            prevframe = now

        end # loop

        gameover()
    end


    # page: show "game over"
    def gameoverpage
        # little delay for no reason
        Timer.delay(100)

        # print one line a bit above center, and another a bit below
        centerprint("Game over!", -6)
        centerprint("Press spacebar to restart", +6)
        @win.flip

        Timer.delay(100)

        # clear the event queue
        EventQueue.get

        # wait for a keypress
        loop do
            ev = EventQueue.wait
            raise Quit if ev.is_a? QuitEvent
            break if ev.is_a? KeyDownEvent and ev.key == K_SPACE
        end

        # next, run the game again
        game()
    end


    # for debugging: in case a page doesn't specify the next page
    def errorpage
        puts "Oh God please no!"
        raise Exception, "errorpage"
    end


    # this generates random coordinates that are "off" the worm
    def createfood(worm)
        while true
            x, y = (rand*$screenw).to_i, (rand*$screenh).to_i
            break unless worm.collides_with?(x, y)
        end
        [x,y]
    end

    # print a text in the center of the window
    # (or above it or below)
    def centerprint(text, *yoffset)
        offset = 0
        offset = yoffset[0] unless yoffset.empty?
        @win.print [@win.w/2 - text.length*4, @win.h/2 - 4 + offset], text, $textcolor
    end


end # class WormGame




class Worm
    @@size = $wormsize

    attr_reader :speed, :pos, :frametime, :alive, :len

    def initialize
        # we're alive!
        @alive = true

        # initial length (blocks)
        @len = 4
        # calculate moving speed based on the length
        speedupdate()

        # starting position (of the head)
        startx, starty = $screenw/2, $screenh/2
        @pos = [startx, starty]

        # coordinates of our slices (first = head, last = tail)
        @slices = []
        @len.times do |i|
            @slices.push [startx - i, starty]
        end

        # direction (:r, :l, :u, :d)
        @dir = :r
    end

    # set where we shall draw this worm
    def window=(win)
        @win = win
    end

    # update the worm speed to match its length
    def speedupdate
        @speed = 4 + 1.7*Math.sqrt(@len)
        @frametime = 1000/@speed
    end

    # grow the worm by 1 and adjust its speed
    def grow
        @slices.push @slices[-1].dup
        @len += 1
        speedupdate()
    end

    # move the worm and draw it
    def move(dir)
        # update our direction
        if dir
            if $allow_u_turn
                @dir = dir
            else
                opposite = { :r => :l, :l => :r, :d => :u, :u => :d }
                @dir = dir unless dir == opposite[@dir]
            end
        end

        # see where our head goes
        case @dir
            when :r then @pos.x += 1
            when :l then @pos.x -= 1
            when :d then @pos.y += 1
            when :u then @pos.y -= 1
        end

        # if we're out of the area, clip to the opposite edge
        @pos.x %= $screenw
        @pos.y %= $screenh

        # add our head slice to the beginning and remove the last slice
        @slices.unshift @pos.dup
        tail = @slices.pop
        @win.fill($bgcolor, [tail.x*@@size, tail.y*@@size, @@size,@@size])

        # draw all the slices
        @slices.each_with_index do |coord, i|
            # choose the square color
            if $fancyworm
                c = (1 - i/(@len - 1.0)) * 255
                color = [c,c,c]
            else
                color = $wormcolor
            end

            # did we collide into ourself?
            if coord == @pos and i != 0
                @alive = false
                color = $deadcolor
            end

            # draw the slice
            @win.fill(color, [coord.x*@@size, coord.y*@@size, @@size, @@size])
        end

    end

    # this method checks if the given coords are occupied by the worm
    def collides_with?(x, y)
        @slices.each do |coord|
            return true if coord == [x,y]
        end
        return false
    end

end


# start the game!
WormGame.new.run
