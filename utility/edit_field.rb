module RUDL
	class EditField
		include Constant

		attr_accessor :cursor_pos, :content, :max_length, :blink_timer, :cursor_visible
		attr_accessor :foreground_color, :background_color
		attr_reader :area, :enter_given, :escape_given


		def initialize(area, foreground_color=[255,255,255], background_color=[0,0,0], transparant=true, max_length=nil)
			@area=Rect.new(area)
			@max_length=max_length
			@text_surface=Surface.new [@area.w, @area.h]
			@cursor_visible=true
			set_colors(foreground_color, background_color, transparant)
			clear
		end

		def set_colors(foreground_color, background_color, transparant)
			@foreground_color=foreground_color
			@background_color=background_color
			@text_surface.set_colorkey(@background_color) if transparant
		end

		def clear
			@content=''
			@cursor_pos=0
			@enter_given=false
			@escape_given=false
			reset_blink
		end

		def insert(character)
			if room_for_character?(character)
				@content=@content[0..@cursor_pos-1]+character+@content[@cursor_pos..-1]
				@cursor_pos+=1
				reset_blink
			end
		end

		def delete
			pre=@content[0..@cursor_pos-1]
			post=@content[@cursor_pos+1..-1]
			puts pre+' '+post
			post='' if !post
			pre='' if @cursor_pos==0
			@content=pre+post
			reset_blink
		end

		def backspace
			if @cursor_pos>0
				pre=@content[0..@cursor_pos-2]
				post=@content[@cursor_pos..-1]
				post='' if !post
				pre='' if @cursor_pos==1
				@content=pre+post
				@cursor_pos-=1
				reset_blink
			end
		end

		def cursor_left
			@cursor_pos-=1 if @cursor_pos>0
			reset_blink
		end

		def cursor_right
			@cursor_pos+=1 if @cursor_pos<@content.length
			reset_blink
		end

		def cursor_home
			@cursor_pos=0
			reset_blink
		end

		def cursor_end
			@cursor_pos=@content.length
			reset_blink
		end

		def cursor_goto_xy(coordinate)
			if @area.contains(coordinate)
				offset=coordinate[0]-@area.x
				new_pos=0
				while offset>0 && new_pos<@content.length
					char_size=compute_character_size(@content[new_pos])
					offset-=char_size
					if offset<=0
						if (offset.abs)<char_size/2
							new_pos+=1
						end
					else
						new_pos+=1
					end
				end
				@cursor_pos=new_pos
			end
		end

		def process_event(event)
			case event
				when KeyDownEvent
					case event.key
						when K_SPACE..K_z
							if event.mod&(KMOD_LSHIFT|KMOD_RSHIFT)!=0
								insert(shiftify(event.key))
							else
								insert(event.key.chr)
							end
						when K_BACKSPACE
							backspace
						when K_DELETE
							delete
						when K_LEFT
							cursor_left
						when K_RIGHT
							cursor_right
						when K_END
							cursor_end
						when K_HOME
							cursor_home
						when K_RETURN
							@enter_given=true
						when K_ESCAPE
							@escape_given=true
					end
				when MouseButtonDownEvent
					cursor_goto_xy(event.pos)
			end
		end

		def blinkstate
			((Timer.ticks-@blink_timer)/500)%2==0
		end

		def reset_blink
			@blink_timer=Timer.ticks
		end

		def draw(surface)
			@text_surface.fill @background_color
			draw_text
			cursor_x=0
			cursor_x=compute_string_length(@content[0..@cursor_pos-1]) if @cursor_pos>0
			draw_cursor(cursor_x) if @cursor_visible
			surface.blit @text_surface, [@area.x, @area.y]
		end

		private

		def compute_string_length(string)
			(string.length)*8
		end

		def compute_character_size(character)
			8
		end

		def room_for_character?(character)
			if @max_length
				@content.length<@max_length
			else
				compute_string_length(@content+character)<@area.w
			end
		end

		def draw_text
			@text_surface.print [0,0], @content, @foreground_color
		end

		def draw_cursor(x)
			@text_surface.line [x,0], [x,8], @foreground_color if blinkstate
		end

		MapTable={
			K_1=>'!',
			K_2=>'@',
			K_3=>'#',
			K_4=>'$',
			K_5=>'%',
			K_6=>'^',
			K_7=>'&',
			K_8=>'*',
			K_9=>'(',
			K_0=>')',
			K_BACKQUOTE=>'~',
			K_MINUS=>'_',
			K_EQUALS=>'+',
			K_BACKSLASH=>'|',
			K_LEFTBRACKET=>'{',
			K_RIGHTBRACKET=>'}',
			K_SEMICOLON=>':',
			K_QUOTE=>'"',
			K_COMMA=>'<',
			K_PERIOD=>'>',
			K_SLASH=>'?'};

		def shiftify(character)
			case character
				when K_a..K_z
					character.chr.upcase
				else
					if MapTable.include?(character)
						MapTable[character]
					else
						' '
					end
			end
		end 
	end

	class TTFEditField < EditField
		attr_accessor :font

		def initialize(font, area, foreground_color=[255,255,255], background_color=[0,0,0], max_length=nil)
			@font=font
			super(area, foreground_color, background_color, false, max_length)
		end

		private

		def compute_string_length(string)
			@font.size(string)[0]
		end

		def compute_character_size(character)
			@font.size(character)[0]
		end

		def room_for_character?(character)
			if @max_length
				@content.length<@max_length
			else
				compute_string_length(@content+character)<@area.w
			end
		end

		def draw_text
			if @content.length>0
				@text_surface.blit(@font.render(@content, true, @foreground_color, @background_color), [0,0])
			end
		end

		def draw_cursor(x)
			@text_surface.line [x,0], [x,@font.h], @foreground_color if blinkstate
		end
	end

	class SFontEditField < EditField
		attr_accessor :font

		def initialize(font, area, max_length=nil)
			@font=font
			@font_height=@font.size(' ')[1]
			super(area, [255,255,255], [0,0,0], true, max_length)
		end

		private

		def compute_string_length(string)
			@font.size(string)[0]
		end

		def compute_character_size(character)
			@font.size(character)[0]
		end

		def room_for_character?(character)
			if @max_length
				@content.length<@max_length
			else
				compute_string_length(@content+character)<@area.w
			end
		end

		def draw_text
			if @content.length>0
				@font.puts(@text_surface, [0,0], @content)
			end
		end

		def draw_cursor(x)
			@text_surface.line [x,0], [x,@font_height], @foreground_color if blinkstate
		end
	end
end
