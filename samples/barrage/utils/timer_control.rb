# The TimerControl class allows the syncronization of real-time simulations
# The biggest drawback to most timing schemes is that the logic rate becomes
# tied to the display frame rate (Frames Per Second), which is a Very Bad Thing.
#
# Many solutions to this problem have become standard and accepted, including
# "delta times" used to calculate all movements (which adds a ton of extra calculations
# to the game logic and introduces random error over time, very difficult to deal with
# if you want any kind of "playback" or "instant replay" mechanism) and setting a 
# constant frame rate to create a predictable logic rate.
#
# This method allows you to specify a steady logic rate, and the screen frames simply go
# as fast as they can go.
# The logic frame rate is guaranteed to be steady -- if the computer falls behind,
# the logic will be executed multiple times without updating the screen to bring
# things up to speed.  This allows for predictable behaviour -- if you use the same random
# seed, you will _always_ get exactly the same outcome, regardless of little "hiccoughs"
# on your system.
#
# Usage:
# timer = TimerControl::new(60, 1000)    # TODO: note that maxFPS currently doesn't do anything
# timer.define_logic_callback { do_logic_stuff }
# timer.define_graphics_callback { draw_the_screen }
# timer.run   # This function will return when timer.exit is called from your logic code
#
# Note that your game logic and graphics truly have to be separated for this to work as intended
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
#
# Speaking of which, I got the original idea for this method of timing from David Olofson's
# "fixed frame-rate pig" SDL demo

require 'RUDL'

module RUDL

  class TimerControl
    attr_accessor :logicFPS, :maxFPS
  
    def initialize(logicUPS = 30, maxFPS = 60)
      @logicUPS = logicUPS
      @maxFPS = maxFPS
      @in_game_FPS = maxFPS.to_f
      @in_game_UPS = logicUPS.to_f
      @logic_updates = @rendered_frames = 0.0
    end
    
    def curUPS
      @logic_updates / (@cur_tick * 0.001)
    end
    
    def curFPS
      @rendered_frames / (@cur_tick * 0.001)
    end
    
    def define_logic_callback(&block)
      @logic_callback = block
    end
    
    def define_graphics_callback(&block)
      @graphics_callback = block
    end
    
    def exit
      @exit = true
    end
    
    def run
      if @logic_updates > 0
        puts "Warning: you should only call TimerControl#run ONCE, and destroy the object when it returns."
      end
      @exit = false
      start_time = last_time = RUDL::Timer::ticks
      loop {
        @cur_tick = RUDL::Timer::ticks
        delta = (@cur_tick - last_time)
        logic_frames = delta * 0.001 * @logicUPS
        num_frames = (@logic_updates + logic_frames).floor - @logic_updates.floor
        num_frames.times { @logic_callback.call }
        @logic_updates += logic_frames
        
        @graphics_callback.call
        @rendered_frames += 1
        last_time = @cur_tick
        break if @exit
      }
      
    end
    
  end
end