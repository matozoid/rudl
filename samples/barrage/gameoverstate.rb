# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
# See comments in barrage.rbw
#
# This is the least complete part of the program.

module GameOverState
    safeCall
    
    WIN_COLOR = [200,150,25]

    def GameOverState.init
        $backbuffer.fill [0,0,0]
        @font = TrueTypeFont::new 'rsc/polarbear.ttf', 60
        @title = MenuState.noAlpha(@font.render("*#{$tanks[0].name} wins*",true,WIN_COLOR))
        @title_alpha = 0
        @tank_dest = [400, 250]
        @at_pos = 0
        $tanks[0].pos[0] = $tanks[0].pos[0].to_i
        $tanks[0].pos[1] = $tanks[0].pos[1].to_i
    end
    
    def GameOverState.shutdown
        @title.destroy
        @font = nil
    end
    
    def GameOverState.run
        @title_alpha += 4 if @title_alpha < 255
        @title_alpha = 255 if @title_alpha > 255
        3.times {
            $tanks[0].pos[0] += 1 if $tanks[0].pos[0] < @tank_dest[0]
            $tanks[0].pos[0] -= 1 if $tanks[0].pos[0] > @tank_dest[0]
            $tanks[0].pos[1] += 1 if $tanks[0].pos[1] < @tank_dest[1]
            $tanks[0].pos[1] -= 1 if $tanks[0].pos[1] > @tank_dest[1]
            @at_pos += 1 if (($tanks[0].pos[0] == @tank_dest[0]) and ($tanks[0].pos[1] == @tank_dest[1]))
        }
        changeState(MenuState) if @at_pos > 450
    end
    
    def GameOverState.draw
        $tanks[0].draw
    end
    
    def GameOverState.erase
        $tanks[0].erase
    end
    
    def GameOverState.draw_overlay screen
        @title.set_alpha @title_alpha
        screen.blit @title, [400-(@title.w/2),25]
    end

end