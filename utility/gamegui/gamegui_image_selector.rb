=begin
@file gamegui
@class ImageSelector
<b>Not written yet</b> Lets the user pick an image from a list.
=end
module GameGUI
	class ImageSelector < Selector
		def initialize(text, id, image_list)
			super(text, id)
		end
	end
end
