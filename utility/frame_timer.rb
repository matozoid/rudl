module RUDL

	# FrameTimer implements a time keeping system for pacing games. A frame is the image that is shown
	# to the viewer, and many times per second, a new frame should be shown to get animation.
	# FrameTimer will figure out the right time to show a new frame and will tell you if the computer
	# is falling behind, so that you can implement frameskipping. (Which means continuing with the game 
	# logic as normal, but not drawing one or more frames to keep up the speed)

	class FrameTimer

		attr_reader :count, :rate
		attr_accessor :max_skip

		# rate is the desired framerate, max_skip is the maximum number of skipped frames that
		# wait_for_next_move will return.
		def initialize(rate=60, max_skip=10)
			reset
			self.rate=rate
			@max_skip=max_skip
		end

		# Reset. Could be called just before your main game loop starts.
		def reset
			@last_time=Timer.ticks
			@frame_skip=0.0
			@count=0
		end

		# Set the desired framerate to another value
		def rate=(new_rate)
			@rate_ticks=1000.0/new_rate
			@rate=new_rate
		end

		# Returns the average frames per second of the last 10 calls to wait_for_next_frame.
		# Useful when written to the screen so that you can keep track of the current performance.
		def fps
			@rate/@frame_skip
		end

		# Waits until it is time to display a new frame.
		# Returns the amount of frames since the last call to wait_for_next_frame.
		# A number bigger than one means that the computer is not able to keep up. Ignoring this
		# number will mean that every hiccup or virus scanner will be able to slow your game down.
		# It should be used to compute a few frames without actually drawing them.
		def wait_for_next_frame
			ticks=Timer.ticks
			passed_ticks=ticks-@last_time
			passed_frames=passed_ticks/@rate_ticks
			if passed_frames<1
				passed_frames=1
				Timer.delay @rate_ticks-passed_ticks
			end
			if passed_frames>@max_skip
				passed_frames=@max_skip
			end
			@last_time=ticks
			@frame_skip=@frame_skip*0.9+passed_frames*0.1
			@count+=1
			passed_frames
		end
	end
end
