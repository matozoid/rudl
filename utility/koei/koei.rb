require 'koei_layoutmanager.rb'
require 'koei_default.rb'
require 'koei_drawing.rb'
require 'koei_eventhandling.rb'
require 'koei_nesting.rb'

module Koei

	# BEHAVIOURS

	module Windowish
		# Keep track of focused element
		# Hotkeys
		# subwindows

		attr_reader :focused_element
		attr_accessor :focus_order

		def initialize(*args, &init)
			super
			@focus_order=[]
		end

		def focused_element=(new_focus)
			if @focus_order.size>1 && @focus_order.include?(new_focus)
				@focused_element
			else
				puts "tried an invalid focus"
			end
		end

		def focus_next
			puts "focus next"
		end
	end

	class EventHandler
		module LoseFocus
			attr_reader :on_lose_focus
			def initialize(*args, &init)
				@on_lose_focus=EventHandler.new(:on_lose_focus)
				super
			end
		end

		module GainFocus
			attr_reader :on_gain_focus
			def initialize(*args, &init)
				@on_gain_focus=EventHandler.new(:on_gain_focus)
				super
			end
		end
	end

	module Focusable
		include EventHandler::LoseFocus
		include EventHandler::GainFocus

		def initialize(*args, &init)
			super
			on_mouse_down.add(proc {find_root.focused_element=(self)})
			on_key_down.add(method :handle_shortcuts)
		end

		def handle_shortcuts(sym, mod)
			case sym
				when K_TAB
					find_root.focus_next
			end
		end
	end

	class EventHandler
		module Dragging
			attr_reader :on_dragging
			def initialize(*args, &init)
				@on_dragging=EventHandler.new(:on_dragging)
				super
			end
		end

		module DragStart
			attr_reader :on_drag_start
			def initialize(*args, &init)
				@on_drag_start=EventHandler.new(:on_drag_start)
				super
			end
		end

		module DragEnd
			attr_reader :on_drag_end
			def initialize(*args, &init)
				@on_drag_end=EventHandler.new(:on_drag_end)
				super
			end
		end
	end

	module Draggable
		include EventHandler::Dragging
		include EventHandler::DragStart
		include EventHandler::DragEnd

		def drag_mousedown(pos, button)
			if button==1 && !@drag_dragging
				grab_mouse
				@drag_start_pos=pos.dup
				@drag_last_pos=pos.dup
				on_drag_start.send(self, @drag_start_pos)
				@drag_dragging=true
			end
			true
		end

		def drag_mouseup(pos, button)
			if button==1 && @drag_dragging
				release_mouse
				@drag_dragging=false
				on_drag_end.send(self, @drag_start_pos, pos)
				@drag_dragging=false
			end
			true
		end

		def drag_mousemove(pos, rel, button)
			if @drag_dragging
				on_dragging.send(self, pos, [pos[0]-@drag_last_pos[0], pos[1]-@drag_last_pos[1]])
				@drag_last_pos=pos.dup
			end
			true
		end

		def dragging(pos, rel)
		end

		def drag_end(start_pos, end_pos)
		end

		def drag_start(pos)
		end

		def initialize(*args, &init)
			super
			on_mouse_down.add method :drag_mousedown
			on_mouse_up.add method :drag_mouseup
			on_mouse_move.add method :drag_mousemove
			on_dragging.add method :dragging
			on_drag_end.add method :drag_end
			on_drag_start.add method :drag_start
		end
	end

	class EventHandler
		module SwitchPushedLook
			attr_reader :on_switch_pushed_look
			def initialize(*args, &init)
				@on_switch_pushed_look=EventHandler.new(:on_switch_pushed_look)
				super
			end
		end

		module Click
			attr_reader :on_click
			def initialize(*args, &init)
				@on_click=EventHandler.new(:on_click)
				super
			end
		end
	end

	module Clickable
		include EventHandler::SwitchPushedLook
		include EventHandler::Click

		def click_mousedown(pos, button)
			if button==1 && !@click_in_progress
				grab_mouse
				@click_pos=pos.dup
				@button_pushed=true
				@click_in_progress=true
				on_switch_pushed_look.send(self, true)
			end
			true
		end

		def click_mouseup(pos, button)
			if button==1 && @click_in_progress
				release_mouse
				if find_root.find_target(pos)==self
					on_click.send(self, @click_pos)
				end
				on_switch_pushed_look.send(self, false)
				@click_in_progress=false
			end
			true
		end

		def click_mousemove(pos, rel, button)
			if @click_in_progress
				@new_button_pushed=find_root.find_target_ignoring_grabbed_mouse(pos)==self
				if @button_pushed!=@new_button
					on_switch_pushed_look.send(self, @new_button_pushed)
				end
			end
			true
		end

		def initialize(*args, &init)
			super
			on_mouse_down.add method :click_mousedown
			on_mouse_up.add method :click_mouseup
			on_mouse_move.add method :click_mousemove
		end
	end

	module TopFix
		def initialize(*args, &proc)
			super()
		end
	end

	class Element

		include TopFix
		include Layoutmanager::Laamella
		include Drawing
		include EventHandling
		include Nesting

		attr_accessor :rect
		attr_accessor :name

		def set_defaults
			set_drawing_defaults

			if @parent
				@measure=parent.measure
				@rect=[0,0,0,0]
				@name=id.to_s
			else
				@measure=Default.measure
				@rect=Default.rect
				@name='top'
			end

			@dirty_image=true
			@dirty_size=true
			@children=[]
			@min_size=[0,0]
			@max_size=[5000,5000]
			@alignment=0
			@margin=[0,0,0,0]
		end

		def add(elementname, element=nil, pos=nil)
			elementname=elementname.to_s
			if !element
				if instance_eval "@#{elementname}.is_a? Koei::Element"
					instance_eval <<-EOD
						def #{elementname}
							@#{elementname}
						end
						@#{elementname}.name='#{elementname}'
					EOD
				else
					puts "tried to add a non-element to #@name"
				end
			else
				if element.is_a? Koei::Element
					instance_eval <<-EOD
						def #{elementname}
							@#{elementname}
						end
						@#{elementname}=element
						@#{elementname}.name='#{elementname}'
				EOD
				else
					puts "tried to add a non-element to #@name"
				end
			end
		end

		def initialize_elements(&init)
			if init
				attr_list=instance_variables
				push_binding
				instance_eval &init
				pop_binding
				new_attrs=instance_variables-attr_list
				new_attrs.each {|new_attr|
					attr_name=new_attr[1..-1]
					add(attr_name)
				}			
			end

		end

		def pre_user_elements(*args)
		end

		def post_user_elements(*args)
		end

		def initialize(*args, &init)
			set_defaults
			super
			pre_user_elements *args
			initialize_elements &init
			post_user_elements *args
		end

		def new_surface
			if w>0 && h>0
				@surface=RUDL::Surface.new [w,h]
			else
				@surface=RUDL::Surface.new [1,1]
			end			
		end

		def resize_internal
			reorganize_children
			if @surface
				if @surface.w!=w || @surface.h!=h
					new_surface
				end
			else
				new_surface
			end
		end

		def recursive_inspect(level)
			retval=".."*level+@name+"\n"
			children.each {|child|
				retval+=child.recursive_inspect(level+1)
			}
			retval
		end

		def inspect
			recursive_inspect(0)
		end
	end

	# ELEMENTS

	class Panel < Element
		def set_defaults
			super
			@alignment=Fill
		end

		def draw_self
			@surface.fill(@color1)
		end
	end

	class BorderedPanel < Panel
		def pre_user_elements(raised)
			@raised=raised
		end

		def set_defaults
			super
			@margin=[2,2,2,2]
		end

		def draw_self
			super
			if @raised
				draw_raised_border(@color1)
			else
				draw_lowered_border(@color1)
			end
		end
	end

	class LoweredPanel < Panel
		def draw_self
			super
			draw_lowered_border(@color1)
		end
		def set_defaults
			super
			@margin=[2,2,2,2]
		end

	end

	class RaisedPanel < Panel
		def draw_self
			super
			draw_raised_border(@color1)
		end
		def set_defaults
			super
			@margin=[2,2,2,2]
		end

	end

	class TitleBar < Panel

		include Draggable

		def set_defaults
			super
			@rect=[0,0,10*@measure,@measure]
			@alignment=Top|Left|Right
		end

		def pre_user_elements(text)
			initialize_elements {
				@close_bitton=CloseBitton.new
				@maximize_bitton=MaximizeBitton.new
				@minimize_bitton=MinimizeBitton.new
				@label=Label.new text
			}
		end

		def dragging(pos, rel)
			root=find_root
			root.rect[0]+=rel[0]
			root.rect[1]+=rel[1]
			set_size_dirty
		end
	end

	class StatusBar < LoweredPanel
		def set_defaults
			super
			@rect=[0,0,10*@measure,@measure]
			@alignment=Bottom|Left|Right
		end
	end

	class ScrollBar < Panel
		def set_defaults
			super
			@rect=[0,0,@measure,10*@measure]
			@alignment=Right|Top|Bottom
		end
	end

	class Bitton < BorderedPanel
		include Clickable
		include Focusable

		def set_defaults
			super
			@rect=[0,0,@measure,@measure]
			@alignment=Right
		end

		def pre_user_elements
			super true
			on_switch_pushed_look.add method :switch_pushed_look
		end

		def switch_pushed_look(pushed)
			@raised=!pushed
			set_image_dirty
		end
	end

	class CloseBitton < Bitton
		def draw_self
			super
			@surface.line([3,2],[w-4,h-4],0x000000FF)
			@surface.line([w-4,2],[3,h-4],0x000000FF)
			@surface.line([2,2],[w-5,h-4],0x000000FF)
			@surface.line([w-5,2],[2,h-4],0x000000FF)
		end

		def post_user_elements
			super
			on_click.add method :clicked
		end

		def clicked(pos)
			exit # find_root.close
			true
		end
	end

	class MinimizeBitton < Bitton
		def draw_self
			super
			@surface.line([2,h-4],[w-3,h-4],@color2)
		end
	end

	class MaximizeBitton < Bitton
		def draw_self
			super
			@surface.line([2,3],[w-3,3],@color2)
		end
	end

	class ResizeBitton < Panel
		include Draggable
		
		def draw_self
			super
			@surface.line([w-3,3],[3,h-3],@color2)
			@surface.line([w-3,6],[6,h-3],@color2)
			@surface.line([w-3,9],[9,h-3],@color2)
			@surface.line([w-3,12],[12,h-3],@color2)
		end
		
		def set_defaults
			super
			@rect=[0,0,@measure,@measure]
			@alignment=Right
		end

		def dragging(pos, rel)
			root=find_root
			root.rect[2]+=rel[0]
			root.rect[3]+=rel[1]
			set_size_dirty
		end
	end

	class Label < Panel
		attr_reader :text

		def set_defaults
			super
			@rect=[0,0,5*@measure,@measure]
			@alignment=Left|Top|Right|Bottom
		end

		def pre_user_elements(text)
			@text=text
		end

		def draw_self
			super
			@surface.print [2,2], @text, @color3
		end

		def text=(text)
			@text=text
			set_image_dirty
		end
	end

	class BareWindow < Panel
		include Windowish
	end

	class Window < BareWindow

		def pre_user_elements(title, menu=nil)
			initialize_elements {
				add(:titlebar, TitleBar.new(title))
				add(:menubar, MenuBar.new(menu)) if menu
			}
		end
	end

	class MenuItem < RaisedPanel
	end

	class Menu < RaisedPanel
	end

	class MenuBar < Panel
		attr_accessor :menus

		def pre_user_elements(menus)
			@menus=menus
		end

		def set_defaults
			super
			@rect=[0,0,10*@measure,@measure]
			@alignment=Top|Left|Right
		end
	end
end
