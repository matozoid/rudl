require 'RUDL'

module Mappy
	class Animation
		

		def self.size
			16
		end
		
		def load(chunk)
			@type, 
			@delay, @count, 
			@user_info,
			@current_offset, 
			@start_offset, @end_offset,
			chunk=chunk.unpack("ccccllla*")
			reset	# hmmm
			p self
			chunk
		end
		
		def reset
			@current_offset=@start_offset
			@count=@delay
		end
	end
	
	class DataBlock
	
		attr_accessor :bgoffset, :fgoffset, :fgoffset2, :fgoffset3
		attr_accessor :user1, :user2, :user3, :user4, :user5, :user6, :user7
		attr_accessor :top_left, :top_right, :bottom_left, :bottom_right
		attr_accessor :event_trigger
		attr_accessor :unused1, :unused2, :unused3
		
		def initialize
		end
		
		def load(chunk)
			@bgoffset, @fgoffset,
			@fgoffset2, @fgoffset3, 	# more overlay blocks
			@user1, @user2,			# user long data
			@user3, @user4,			# user short data
			@user5, @user6, @user7,	# user byte data
			bitfield, chunk=chunk.unpack('VVVVVVvvCCCCa*')
			
			@top_left=bitfield[0]==1
			@top_right=bitfield[1]==1
			@bottom_left=bitfield[2]==1
			@bottom_right=bitfield[3]==1
			@event_trigger=bitfield[4]==1
			@unused1=bitfield[5]==1
			@unused2=bitfield[6]==1
			@unused3=bitfield[7]==1

			chunk
		end

		def self.size
			32
		end
	end
	
	class Map
		attr_accessor :palette
		def initialize
			@layer=[nil, nil, nil, nil, nil, nil, nil, nil, nil]
		end
		
		def load(string)
			string=read_header(string)
			while string!=''
				string=read_chunk(string)
			end
		end
		
		def draw(surface)
			(0...@width).each do |x|
				(0...@height).each do |y|
					block=@layer[0][y][x]
					if block>=0
						surface=@graphics_blocks[block]
						surface.blit(surface, [x*@block_width, y*@block_height]) if surface
					else
						
					end
				end
			end
		end
		
		private
		
		def read_header(string)
			name, @map_size, string=read_chunk_header(string)
			throw("Bad map header: #{name}") if name!='FORM'
			name,string=string.unpack('a4a*')
			throw("Bad map header: #{name}") if name!='FMAP'
			string
		end
		
		def read_chunk_header(string)
			name, size, string=string.unpack('a4Na*')
			[name, size, string]
		end
		
		def read_chunk(string)
			name, size, string=read_chunk_header(string)
			chunk, string=string.unpack("a#{size}a*")
			case name
				when 'ATHR'
					@author_info=chunk.split('\0')
				when 'MPHD'
					read_map_header_chunk(chunk)
				when 'EDHD'
					# ignore editor preferences
				when 'NOVC'
					# ignore whatever this is
				when 'CMAP'
					read_colour_palette_chunk(chunk)
				when 'BKDT'
					read_block_data_chunk(chunk)
				when 'ANDT'
					read_animation_data_chunk(chunk)
				when 'AGFX'
					read_alternate_graphics_chunk(chunk)
				when 'BGFX'
					read_graphics_chunk(chunk)
				when 'BODY'
					read_layer_chunk(chunk, 0)
				when /LYR./
					read_layer_chunk(chunk, name[3]-?0)
				else
					puts "unknown chunk: #{name}"
			end
			string
		end

		def read_animation_data_chunk(chunk)
			@animations=[]
			
			while chunk.length>0
				animation=Animation.new
				chunk=animation.load(chunk)
				@animations.push(animation)
			end
		end
	
		def read_alternate_graphics_chunk(chunk)
			raise "not implemented"
		end
		
		def read_colour_palette_chunk(chunk)
			@palette=[]
			while chunk.length>2 do
				r,g,b, chunk=chunk.unpack('CCCa*')
				@palette.push([r,g,b])
			end
		end
		
		def read_block_data_chunk(chunk)
			@data_blocks=[]
			
			while chunk.length>0
				block=DataBlock.new
				chunk=block.load(chunk)
				@data_blocks.push(block)
			end
		end
		
		def read_graphics_chunk(chunk)
			@graphics_blocks=[]
			masks=[
				[],						# 0
				[0,0,0,0],					# 8 bit
				[0x00F8, 0xE007, 0x1F00, 0x0000],	# 16 bit
				[0XFF0000, 0XFF00, 0xFF],		# 24 bit
				[0XFF0000, 0XFF00, 0xFF]]		# 32 bit
			offset=0
			bytes_per_pixel=@block_depth/8
			byte_width=@block_width*bytes_per_pixel
			while offset<chunk.size
				rows=[]
				(0...@block_height).each do |y|
					rows.push(chunk[offset...offset+byte_width])
					offset+=byte_width
				end
				s=RUDL::Surface.new([@block_width, @block_height], RUDL::Constant::SWSURFACE, @block_depth, masks[bytes_per_pixel])
				s.rows=rows
				if bytes_per_pixel==1
					s.set_palette(0, @palette)
				end
				@graphics_blocks.push(s)
			end
		end
		
		def read_layer_chunk(chunk, layer_nr)
			layer=[]
			shorts=chunk.unpack('s*')
			case @type
				when 0
					i=0
					(0...@height).each do |y|
						line=[]
						(0...@width).each do |x|
							val= shorts[i]
							if val>=0
								val/=DataBlock.size
							else
								val/=16
							end
							line.push(val)
							i+=1
						end
						layer.push(line)
					end
				when 1
					i=0
					(0...@height).each do |y|
						line=[]
						val=nil
						(0...@width).each do |x|
							val=shorts[i]
							line.push(val)
							i+=1
						end
						layer.push(line)
						p line
					end
					
				#when 2
				#when 3
				else
					raise "Map type not supported: #{@type}"
			end
			@layer[layer_nr]=layer
		end
		
		def read_map_header_chunk(string)
			version_high, version_low, lsb, @type,
			@width, 
			@height, reserved1, reserved2,
			@block_width, 
			@block_height, 
			@block_depth,
			@blockstructure_size, 
			@blockstructures, 
			@graphicsblocks,
			@ckey8bit, @ckeyred, @ckeygreen, @ckeyblue, # colour key values added FMP0.4
			@blockgapx, @blockgapy, @blockstaggerx, @blockstaggery,
			@clickmask, 
			@pillars, string=string.unpack("CCCCvvvvvvvvvvCCCCvvvvvva*")
			
			@version=version_high+version_low/10.0
			@lsb=lsb==1
			
			p self
		end
	end
end

d=RUDL::DisplaySurface.new [640,480]

m=Mappy::Map.new
#m.load(File.open('../samples/media/test.fmp', 'rb').read)
m.load(File.open('C:/WINDOWS/Desktop/rrr/mapw1312.zip/mapw1312/MAPS/test2_10.fmp', 'rb').read)
d.set_palette(0, m.palette)
m.draw(d)
d.flip
RUDL::Timer.delay(5000)

