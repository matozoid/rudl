require 'RUDL'
include RUDL
include Constant

#

class RUDL::Surface
	def contained_image_areas
		lines_done=false
		row_done=false
		corner_color=get [0,0]
		image_area=[0,0,0,0]
		
		images=[]
		image_line=[]
		
		while !(row_done&&lines_done)
			image_area.w=1
			image_area.h=1
			row_done=false
			raise("Upper left pixel at #{image_area.x}, #{image_area.y} not the same as bordercolor.") if get(image_area)!=corner_color
			
			while get([image_area.x+image_area.w, image_area.y])!=corner_color
				image_area.w+=1
				raise("No terminating pixel at #{image_area.x}, #{image_area.y}") if image_area.x+image_area.w>w
			end

			while get([image_area.x, image_area.y+image_area.h])!=corner_color
				image_area.h+=1
				raise("No terminating pixel at #{image_area.x}, #{image_area.y}") if image_area.y+image_area.h>h
			end
			
			#puts "Found an image at #{image_area.join(',')}"
			image_area.w-=1
			image_area.h-=1
			image_area_to_add=image_area.dup
			image_area_to_add.x+=1
			image_area_to_add.y+=1
			image_line.push image_area_to_add

			# Find next line
			if image_area.x==0
				#puts "Looking for next line of images"
				next_row_y=image_area.y+image_area.h+2
				done=false
				while !done
					if next_row_y>=h
						done=true
						lines_done=true
					else
						if (get [0, next_row_y]) ==corner_color
							done=true
						else
							next_row_y+=1
						end
					end
				end
			end
			# Setup for next image
			image_area.x+=image_area.w+2

			# Find next X
			next_x_found=false
			while !next_x_found
				if image_area.x>=w
					#puts "Line done"
					image_area.x=0
					image_area.y=next_row_y
					row_done=true
					next_x_found=true
					images.push image_line
					image_line=[]
				else
					if (get image_area)==corner_color
						next_x_found=true
					else
						image_area.x+=1
					end
				end
			end
		end
		return nil if images.size==0
		return images[0] if images.size==1
		return images
	end
	
	def contained_images
		contained_image_areas.collect do |area|
			copy_part_to_new_surface(area)
		end
	end
	
	def copy_part_to_new_surface(area)
		surface=Surface.new([area.w, area.h], self)
		p area
		(0...area.h).each do |row_nr|
			row=get_row(row_nr+area.y)
			row=row[area.x*surface.bytesize, area.w*surface.bytesize]
			surface.set_row(row_nr, row)
		end
		surface
	end
end
