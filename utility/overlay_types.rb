require 'RUDL'

module Overlay
  # Add event stuff to base class
  class TimedEvent
    attr_reader :event

    def initialize(event, ms)
      #throw "event must be a String" if !event.kind_of?(String)

      @trigger_at = Timer.ticks + ms
      @event = event
    end

    def due?(time)
      time >= @trigger_at
    end
  end

  class MultiEvent < Array
    def initialize(*args)
      args.each { |a| self << a }
    end
  end

  class Generic < Overlay::Base
  
    attr_reader :clickable
    attr :name, true
    attr :clickable, true
    attr :skip_frame, true
    
    def initialize
      super
      @name = "Overlay#{self.id}"
      @pending = []
      @pending_timed = []
      @clickable = false
      @skip_frame = 0
    end
    
    def add_child_named(c,n)
      add_child(c)
    end
  
    # Default event handlers
    #
    def poll(ds, drawall=false)
    
      while (@pending.empty?)
                  
	case event = EventQueue.poll
          when NilClass
	    
	    if drawall
		paint_all_nodirty(ds)
	    else
		paint_all(ds)
	    end
	    	    
	    response  = nil
	    responder = nil
	    
	    # Call all the objects' idle handlers
	    each_child { |k|
	      response = k.respond_to?(:idle) ? k.idle : nil
	      event(response, k) if !response.nil?
	    }
	    
	    # Push any overdue events onto the pending queue
	    now = Timer.ticks
	    @pending_timed.each { |timed|
	      response, responder = *timed
	      next unless response.due?(now)
	      @pending_timed.delete(timed)
	      event(response.event, responder)
	    }
	    
	  when MouseButtonDownEvent
	    x, y = event.pos[0], event.pos[1]
		    
	    # Check for mouse clicks in reverse order, since the latest items
	    # that we drew will be on top
	    each_child { |k|
	      #print "#{k} : #{k.inside?(x,y)} #{k.visible}\n"
	      next unless k.inside?(x, y) && k.visible && k.clickable
	      
	      response = nil
	      while !k.nil? && response.nil?
	        response  = nil
	        response = k.clickable ? 
		  k.mouse_click(event.button, event.pos[0], event.pos[1]) :
		  nil
		k = k.parent unless !response.nil?
              end
	      
	      event(response, k)
	      break
	    }
	  
	  when KeyDownEvent
	    response = key_pressed(event.key)
	    event(response, self)
	  
	  when QuitEvent
	    raise "Quit"

	end
      end
      
      return @pending.shift
    end
    
    def event(response, responder=nil)
      return if response.nil?
      
      if response.kind_of?(MultiEvent)
        response.each { |r| event(r, responder) }
	return
      end
      
      if responder.nil?
        responder = self
	while (!responder.parent.nil?)
	  responder = responder.parent
	end
      end
      
      Log.log(3, "Event #{response} stored from #{responder} (#{caller[1]})")
    
      case response
        when TimedEvent
	  @pending_timed << [response, responder]
	else
	  @pending << [response, responder]
      end
    end
    
    def mouse_enter
      nil
    end
    def mouse_exit
      nil
    end
    def mouse_click(button, x, y)
      nil
    end
    def key_pressed(key)
      nil
    end

  end
  
  
  class Bitmap < Generic
  
    attr_reader :surface
      
    def initialize(sfcarg=nil)
      super()
      set_surface(sfcarg) if !sfcarg.nil?
    end
    
    def Bitmap.retrieve(name)
      #Log.log(1, "Retrieved #{name} = #{$gfx[name]}")
      s = Bitmap.new
      s.set_surface($gfx[name])
      @name = name
      return s
    end
  
    def set_surface(sfcarg)
      raise "sfcarg must be a RUDL::Surface" if !sfcarg.kind_of?(RUDL::Surface)
      @surface = sfcarg
    end
  
    def w
      @surface.nil? ? 0 : @surface.w
    end
    
    def h
      @surface.nil? ? 0 : @surface.h
    end
    
    def alpha
      @surface.alpha
    end
    
    def alpha=(a)
      @surface.set_alpha(a)
      flag_dirty
    end
    
    def w=(w)
      raise "Can't change width or height of a Bitmap"
    end
    def h=(h)
      raise "Can't change width or height of a Bitmap"
    end
    
    def update(sfc, up)
      if @skip_frame > 0
        @skip_frame -= 1
	return
      end
      up.x0 = 0 if up.x0 < 0 
      up.y0 = 0 if up.y0 < 0
      up.x1 = 0 if up.x1 < 0
      up.y1 = 0 if up.y1 < 0
      up.x0 = 799 if up.x0 > 799
      up.y0 = 799 if up.y0 > 799
      up.x1 = 799 if up.x1 > 799
      up.y1 = 799 if up.y1 > 799
      r = RUDL::Rect.new(up.x0, up.y0, up.w, up.h)
      #print "updating #{r.x},#{r.y} #{r.w}x#{r.h}\n"
      sfc.update([r])
    end
    
    def paint(sfc, rect=nil)
      pos = self.pos
      #print "sfc = #{sfc}\n"
      #print "rect = #{rect}\n"
      #Log.log(3, "Request to paint #{rect} for surface #{sfc}/#{self.name} : painting surface fragment @ #{pos.x0 + rect.x0},#{pos.y0+rect.y0} with bounding box [#{rect.x0}, #{rect.y0}, #{rect.w}, #{rect.h}]")
      
      if rect.nil?
        sfc.blit(@surface, [pos.x0, pos.y0])
      else
        sfc.blit(@surface, [pos.x0 + rect.x0, pos.y0 + rect.y0], [rect.x0, rect.y0, rect.w, rect.h])
      end
      #sfc.rectangle(Rect.new(@pos.x0 + rect.x0, @pos.y0 + rect.y0, self.w, self.h), 0xffffffff)
      #gets
    end
    
    def surface=(sfc)
      oldsfc = @surface
      set_surface(sfc)
      check_consistency
      flag_moved if oldsfc.nil? || oldsfc.w != self.w || oldsfc.h != self.h
      check_consistency
      flag_dirty
      check_consistency
    end
    
  end
  
  class Text < Bitmap
  
    attr_reader :font, :text, :wrap_width, :color
        
    def initialize(font=nil, text=nil, wrap=0, color=[255,255,255,255])
      raise "Font must be a TrueTypeFont" if font.type != TrueTypeFont
      raise "text must be a String" if text.type != String
      super()
      @name = "Text-#{text}"
      @text = text
      @font = font
      @wrap_width = wrap
      @color = color
      update_text
    end
    
    def font=(font)
      @font = font
      update_text
    end

    def text=(txt)
      raise TypeError, "Expecting String" if !txt.kind_of? String
      @text = txt
      update_text
    end
        
    def wrap_width=(wrap)
      @wrap_width = wrap
      update_text
    end
    
    def color=(color)
      @color = color
      update_text
    end
    
    def update_text
      return if @font.nil? || @text.nil?
      # FIXME: implement wrap width properly!
      @text = "(?)" if @text === ""
      text_surface = @font.render(@text, true, @color)
      if @wrap_width > 0 && text_surface.w > @wrap_width
      
        words = @text.split(" ")
        split_at = 0
	split_len = 0
	while split_len < @wrap_width
	  split_len += @font.size(words[split_at])[0]+10
	  split_at += 1
	end
	split_at -= 1
	
	split_surface = Surface.new([@wrap_width, text_surface.h*2+10])
	words1 = " "+words[0..split_at-1].join(" ")
	words2 = " "+words[split_at..words.length-1].join(" ")
	text1 = @font.render(words1, true, @color)
	text2 = @font.render(words2, true, @color)
	split_surface.blit(text1, [0, 0])
	split_surface.blit(text2, [0, split_surface.h/2])
	split_surface.set_colorkey(0)
	text_surface = split_surface
      end
      self.surface = text_surface
    end
    
    def Text.same_size(textobjs, fonts, width)
      textobjs.sort! { |a,b| 
        fonts[0].render(b.text, true, b.color).w <=> 
	fonts[0].render(a.text, true, a.color).w
      }
      
      lastfont = nil
      fonts.each { |f|
        lastfont = f
        tryrender = f.render(textobjs[0].text, true, textobjs[0].color)
	break if tryrender.w < width
      }
      textobjs.each { |t| t.font = lastfont }
    end
    
  end
  
  class SolidRectangle < Generic
    
    def initialize(w, h, color=0x000000ff)
      super()
      @color = color
      self.w = w
      self.h = h
    end
    
    def paint(sfc, rect)
      pos = self.pos
      return if pos.x0 == 0 && pos.y0 == 0
      if !rect.nil?
        sfc.filled_rectangle(RUDL::Rect.new(pos.x0 + rect.x0, pos.y0 + rect.y0, self.w, self.h), @color)
      else
        sfc.filled_rectangle(RUDL::Rect.new(pos.x0, pos.y0, self.w, self.h), @color)
      end
    end
    
  end
      
end
