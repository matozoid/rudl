=begin
@file gamegui
@module GameGUI
@section gameguitest
A sample program.
=end
require 'RUDL'
require 'gamegui'
require '../colors'

GameGUI::ControlSelector.initialize([GameGUI::Control::KeyboardControl.new])

mainscreen=GameGUI::Screen.new(
	RUDL::DisplaySurface.new([800,600]),
	[100,200,540,250],
	RUDL::TrueTypeFont.new('../../samples/media/adlibn.ttf', 30),
	RUDL::Constant::colors["red"],
	RUDL::Constant::colors["yellow"],
	[	GameGUI::CustomSelector.new('Start!') {
			puts "Start!"
			mainscreen.quit
		},
		GameGUI::TextInput.new('Player name:', :player_name, 15, 'Bloemenmeisje'),
		GameGUI::ListSelector.new('Gender:', :gender, ['Male', 'Female', 'Something else']),
		GameGUI::ImageSelector.new('Face:', :face, []),
		GameGUI::ControlSelector.new('Left:', :left, ','),
		GameGUI::ControlSelector.new('Right:', :right, '.'),
		GameGUI::ControlSelector.new('Fire:', :fire, ' '),
		GameGUI::Slider.new('Volume:', :volume, 0, 100, 90, 5),
		GameGUI::ResolutionSelector.new('Resolution:', :resolution),
		GameGUI::CustomSelector.new('Quit!') {
			mainscreen.quit
		}
	])

mainscreen.pre_draw=Proc.new { |screen|
	screen.display_surface.fill([0,0,0])
}

Key.set_repeat(500,100)

mainscreen.run

puts mainscreen.selectors_by_id[:player_name].value
puts mainscreen.selectors_by_id[:gender].value
puts mainscreen.selectors_by_id[:face].value
puts mainscreen.selectors_by_id[:left].value
puts mainscreen.selectors_by_id[:right].value
puts mainscreen.selectors_by_id[:fire].value
puts mainscreen.selectors_by_id[:volume].value
puts mainscreen.selectors_by_id[:resolution].value


