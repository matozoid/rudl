#!/usr/local/bin/ruby
=begin
@file Samples
@class Info
This sample demonstrates some utility functions in RUDL.
=end

require 'RUDL'
include RUDL
include Constant

puts
puts "RUDL info"
puts

# This prints the versions of all libraries that were compiled into
# RUDL. Libraries that have version 0 don't provide version information.
puts "Libraries used:"
RUDL.versions.each {|library, version|
	puts "#{library} #{version}"
}

begin
	# This prints information about what the hardware can do.
	# Normally you would query it for a few values, but here
	# we just list all of them.
	puts
	puts "Video hardware info:"
	DisplaySurface.best_mode_info.sort.each {|name, value|
		if value
			puts "y #{name}"
		else
			puts "n #{name}"
		end
	}

	# Here we retrieve possible display modes. This information can
	# be used for selecting a good videomode to use.
	puts
	puts "Video modes: (F means fullscreen)"
	modes={}

	[	['8', 8, SWSURFACE],
		['16', 16, SWSURFACE],
		['32', 32, SWSURFACE],
		['8F', 8, FULLSCREEN],
		['16F', 16, FULLSCREEN],
		['32F', 32, FULLSCREEN]].each {|parameters|
		list=DisplaySurface.modes(parameters[1],parameters[2])
		list=[['any mode']] if !list
		list.each {|mode|
			modename=mode.join('x')
			if !modes[modename]
				modes[modename]=[parameters[0]]
			else
				modes[modename].push parameters[0]
			end
		}
	}
	sorted=[]
	modes.each {|mode, depths|
		sorted.push("#{mode}: #{depths.join(' ')}")
	}
	sorted.sort.each {|line| # OK, so the sorting is crap
		puts line
	}
rescue
	puts "No video driver available"
end

# Well, joystick info.
joysticks=Joystick.count

if joysticks==0
	puts "No joysticks available"
else
	(0...joysticks).each {|i|
		p Joystick.new(i)
	}
end

# The audio module returns an odd amount of channels...
begin
	format=Mixer.format
	puts "Format: #{format[0]} Channels: #{format[1]} Samplerate: #{format[2]} Driver: #{Mixer.driver}"
#rescue
end
