#!/usr/local/bin/ruby

require '../RUDL'
include RUDL
include Constant

puts
puts "RUDL info"
puts
puts "Libraries used:"
RUDL.versions.each {|library, version|
	puts "#{library} #{version}"
}

begin
	puts
	puts "Video hardware info:"
	DisplaySurface.best_mode_info.sort.each {|name, value|
		if value
			puts "o #{name}"
		else
			puts "x #{name}"
		end
	}
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

joysticks=Joystick.count

if joysticks==0
	puts "No joysticks available"
else
	(0...joysticks).each {|i|
		p Joystick.new(i)
	}
end

begin
	puts "Audio driver: #{Mixer.driver}"
#rescue
end