# $Log: koei_layoutmanager.rb,v $
# Revision 1.2  2003/10/05 10:53:20  tsuihark
# Ready for packaging
#

module Koei

	Left=0x01
	Right=0x02
	Top=0x04
	Bottom=0x08
	Fill=Left|Right|Top|Bottom

	module Layoutmanager

		module Nil
			def reorganize_children
			end
		end

		module Laamella
			attr_accessor :measure
			attr_accessor :margin
			attr_accessor :alignment
			#attr_accessor :min_size, :max_size

			def get_horizontal_size_range
				[100,1000]
			end

			def get_vertical_size_range
				[100,1000]
			end

			def do_stretchers(left,top,right,bottom,stretchers)

				old_left=left
				old_right=right
				old_top=top
				old_bottom=bottom
				stretchers_left=stretchers.size
				biggest=0

				horizontal=false
				vertical=false
				fill=false

				alignment=stretchers[0].alignment
				
				if alignment!=Fill
					horizontal=(alignment&(Left|Right))==(Left|Right)
					vertical=(alignment&(Top|Bottom))==(Top|Bottom)
				else
					horizontal=true
					fill=true
				end


				topleft=false
				topleft=(vertical && (alignment&Left>0)) ||
					(horizontal && (alignment&Top>0))

				space_left=0
				stretchers.each {|koe|

					if horizontal
						space_left=right-left
					else
						space_left=bottom-top
					end
						
					my_space=space_left/stretchers_left

					if horizontal
						if topleft||fill
							koe.left=left
							koe.top=top
							left+=my_space
						else
							koe.top=bottom-koe.h
							right-=my_space
							koe.left=right
						end
						koe.w=my_space
						biggest=koe.h if koe.h>biggest
						koe.h=bottom-top if fill
					else
						if topleft||fill
							koe.left=left
							koe.top=top
							top+=my_space
						else
							koe.left=right-koe.w
							bottom-=my_space
							koe.top=bottom
						end
						koe.h=my_space
						koe.w=right-left if fill
						biggest=koe.w if koe.w>biggest
					end
					stretchers_left-=1
				}

				if horizontal
					if topleft
						[old_left, old_top+biggest, old_right, old_bottom]
					else
						[old_left, old_top, old_right, old_bottom-biggest]
					end
				elsif vertical
					if topleft
						[old_left+biggest, old_top, old_right, old_bottom]
					else
						[old_left, old_top, old_right-biggest, old_bottom]
					end
				else
					[0,0,0,0]
				end
			end

			def reorganize_children
				left=@margin[0]
				right=w-@margin[2]
				top=@margin[1]
				bottom=h-@margin[3]

				stretcher=[]	# Can be stretched or shrunk
				stretchtype=0
				largest=0
				unorganized=[]	# Should stay put
				@children.each {|child|
					case child.alignment
						when 0
							unorganized.push child
						when Left
							child.left=left
							left+=child.w
						when Right
							child.left=right-child.w
							right-=child.w
						when Top
							child.top=top
							top+=child.h
						when Bottom
							bottom-=child.h
							child.top=bottom
						when Left|Bottom
							bottom-=child.h
							child.top=bottom
							child.left=left
						when Left|Top
							child.top=top
							child.left=left
							top+=child.h
						when Right|Bottom
							bottom-=child.h
							child.top=bottom
							child.left=right-child.w
						when Right|Top
							child.top=top
							child.left=right-child.w
							top+=child.h
						else # Stretchers must be summed
							if stretchtype!=0 && stretchtype!=child.alignment
								left,top,right,bottom=do_stretchers(left,top,right,bottom,stretcher)
								stretcher=[]
							end
							stretchtype=child.alignment
							stretcher.push(child)
					end
				}

				if stretcher.size>0
					left,top,right,bottom=do_stretchers(left,top,right,bottom,stretcher)
				end

				if $DEBUG			
					@children.each {|child|
						puts "  #{child.name} is now at #{child.rect.join(',')}"
					}
				end
			end
		end
	end
end