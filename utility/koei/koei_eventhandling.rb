# $Log: koei_eventhandling.rb,v $
# Revision 1.2  2003/10/05 10:53:20  tsuihark
# Ready for packaging
#

module Koei

	class Focus
		def Focus.window
		end
	end

	class EventHandler
		def initialize(id)
			@handlers=[]
			@id=id
		end

		def reset
			@handlers=[]
		end

		def add(handler)
			@handlers.push(handler)
		end

		def handle(*args)
			retval=false
			@handlers.each {|handler| 
				retval|=handler.call(*args)
			}
			retval
		end

		def bubble(destination, &block)
			while destination
				if yield(destination)
					return true
				end
				destination=destination.parent
			end
			return false
		end

		def send(destination, *args)
			bubble(destination) {|current|
				handled=false
				if current.respond_to?(@id)
					handled=current.__send__(@id).handle(*args)
				end
				handled
			}
		end

		module KeyDown
			attr_reader :on_key_down
			def initialize(*args, &init)
				@on_key_down=EventHandler.new(:on_key_down)
				super
			end
		end

		module KeyUp
			attr_reader :on_key_up
			def initialize(*args, &init)
				@on_key_up=EventHandler.new(:on_key_up)
				super
			end
		end

		module MouseUp
			attr_reader :on_mouse_up
			def initialize(*args, &init)
				@on_mouse_up=EventHandler.new(:on_mouse_up)
				super
			end
		end

		module MouseDown
			attr_reader :on_mouse_down
			def initialize(*args, &init)
				@on_mouse_down=EventHandler.new(:on_mouse_down)
				super
			end
		end

		module MouseMove
			attr_reader :on_mouse_move
			def initialize(*args, &init)
				@on_mouse_move=EventHandler.new(:on_mouse_move)
				super
			end
		end

		module Resize
			attr_reader :on_resize
			def initialize(*args, &init)
				@on_resize=EventHandler.new(:on_resize)
				super
			end
		end

		module Timer
			attr_reader :on_timer
			def initialize(*args, &init)
				@on_timer=EventHandler.new(:on_timer)
				super
			end
		end

	end

	module EventHandling

		attr_accessor :event
		attr_accessor :focus

		include EventHandler::KeyDown
		include EventHandler::KeyUp
		include EventHandler::MouseDown
		include EventHandler::MouseUp
		include EventHandler::MouseMove
		include EventHandler::Resize
		include EventHandler::Timer

		def inside?(rect, point)
			(point[0]>=rect[0]) &&
				(point[0]<rect[0]+rect[2]) &&
				(point[1]>=rect[1]) &&
				(point[1]<rect[1]+rect[3])
		end

		def find_target_ignoring_grabbed_mouse(pos)
			if !inside?(@rect, pos)
				return nil
			else
				pos[0]-=@rect[0]
				pos[1]-=@rect[1]
				@children.each {|child|
					target=child.find_target_ignoring_grabbed_mouse(pos)
					return target if target
				}
				return self
			end
		end

		def find_target(pos)
			if mouse_grabber
				return mouse_grabber
			end
			if !inside?(@rect, pos)
				return nil
			else
				pos[0]-=@rect[0]
				pos[1]-=@rect[1]
				@children.each {|child|
					target=child.find_target(pos)
					return target if target
				}
				return self
			end
		end

		def process_event event
			case event
				when ResizeEvent
				when ActiveEvent
				when EndOfMusicEvent
				when KeyDownEvent
					@focus.key_down.call(event.key, event.mod) if @focus && @focus.key_down
				when MouseMotionEvent
					on_mouse_move.send(find_target(event.pos.dup), event.pos, event.rel, event.button)
				when MouseButtonUpEvent
					on_mouse_up.send(find_target(event.pos.dup), event.pos, event.button)
				when MouseButtonDownEvent
					on_mouse_down.send(find_target(event.pos.dup), event.pos, event.button)
			end
		end

		def grab_mouse
			external_grab_mouse(self)
		end

		def release_mouse
			external_release_mouse
		end

	end

	@@__mouse_grabber=nil

	def mouse_grabber
		@@__mouse_grabber
	end

	private

	def external_grab_mouse(element)
		@@__mouse_grabber=element
	end
	def external_release_mouse
		@@__mouse_grabber=nil
	end
	
end
