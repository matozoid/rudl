# TODO: 
# - FIND OUT WHY rescue LocalJumpError IS NEEDED IN MENU
# - add Lineprompt class similar to Keyprompt
# - beautification ?

module UI
	TW,TH=8,8

	class Interface
		def initialize()
			@elements=[]
			@modal=0
		end
		def draw()
			@elements.each { |element|
				element.draw
			}
		end
		def modal?
			@modal>0
		end
		def add(elem)
			@elements+=[elem]
			@modal+=1 if elem.modal?
		end
		def remove(elem)
			@modal-=1 if elem.modal? && @modal>0
			@elements-=[elem]
		end
		def keypress(key)
			if @modal>0
				debug "Modal Keypress - Reciever: #{@elements[-1]}"
				@elements[-1].keypress(key)
			else
				@elements.each { |element|
					element.keypress(key)
				}
			end
		end
	end
	
	class Element
		attr_accessor :rect
		def initialize(target,r)
			@bmp=target
			@rect=r
		end
		def draw() end
		def modal?() false end
		def keypress(key) end
	end
	
	class Box < Element
		def initialize(bmp,rect,border=4,bcol=[230,230,230],fcol=[40,40,40])
			super(bmp,rect)
			@border=border
			@bcol=bcol
			@fcol=fcol
		end
		def draw()
			@bmp.fill(@fcol,@rect)
			(1..@border).each { |i|
				@bmp.rectangle( [@rect[0]+i,@rect[1]+i,@rect[2]-i*2,@rect[3]-i*2],
												@bcol.collect { |v| v/i } )
			}
		end
	end
	
	class Text < Element
		attr_accessor :text
		attr_accessor :color
		def initialize(bmp,rect,text,color=[230,230,230])
			super(bmp,rect)
			@text=text
			@color=color
		end
		def draw()
			@bmp.print(@rect.pos,
				defined?(@text.call) ? @text.call : @text, @color)
		end
		def length()
			(defined?(@text.call) ? @text.call : @text).length
		end
	end
		
	class Centeredtext < Text	
		def initialize(bmp,rect,text,color=[230,230,230])
			super(bmp,rect,text,color)
			reposition()
		end
		def reposition()
			@rect.x=@rect.mid_x-length*TW/2
			@rect.y=@rect.mid_y-TH/2
		end
	end
	
	class Hline < Element
		def initialize(bmp,rect,thick=4,color=[230,230,230])
			super(bmp,rect)
			@thick=thick
			@color=color
		end
		def draw()
			(1..@thick).each { |i|
				@bmp.horizontal_line( [@rect.x,@rect.y+i], @rect.w,
												@color.collect { |v| v/i } )
			}
		end
	end
	
	class Messagebox < Element
		def initialize(bmp,rect,*lines)
			super(bmp,rect)
			longest=0
			lines.each { |l|
				length = (defined?(l.call) ? l.call : l).length
				longest= length if length > longest 
			}
			@box=Box.new(bmp,[rect.mid_x-longest*TW/2-8, 
										rect.mid_y-lines.length*(TH+2)/2-8,
										longest*TW+16,lines.length*(TH+2)+16] )
			@texts=[]
			lines.each_with_index { |l,i|
				@texts+=[Centeredtext.new(bmp,[rect.x,
							rect.mid_y-lines.length*(TH+2)/2+i*(TH+2),
							rect.w,TH],l)]
			}
		end
		def draw()
			@box.draw
			@texts.each { |t| t.draw() }
		end
	end
	
	class Keyprompt<Messagebox
		def initialize(bmp,rect,dproc,*lines)
			super(bmp,rect,*lines)
			@proc=dproc
		end
		def modal?
			true
		end
		def keypress(key)
			@proc.call(key,self)
		end
	end
	
	class Menu < Messagebox
		def initialize(bmp,rect,*options)
			super(bmp,rect,*(options.collect { |o| o[0] }))
			@procs = options.collect { |o| o[1] }
			@box.rect.add_width(TW*4)
			@selected=0
			select(0)
		end
		def select(entry)
			if entry>=0 && entry<@texts.length
				@texts[@selected].color=[230,230,230]
				@texts[entry].color=[200,50,50]
				@selected=entry
			end
		end
		def keypress(key)
			case key
				when K_UP: 
					select(@selected-1)
				when K_DOWN: 
					select(@selected+1)
				else
					begin
						@procs[@selected].call(@texts[@selected],self,key)
					rescue LocalJumpError 
					end
			end			
		end
	end
end