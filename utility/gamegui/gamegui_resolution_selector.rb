=begin
@file gamegui
@class ResolutionSelector
<b>Not written yet</b> Lets the user pick a screen resolution.
=end
module GameGUI
	class ResolutionSelector < ListSelector
		def initialize(text, id, resolution_list=nil)
			resolution_list=DisplaySurface.modes if !resolution_list
			super(text, id, resolution_list)
		end
	end
end
