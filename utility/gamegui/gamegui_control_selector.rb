=begin
@file gamegui
@class ControlSelector
<b>Not written yet</b> Lets the user pick a control.
=end
module GameGUI
	module Control
		class KeyboardControl
		end
		
		class DigitalJoystickControl
		end
		
		class AnalogJoystickControl
		end
		
		class MouseControl
		end
	end

	# For assigning a control (keyboard, joystick, mouse) to an action
	class ControlSelector < Selector
		attr_accessor :control
		def initialize(text, id, default_control)
			super(text, id)
			@control=default_control
		end

		def ControlSelector.initialize(supported_controls)
		end
	end
	
end