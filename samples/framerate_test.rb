#!/usr/local/bin/ruby

# For demonstration purposes: display a string at 5 frames per second.

require 'RUDL'
require '../utility/frame_timer'

# f is the FrameTimer that keeps track of time.
# We ask it to keep track of a speed of 5 frames per second, (the screen gets updated
# 5 times per second) and ask it to never return a value higher than 100 lost frames
# per gameloop.
f=RUDL::FrameTimer.new(5, 100)
loop do
	# This will be done 5 times per second or less, because we're waiting for
	# the next frame each loop.
	# The returnvalue should be used to calculate how much everything moves,
	# if it is 1, you should move everything 1 millimeter/centimeter/pixel/yard
	# lightyear or whatever. If it's 7, you should move it seven times as far
	# (or even better: move it 7 times)
	puts "skip: #{f.wait_for_next_frame}\tfps: #{f.fps}"
end
