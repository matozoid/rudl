# $Log: koei_default.rb,v $
# Revision 1.2  2003/10/05 10:53:20  tsuihark
# Ready for packaging
#

module Koei
	module Default
		@@measure=16
		@@rect=[10,10,300,300]
		@@color1=[176,152,96]
		@@color2=[100,50,10]
		@@color3=[55,0,100]

		def Default.measure
			@@measure
		end

		def Default.measure= measure
			@@measure=measure
		end

		def Default.rect
			@@rect
		end

		def Default.rect= rect
			@@rect=rect
		end

		def Default.color1
			@@color1
		end

		def Default.color1= color
			@@color1=color
		end

		def Default.color2
			@@color2
		end

		def Default.color2= color
			@@color2=color
		end

		def Default.color3
			@@color3
		end

		def Default.color3= color
			@@color3=color
		end
	end
end
