=begin
@file gamegui
@class CustomSelector
A selector that does not represent a value to choose, but an action to take,
like "start," "save," or "preferences."

Create it with a block that will be executed when the user presses enter.
=end
module GameGUI	
	class CustomSelector < Selector
		def initialize(text, &code)
			super(text, nil)
			@code=code
		end

		def on_select
			@code.call
		end
	end
end