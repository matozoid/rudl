# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
# See comments in barrage.rbw

module MenuState
    safeCall
    
    # if a surface has an alpha channel the bitmap-wide alpha is ignored, and ttf.render returns an alpha channel,
    # so we have to get rid of it for our simple alpha effects
    def MenuState.noAlpha rendered
        tmp = Surface::new rendered.size, SWSURFACE, 24
        tmp.blit rendered, [0,0]
        rendered.destroy
        tmp
    end

    MAIN_TITLE_COLOR = [200,150,25]
    OPTION_TITLE_COLOR = [150, 75, 0]
    PLAY_GAME_COLOR = [150,255,255]
    OPTION_TITLE_X = 150
    OPTION_TITLE_Y_START = 120
    OPTION_TITLE_Y_DELTA = 75
    
    def MenuState.init
        $backbuffer.fill [0,0,0]
        @font = TrueTypeFont::new 'rsc/polarbear.ttf', 64
        @small_font = TrueTypeFont::new 'rsc/adlibn.ttf', 35
        @title = MenuState.noAlpha(@font.render("*Barrage*", true, MAIN_TITLE_COLOR))
        @title_alpha = 0
        @cur_selection = "Play Game"
        @help_text = "Press UP and DOWN to select an option, press ENTER to change it."
        @options_graphics = {}
        $game_options.each { |k|
            @options_graphics[k] = []
            if k == "Play Game"
                @options_graphics[k][0] = MenuState.noAlpha(@small_font.render(k, true, PLAY_GAME_COLOR))
            else
                @options_graphics[k][0] = MenuState.noAlpha(@small_font.render(k, true, OPTION_TITLE_COLOR))
            end
            $game_possibilities[k].each { |o|
                @options_graphics[k] << MenuState.noAlpha(@small_font.render(o.to_s, true, OPTION_TITLE_COLOR))
            }
        }
    @cur_changed = true
    Key::set_repeat 0, 0
    end
    
    def MenuState.shutdown
        @title.destroy
        $game_options.each { |k|
            @options_graphics[k].length.times { |i|
                @options_graphics[k][i].destroy
            }
        }
        @font = @small_font = nil
    end
    
    def MenuState.changeHelpText
        @help_text = $game_options_help[@cur_selection]
        @cur_changed = true
        $backbuffer.fill [0,0,0]
    end
    
    def MenuState.keyDown k
        case k
            when K_ESCAPE
            $timer.exit
            when K_RETURN
            cur_choice = $game_option_values[@cur_selection]
            new_choice = $game_possibilities[@cur_selection].index(cur_choice)
            new_choice += 1
            new_choice = 0 if new_choice == $game_possibilities[@cur_selection].length
            $game_option_values[@cur_selection] = $game_possibilities[@cur_selection][new_choice]
            MenuState.changeHelpText
            changeState(GameState) if @cur_selection == "Play Game"
            when K_UP
            idx = $game_options.index(@cur_selection)
            idx -= 1
            idx = $game_options.length-1 if idx < 0
            @cur_selection = $game_options[idx]
            MenuState.changeHelpText
            when K_DOWN
            idx = $game_options.index(@cur_selection)
            idx += 1
            idx = 0 if $game_options[idx] == nil
            @cur_selection = $game_options[idx]
            MenuState.changeHelpText
        end
    end
    
    def MenuState.run
        @title_alpha += 4 if @title_alpha < 255
        @title_alpha = 255 if @title_alpha > 255
    end
    
    def MenuState.draw
        if @cur_changed
            screen = $backbuffer
            y = OPTION_TITLE_Y_START
            $game_options.each { |option|
                alpha = 128
                alpha = 255 if @cur_selection == option
                @options_graphics[option][0].set_alpha alpha
                if option == "Play Game"
                    screen.blit @options_graphics[option][0], [400-@options_graphics[option][0].w/2, y]
                    y += OPTION_TITLE_Y_DELTA/2 # 1.5 spacing
                else
                    screen.blit @options_graphics[option][0], [OPTION_TITLE_X, y]
                    idx = $game_possibilities[option].index($game_option_values[option]) + 1
                    @options_graphics[option][idx].set_alpha alpha
                    screen.blit @options_graphics[option][idx], [500, y]
                end
                y += OPTION_TITLE_Y_DELTA
            }
            @cur_changed = false
        end
    end

    def MenuState.draw_overlay screen
        @title.set_alpha @title_alpha
        screen.blit @title, [400-(@title.w/2),5]
        help_text_len = @help_text.length * 8
        screen.print [400-(help_text_len/2), 580], @help_text, [255,255,255,128]
    end
    
end