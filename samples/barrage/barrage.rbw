START_FULLSCREEN = false
SHOW_FPS = false

# Barrage -- an Artillery RUDL demo
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
#
# Basically a proof-of-concept that rapid game development is a dream using RUDL
# I created the entire game and included utility classes in under 30 hours, even though I was doing many
# things for the first time (Physics engine, combining vector and bitmap graphics, timing, etc.)
#
# More a prototype than a finished product, but that was my intent.
# Note the game is not optimized at all.  I can see a million places where I could
# speed things up, but I've left them for the sake of clarity.
#
# Ideas:
#  - More graphics, special effects engine, sprites for bullets
#  - Some kind of gradient or bitmap sky instead of pitch black background
#  - High scores?
#  - Computer players (simple AI)
#  - Network play
#  - Changing the color of the playfield randomly
#  - Importing 800x600 bitmaps to use as the playfield instead of a randomly generated field (would be easy)
#  - 
#
# Important:  PLEASE don't use my physics engine.  It's a giant hack, I was basically expirimenting
# with different ways to do vector-raster collisions.  Besides, it's painfully slow, mostly I think
# because of the calls to Surface#get.  I can think of a dozen ways to optimize this.
# If you really want to learn more about game physics, here's a great place to start:
# http://www.harveycartel.org/metanet/news.html
# Their tutorials are good, and their linked references are even better.
# Check out especially "Advanced Character Physics"

require 'RUDL'
require 'utils/timer_control'
require 'graphics'
require 'particle'
require 'utils/state_engine'
require 'playfield'
require 'tank'
require 'menustate'
require 'gamestate'
require 'gameoverstate'
require 'yaml'
include RUDL;  include Constant

$tank_colors = [ [255,255,255], [255,0,0], [0,255,0], [0,0,255] ]
$player_names = ["Mike", "Amy", "Darben", "Trill"]

$game_options = []
$game_possibilities = {}
$game_option_values = {}
$game_options_help = {}
$game_options[0] = "Play Game"
$game_option_values["Play Game"] = "yes"
$game_possibilities["Play Game"]  = ["yes"]
$game_options_help["Play Game"] = "Start a new game."
$game_options[1] = "Players"
$game_option_values["Players"] = '2'
$game_possibilities["Players"] = ['2','3','4']
$game_options_help["Players"] = "Number of players joining the game."
$game_options[2] = "Player Starts"
$game_option_values["Player Starts"] = "Fixed"
$game_possibilities["Player Starts"] = ["Fixed", "Random"]
$game_options_help["Player Starts"] = "Evenly space players across the field, or put them in random positions."
$game_options[3] = "Map Steepness"
$game_option_values["Map Steepness"] = "Rolling"
$game_possibilities["Map Steepness"] = ["Flat", "Gentle", "Rolling", "Mountainous", "Steep", "Cliffs"]
$game_options_help["Map Steepness"] = "Determine how steep the randomly-generated maps are."
$game_options[4] = "Weapons"
$game_option_values["Weapons"] = "Yes"
$game_possibilities["Weapons"] = ["Yes", "No"]
$game_options_help["Weapons"] = "Use the fun weapons, or play 'pure' boring artillery."

# load preferences if they exist
begin
    file = File::open "barrage.prefs.yaml", "r"
    if file
        prefs = YAML::load file
        $player_names = prefs['player_names']
        $game_option_values = prefs['options']
        file.close
    end
rescue
end

$full_help_text =<<HEND
LEFT and RIGHT change angle, X quickly switches directions.
UP and DOWN change power.
W changes weapons.
SPACEBAR fires.
1-4 to change a player's name, RETURN when finished.
Q returns to the main menu.
R restarts the game with the current settings.
ESC quits.

Direct hits cause damage, as does falling and slamming into walls.
Try to knock your opponent into the abyss for an instant kill.

copyright (c) 2004 Brian Palmer, Pocket Martian Software
ruby license, do whatever you want with the source,
just give credit where it's due.

(Press H to return)
HEND
$full_help_text = $full_help_text.split("\n")

graphics = Graphics::new [800,600], START_FULLSCREEN, "Barrage v1.0"
changeState(MenuState)

$timer = TimerControl::new 80, 1000

$timer.define_graphics_callback {
  $state::draw
  graphics.copy
  $state::draw_overlay graphics.overlay
  graphics.flip
  $state::erase
}

$timer.define_logic_callback {
  #~ EventQueue::pump
  if event = EventQueue::poll
    case event
      when QuitEvent
        exit
      when KeyUpEvent
      $state::keyUp event.key
      when KeyDownEvent
      $state::keyDown event.key
    end
  end
  $state::physics
  $state::run
}

$timer.run
graphics.shutdown

# save preferences
file = File::open "barrage.prefs.yaml", "w"
prefs = {}
prefs['player_names'] = $player_names
prefs['options'] = $game_option_values
file.write prefs.to_yaml
file.close

if SHOW_FPS
  puts "FPS: #{$timer.curFPS}"
  puts "UPS: #{$timer.curUPS}"
end