# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
# See comments in barrage.rbw
#
# Where the magic happens.

module GameState
  safeCall
  
    def GameState.checkRandom idx, arr
        arr.length.times { |i|
            if i != idx
                return false if not (arr[i]+Tank::HALF_LENGTH*2 < arr[idx]-Tank::HALF_LENGTH*2 || arr[i]-Tank::HALF_LENGTH*2 > arr[idx]+Tank::HALF_LENGTH*2)
            end
        }
        true
    end
    
    def GameState.init
        @part = ParticleSystem::new
        
        @arrow = Surface::load_new "rsc/arrow_small.png"
        @arrow.set_colorkey [0,0,0], RLEACCEL
        @arrow_delta = 100
        @arrow_on = true
        
        Key::set_repeat 100, 15

        # ## Particle stress test ## #
        #~ 100.times do
        #~ @part << Dirt::new([rand(800), rand(200)], [rand(6)-3, rand(8)-7])
        #~ end
        # using 1.0 as delta allows an important optimization--you can skip all multiplications by @delta and (@delta^2)
        #~ @delta = 1.0
        @gravity = 0.18
        @half_gravity = @gravity / 2.0
        $playfield = Playfield::new
        $tanks = []
        
        # initialize tanks
        players = $game_option_values["Players"].to_i
        positions = []
        if $game_option_values["Player Starts"] == "Fixed"
            dist = 600/(players-1)
            positions[0] = 100
            positions[1] = 100+dist
            positions[2] = 100+dist*2
            positions[3] = 100+dist*3
        else # "Random"
            positions[0] = positions[1] = positions[2] = positions[3] = rand(600)+100
            positions.length.times { |i|
                positions[i] = rand(600)+100 until self.checkRandom(i, positions)
            }
        end
        players.times {|i|
            $tanks[i]=  Tank::new(positions[i], $player_names[i], $tank_colors[i])
            $tanks[i].fire_direction = (positions[i] < 400 ? 1 : -1)
        }

        @turn = rand(players)
        @entering_name = false
        @fired = false

        @help_text = "Press H for help"
        @help_pos = 600
        @help_dir = -1
        @help_pause = 85
        @help_max = 585
        
        @showing_help = false
        @big_help_pos = 600
        @big_help_max = 200
    end
    
    def GameState.shutdown
        $playfield.destroy
        @part = nil
        $backbuffer.fill [0,0,0]
    end
  
      # this is extremely slow!!!
    def GameState.tank_physics tank
        left = tank.pos[0]-Tank::HALF_LENGTH
        right = left + Tank::HALF_LENGTH*2
        check_slide = left if tank.vel[0] < 0
        check_slide = right if tank.vel[0] > 0
        if tank.vel[0] != 0.0
            $tanks.delete tank if tank.pos[1] > 610 or tank.pos[0] < Tank::HALF_LENGTH or tank.pos[0] > (800-Tank::HALF_LENGTH)
            @turn = 0 if @turn >= $tanks.length
            if $playfield.collide([check_slide, tank.pos[1]])
                tank.health -= tank.vel[0].abs*1.5
                tank.vel[0] = 0.0
            else
                tank.pos[0] += tank.vel[0]
                tank.vel[0] *= 0.55 if $playfield.collide([tank.pos[0], tank.pos[1]+1])
                tank.vel[0] = 0.0 if (tank.vel[0] > 0 && tank.vel[0] < 0.1 || tank.vel[0] < 0 && tank.vel[0] > -0.1)
            end
        end
        if $playfield.collide([left, tank.pos[1]+1]) || $playfield.collide([right, tank.pos[1]+1])
            tank.health -= tank.vel[1]*4
            tank.vel[1] = 0
        else
            tank.vel[1] += @gravity
            $tanks.delete tank if tank.pos[1] > 610 or tank.pos[0] < Tank::HALF_LENGTH or tank.pos[0] > (800-Tank::HALF_LENGTH)
            @turn = 0 if @turn >= $tanks.length
        end
        tank.pos[1] += tank.vel[1]
        
    end
    
    def GameState.tank_collide pos
        $tanks.length.times { |cur|
            return cur if $tanks[cur].rect.contains?([pos[0],pos[1]])
        }
        false
    end
    
    # incredibly, stupefyingly slow!!!
    def GameState.particle_physics part
        pos, vel = part.pos, part.vel
        if $playfield.collide(pos)
            part.collide = true
            part.collide_with = Particle::DIRT
            # draw a line (kinda...) between the current position and the previous position
            # then find the exact point of collision
            old_pos = pos.copy
            vel[1] -= @gravity
            old_pos[0] -= vel[0]
            old_pos[1] -= vel[1] + 0.5*@gravity
            pts = @part.line pos, old_pos
            pts.each { |pt|
                if $playfield.collide pt
                    pos[0] = pt[0]
                    pos[1] = pt[1]
                end
            }
        vel[1] = (-vel[1])*part.bounce
        vel[0] = vel[0]*part.friction
        elsif tank = GameState.tank_collide(pos)
            part.collide = true
            part.collide_with = Particle::TANK + tank
        else
            pos[0] += vel[0]
            pos[1] += vel[1] + 0.5*@gravity
            old_vel = vel[1]
            vel[1] += @gravity
            if (old_vel < 0 and vel[1] > 0)
                part.collide = true
                part.collide_with = Particle::APEX
            end
            if pos[0] < 0 || pos[0] > 800 || pos[1] > 600
                @part.kill part
            end
        end
    end

    # Please, please, _please_ don't use my physics engine.  See the comments in barrage.rbw
    def GameState.physics
        @part.each { |part|
            GameState.particle_physics part
        }
        @part.do_collisions $playfield
        $tanks.each { |tank|
            GameState.tank_physics tank
        }
    end
  
  def GameState.run
    if @fired and not @part.any_projectiles?
        @covered = false
        @turn += 1
        @turn = 0 if @turn >= $tanks.length
        @fired = false
        @covered = true if $playfield.collide([$tanks[@turn].x, $tanks[@turn].y-Tank::HALF_HEIGHT*2-1])
        @arrow_on = true
        @arrow_delta = 100
    end
    @help_dir = 1 if @help_pos < @help_max and @help_pause == 0
    @help_pause -= 1 if @help_pos < @help_max and @help_dir == -1
    @help_pos += @help_dir if @help_pos <= 600 and (@help_pause == 0 or @help_pause == 85)
    
    if @showing_help and @big_help_pos > @big_help_max
        @big_help_pos -= 4
    elsif not @showing_help and @big_help_pos < 600
        @big_help_pos += 8
    end
    
    if @arrow_on
        @arrow_delta -= 2
        @arrow_on = false if @arrow_delta < 60
    end
    
    if $tanks.length == 1
        changeState(GameOverState)
    end
    
    if $tanks.length == 0
        changeState(menuState)  # guess I'm not really prepared for this case... but it could happen
    end
  end
  
    def GameState.changePlayerName player
        return if not $tanks[player]
        @entering_name = player
        Key::set_repeat 500, 30
    end
    
    def GameState.keyUp k
        if not @entering_name
            case k
                when K_SPACE
                tank = $tanks[@turn]
                if not @fired and (tank.weapons[tank.cur_weapon] > 0 or tank.cur_weapon == 0)
                    @fired = true
                    angle_rad = tank.fire_angle*(Math::PI/180)
                    vec = [tank.fire_strength*Math::cos(angle_rad), -tank.fire_strength*Math::sin(angle_rad)]
                    vec[0]= -vec[0] if tank.fire_direction < 0
                    $playfield.remove_dirt([tank.x, tank.y-Tank::HALF_HEIGHT*2-1], 5)
                    pos = [tank.x, tank.y-Tank::HALF_HEIGHT*2-1]
                    @part << eval($weapons[tank.cur_weapon][1])
                    tank.weapons[tank.cur_weapon] -= 1
                end
                when K_w
                $tanks[@turn].weapon_inc if $game_option_values["Weapons"] == "Yes"
                when K_h
                @showing_help = !@showing_help
                when K_q
                changeState(MenuState)
                when K_1
                GameState.changePlayerName 0
                when K_2
                GameState.changePlayerName 1
                when K_3
                GameState.changePlayerName 2
                when K_4
                GameState.changePlayerName 3
                when K_x
                $tanks[@turn].flip
            end
        else
        end
    end
  
    def GameState.keyDown k
        if not @entering_name
            case k
                when K_r
                changeState(GameState) if not @entering_name
                when K_RIGHT
                $tanks[@turn].angle_inc 1 if not @fired
                when K_LEFT
                $tanks[@turn].angle_inc -1 if not @fired
                when K_DOWN
                $tanks[@turn].power_inc -0.1 if not @fired
                when K_UP
                $tanks[@turn].power_inc 0.1 if not @fired
                when K_ESCAPE
                $timer.exit if not @entering_name
            end
        else
            if k >= K_a and k <= K_z
                # add letter to player's name
                $tanks[@entering_name].name = '' if not $tanks[@entering_name].name
                chr = (?a + (k - K_a))
                chr += (?A - ?a) if (Key::modifiers & KMOD_SHIFT != 0)
                $tanks[@entering_name].name << chr.chr
            end
            if k == K_BACKSPACE
                $tanks[@entering_name].name = $tanks[@entering_name].name[0..-2] if $tanks[@entering_name].name != nil
            end
            if k == K_RETURN
                $player_names[@entering_name] = $tanks[@entering_name].name
                Key::set_repeat 100, 15
                @entering_name = false
            end
        end
    end
  
    def GameState.erase
        $tanks.each {|t| t.erase }
        @part.erase
    end
    
    def GameState.draw_overlay screen
        if @arrow_on
            screen.blit @arrow, [$tanks[@turn].pos[0]-@arrow.w/2,$tanks[@turn].pos[1]-@arrow_delta]
        end
    
        x = 5
        text_color = [128,255,128,128]
        $tanks.length.times { |cur|
            name_color = $tanks[cur].body_color
            name_color[3] = 150
            name_color[3] = 255 if cur == @turn
            dir = ' <-' if $tanks[cur].fire_direction < 0
            dir = ' ->' if $tanks[cur].fire_direction > 0
            dir = '' if $tanks[cur].fire_angle == 90
            if @entering_name == cur
                screen.print [x,5], "#{$tanks[cur].name}_", name_color
            else
                screen.print [x,5], "#{$tanks[cur].name}", name_color
            end
            screen.print [x,15], " Health: #{sprintf('%.0f', $tanks[cur].health)}%", text_color
            if (cur == @turn)
                screen.print [x,25], " Angle: #{$tanks[cur].fire_angle}#{dir}", text_color
                screen.print [x,35], " Power: #{sprintf('%.1f',$tanks[cur].fire_strength)}", text_color
                if $tanks[cur].cur_weapon == 0
                    screen.print [x,45], "-#{$weapons[0][0]}-", text_color
                else
                    screen.print [x,45], "-#{$weapons[$tanks[cur].cur_weapon][0]}: #{$tanks[cur].weapons[$tanks[cur].cur_weapon]}-", text_color
                end
            end
            x += 150
        }
        screen.print [400-(@help_text.length*8/2),@help_pos], @help_text, [255,255,0,255] if @help_pos < 600
        if @big_help_pos < 600
            y = @big_help_pos
            h = y
            $full_help_text.each {
                h += 14
            }
            screen.fill [15,15,15,127], [100, y-20, 600, h-y+30]
            $full_help_text.each { |line|
                screen.print [400-(line.length*8/2),y], line, [text_color[0],text_color[1],text_color[2], 235]
                y += 14
            }
        end
    end
  
    def GameState.draw
        $playfield.draw
        $tanks.each {|t| t.draw }
        @part.draw
    end

end