
# ajankäyttö: 4h 20min


$bg = [182,180,178]
$discs = 7


require "RUDL"
include RUDL
include Constant


# This exception is raised when we want to quit the game.
# It is inherited from Exception, so a mere "rescue" won't catch it.
class Quit < Exception
end


class HanoiTowers
    # methods to set the next page
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
        @win = DisplaySurface.new [510,300]
        @win.set_caption "Hanoi Blocks"
        @win.set_icon(Surface.load_new("icon.png"))

        # first up: show the logo
        logo()

        # next up: what the previous page wanted!
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


    # page: display the startup screen
    def logopage

        # display the logo here and wait for a keypress,
        # or there could be a menu or something

        # after this: start the game
        game()

    end


    # page: run the game
    def gamepage
        win = @win

        # create poles with some discs in the middle one
        poles = [Pole.new(0), Pole.new($discs), Pole.new(0)]

        poles.each {|p| p.window = win }

        def poles.draw
            each_with_index do |p, i|
                p.draw(20 + i*160, 80)
            end
        end

        # fill background, draw the black "floor" and poles
        win.fill $bg
        win.fill [0,0,0], [10,180,490,10]
        poles.draw
        win.update

        # selecteddisc is a reference to the disc that has been selected by clicking on it
        selecteddisc = nil

        loop do
            # game is finished if the left or right pole has all the discs
            break if poles[0].size == $discs or poles[2].size == $discs

            # wait for an event
            ev = EventQueue.wait
            case ev
                when QuitEvent
                    raise Quit

                when MouseMotionEvent
                    # first, clear any hovering status (but keep selecteddisk highlighted)
                    Pole.clear_hover(selecteddisc)

                    # if the mouse is over a pole, tell the pole to highlight itself or the topmost disc
                    if (p = Pole.which(ev.pos))
                        poles[p].hover(selecteddisc)
                    end

                    poles.draw
                    win.update

                when MouseButtonDownEvent, KeyDownEvent
                    # left button clicked on a pole?
                    if ev.is_a? MouseButtonDownEvent
                        p = Pole.which(ev.pos) if ev.button == 1
                    else
                        case ev.key
                            when K_h
                                p = 0
                            when K_j
                                p = 1
                            when K_k
                                p = 2
                            else
                                p = nil
                        end
                    end

                    if p
                        # if there's a disc already selected, move it to this pole
                        if selecteddisc
                            # (if it's not allowed, it will do nothing)
                            selecteddisc.moveto(poles[p])
                            # deselect it in any case
                            selecteddisc = nil
                        else
                            # otherwise, select the topmost disc of this pole
                            selecteddisc = poles[p].topmost
                            # hilite it (or if the pole was empty, topmost returns nil)
                            selecteddisc.hilite = true if selecteddisc
                        end
                    end

                    Pole.clear_hover(selecteddisc)
                    poles.draw
                    win.update

            end
        end

        # after it's over, proceed to the game over page
        gameover()
    end


    # page: show "game over"
    def gameoverpage

        @win.print [100,250], "Game finished!", [255,255,0]
        @win.update

        while EventQueue.wait.class != QuitEvent; end

        # return to the menu
        logo()
    end


    # for debugging: in case a page doesn't specify the next page
    def errorpage
        raise Exception, "errorpage"
    end

end


class Pole
    attr_accessor :hilite

    @@poles = []

    # initialize the pole, filling it with the wanted amount of Discs
    def initialize(numdiscs = 0)
        @@poles.push self
        @discs = []
        numdiscs.downto(1) do |i|
            push(Disc.new(1+2*i))
        end
    end

    # get the number of discs on this pole
    def size
        @discs.size
    end

    # push a disc on top of this pole
    def push(disc)
        @discs.push disc
        disc.pole = self
    end

    # remove disc from this pole
    def remove(disc)
        @discs.delete disc
    end

    # get the topmost disc (or nil if empty)
    def topmost
        if @discs.size > 0
            @discs[-1]
        else
            nil
        end
    end

    # tell this pole which DisplaySurface to draw itself in
    def window=(win)
        @win = win
        @discs.each do |d|
            d.window = win
        end
    end

    # tell this pole to highlight itself (if there's a selected disc elsewhere and it's
    # smaller than the topmost disc on this pole), or the topmost disc
    def hover(selecteddisc)
        if selecteddisc
            @hilite = true if topmost == nil or topmost.size > selecteddisc.size
        else
            topmost.hilite = true if topmost
        end
    end

    # clear highlights from all existing poles and their topmost discs (but not
    # from the selected disc)
    def Pole.clear_hover(selecteddisc = nil)
        @@poles.each do |p|
            p.hilite = false
            d = p.topmost
            d.hilite = false unless d == selecteddisc or d == nil
        end
    end

    # check which pole (0..2) the coordinate is on, if any
    def Pole.which(pos)
        if pos.y > 200 or pos.y < 10
            return nil
        end
        w = (pos.x-15) / 160
        if w.between?(0, 2)
            return w
        else
            return nil
        end
    end

    # draw the pole and all discs
    def draw(x, y)
        @win.fill $bg, [x,y,150,100]
        if hilite
            color = [0,255,50]
        else
            color = [0,0,0]
        end
        @win.fill color, [x+70,y,10,100]

        @discs.each_with_index do |d, i|
            d.draw(x, y + 90 - i*10)
        end
    end

end


class Disc
    attr_reader :size
    attr_accessor :hilite, :pole

    @@discs = []

    def initialize(size)
        @size = size
        @@discs.push self
    end

    def window=(win)
        @win = win
    end

    # move this disc to the destination pole, if the rules allow it
    def moveto(destpole)
        t = destpole.topmost
        if t and t.size <= @size
            return false
        end
        @hilite = false
        @pole.remove self
        destpole.push self
        true
    end

    def draw(x, y)
        if @hilite
            color = [240,240,0]
        else
            color = [50,100,255]
        end
        @win.fill color, [x + (15-size)*5, y+1, size*10, 9]
    end

end




HanoiTowers.new.run



