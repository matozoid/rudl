# $Log: koei_nesting.rb,v $
# Revision 1.2  2003/10/05 10:53:20  tsuihark
# Ready for packaging
#

module Koei
	$bindings=[]

	def push_binding
		set_trace_func proc {|event, file, line, id, binding, classname|
			$bindings.push(binding) 
			set_trace_func nil
		}
	end

	def pop_binding
		$bindings.pop
	end

	def top_binding
		$bindings.at -1
	end


	#

	module Nesting
		attr_reader :children
		attr_accessor :dirty_image
		attr_accessor :dirty_size
		attr_accessor :parent

		def initialize(*args, &init)
			binding=top_binding

			if binding
				@parent=eval "self", binding
				@parent.children.push self
			else
				@parent=nil
			end
			super
			
		end

		def set_image_dirty
			@dirty_image=true
			@parent.set_image_dirty if @parent
		end

		def set_size_dirty
			@dirty_size=true
			@parent.set_size_dirty if @parent
		end

		def cut_tree
		end

		def copy_tree
		end

		def paste_tree(tree)
		end

		def find_root
			return self if @parent==nil
			@parent.find_root
		end



	end
end
