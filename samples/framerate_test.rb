#!/usr/local/bin/ruby

require 'RUDL'

require '../utility/frame_timer'

# For demonstration purposes: display a string at 2 frames per second.
f=RUDL::FrameTimer.new(5, 100)

loop do
	puts "skip: #{f.wait_for_next_frame}\tfps: #{f.fps}"
end
