# for multiple projects, may include stuff not relevant to this one
# TODO: 
# - Remove unnecessary/duplicate stuff (entire file?)

class Array
	def duplicate()
		ret=[]
		each_with_index { |o,i|
			ret[i]=o.respond_to?(:duplicate) ? o.duplicate() : 
						o.instance_of?(Fixnum) ? o : o.dup()
		}
		ret
	end
	
	alias has? include?

    def m_add(ar)
        rar=self.dup()
        rar.each_index { |i| rar[i]+=ar[i] }
        rar
    end
	def m_mul(ar)
        rar=self.dup()
        rar.each_index { |i| rar[i]*=ar[i] }
        rar
	end
	def m_floor()
		rar=self.dup()
		rar.each_index { |i| rar[i].floor() }
		rar
	end
	def m_floor!()
		each_index { |i| self[i].floor() }
	end
	
	def add_width(val)
		self.w+=val
		self.x-=val/2
	end
	
	# <<< Coldet
    def mid_x() x+w/2 end
    def mid_y() y+h/2 end
    def mid_pos() [mid_x(),mid_y()] end
    def dim() [w,h] end
    def pos() [x,y] end
    def c_left_of?(r) x+w<r.x end
    def p_left_of?(r) x<r.x end
    def c_right_of?(r) x>r.x+r.w end
    def p_right_of?(r) x>r.x+r.w-w end
    def c_above?(r) y+h<r.y end
    def p_above?(r) y<r.y end
    def c_below?(r) y>r.y+r.h end
    def p_below?(r) y>r.y+r.h-h end
    def touches?(r) find_overlapping_rect([r]) end
    def touches_top?(r) (touches?(r) && p_above?(r) && (!c_left_of?(r)) && (!c_right_of?(r))) end
    def touches_bottom?(r) (touches?(r) && p_below?(r) && (!c_left_of?(r)) && (!c_right_of?(r))) end
    def touches_left?(r) (touches?(r) && p_left_of?(r) && (!c_above?(r)) && (!c_below?(r))) end
    def touches_right?(r) (touches?(r) && p_right_of?(r) && (!c_above?(r)) && (!c_below?(r))) end 
	# Coldet >>>
end