=begin
@file gamegui
@class Slider
<b>Not written yet</b> Lets the user select a value on a slider, like volume.
=end
module GameGUI
	class Slider < Selector
		def initialize(text, id, min, max, default, step)
			super(text, id)
		end
	end
end
